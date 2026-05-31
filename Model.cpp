#include"Model.h"

// Constructor: carga y parsea un archivo glTF (.gltf) y prepara los datos binarios.
// - 'file' debe ser la ruta al .gltf en disco. El método parsea el JSON y extrae
//   la referencia al buffer binario asociado (archivo .bin o URI embebida).
Model::Model(const char* file)
{
	// Carga el texto del archivo .gltf y lo parsea como JSON
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	// Guarda la ruta del archivo y obtiene el contenido binario referenciado por el glTF
	Model::file = file;
	data = getData();

	// Comienza el recorrido del grafo de la escena a partir del nodo raíz (índice 0)
	// Esto construirá las mallas, matrices y transformaciones del modelo
	traverseNode(0);
}

// Calcula el centro (outCenter) y el radio (outRadius) de la esfera envolvente del modelo.
// Esta esfera se usa para posicionar la cámara de forma que el modelo quepa en la vista.
void Model::computeBounds(glm::vec3& outCenter, float& outRadius)
{
	// Si no hay mallas, devolvemos valores por defecto
	if (meshes.empty())
	{
		outCenter = glm::vec3(0.0f);
		outRadius = 1.0f;
		return;
	}

	// Inicializamos cajas AABB en espacio mundo para acumular extremos
	glm::vec3 minB(FLT_MAX);
	glm::vec3 maxB(-FLT_MAX);

	// Recorremos todas las mallas y sus vértices transformados para obtener AABB en mundo
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		const std::vector<Vertex>& verts = meshes[i].vertices;
		glm::mat4 modelMat = matricesMeshes[i];
		for (const auto& v : verts)
		{
			// Transformamos la posición local del vértice al espacio mundo usando la matriz de malla
			glm::vec4 worldPos = modelMat * glm::vec4(v.position, 1.0f);
			glm::vec3 p = glm::vec3(worldPos);
			minB = glm::min(minB, p);
			maxB = glm::max(maxB, p);
		}
	}

	// Centro de la AABB -> centro aproximado de la esfera envolvente
	outCenter = (minB + maxB) * 0.5f;

	// Calculamos el radio como la distancia máxima desde el centro a cualquier vértice transformado
	outRadius = 0.0f;
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		const std::vector<Vertex>& verts = meshes[i].vertices;
		glm::mat4 modelMat = matricesMeshes[i];
		for (const auto& v : verts)
		{
			glm::vec3 p = glm::vec3(modelMat * glm::vec4(v.position, 1.0f));
			outRadius = std::max(outRadius, glm::length(p - outCenter));
		}
	}
	if (outRadius <= 0.0f) outRadius = 1.0f;
}

// Dibuja todas las mallas del modelo con el shader y la cámara indicados.
// Se aplica 'modelTransform' global y la matriz local de cada malla antes de dibujar.
void Model::Draw(Shader& shader, Camera& camera)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		// Multiplica la transformación externa del modelo por la transform de la malla
		glm::mat4 combined = modelTransform * matricesMeshes[i];
		meshes[i].Mesh::Draw(shader, camera, combined);
	}
}

void Model::loadMesh(unsigned int indMesh)
{
	// Carga una malla a partir de su índice JSON en 'meshes'.
	// Obtiene los índices de los accessors que describen posiciones, normales, UVs e índices.
	unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
	unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
	unsigned int texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
	unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

	// Usa los accessors para leer arrays planos de floats y agruparlos como vectores (vec2/vec3)
	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
	std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
	std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);
	std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
	std::vector<glm::vec2> texUVs = groupFloatsVec2(texVec);

	// Ensambla los datos leídos en una lista de vértices y lee los índices EBO
	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
	// Intenta obtener texturas específicas del material de la primitiva si existe
	std::vector<Texture> textures;
	json prim = JSON["meshes"][indMesh]["primitives"][0];
	if (prim.find("material") != prim.end())
	{
		int matIndex = prim["material"];
		textures = getTexturesForMaterial(matIndex);
	}
	if (textures.empty())
		textures = getTextures();

	// Construye la malla con el factor base de color si está definido en el material
	glm::vec4 baseColor(1.0f);
	if (prim.find("material") != prim.end())
	{
		int matIndex = prim["material"];
		if (matIndex >= 0 && matIndex < (int)JSON["materials"].size())
		{
			json mat = JSON["materials"][matIndex];
			if (mat.find("pbrMetallicRoughness") != mat.end())
			{
				json pbr = mat["pbrMetallicRoughness"];
				if (pbr.find("baseColorFactor") != pbr.end())
				{
					auto f = pbr["baseColorFactor"];
					baseColor = glm::vec4((float)f[0], (float)f[1], (float)f[2], (float)f[3]);
				}
			}
		}
	}
	meshes.push_back(Mesh(vertices, indices, textures, baseColor));
}

// Recorre el árbol de nodos del glTF de forma recursiva y construye las mallas y sus matrices.
// - 'nextNode' es el índice del nodo actual en JSON["nodes"].
// - 'matrix' es la matriz acumulada de transformación desde los padres.
void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	// Obtiene el objeto JSON del nodo
	json node = JSON["nodes"][nextNode];

	// Extrae componentes de transformación si existen (translation, rotation, scale, matrix)
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	if (node.find("translation") != node.end())
	{
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
			transValues[i] = (node["translation"][i]);
		translation = glm::make_vec3(transValues);
	}

	// Rotación en quaternion (glTF usa [x,y,z,w] pero make_quat espera [w,x,y,z])
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end())
	{
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}

	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end())
	{
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}

	// Si el nodo define una matriz completa (16 valores), la leemos y la usamos
	glm::mat4 matNode = glm::mat4(1.0f);
	if (node.find("matrix") != node.end())
	{
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		matNode = glm::make_mat4(matValues);
	}

	// Construimos matrices de transformación locales a partir de translation/rotation/scale
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Matriz transformada acumulada para este nodo (padre * node.matrix * T * R * S)
	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	// Si el nodo contiene una referencia a una malla, la cargamos y guardamos sus transformaciones
	if (node.find("mesh") != node.end())
	{
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}

	// Recurre sobre hijos si los hay, pasando la matriz acumulada actual
	if (node.find("children") != node.end())
	{
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}
}

std::vector<unsigned char> Model::getData()
{
	// Lee el archivo binario referenciado por JSON["buffers"][0].uri y devuelve sus bytes.
	// Nota: asume que la URI es relativa al directorio del archivo .gltf.
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	// Convierte la cadena de texto en un vector de bytes (unsigned char)
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor)
{
	std::vector<float> floatVec;

	// Propiedades del accessor: bufferView index, número de elementos y offset en bytes
	unsigned int buffViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	// Extrae la información del bufferView correspondiente
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Determina cuántos floats hay por elemento según el tipo (SCALAR, VEC2, VEC3, VEC4)
	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	// Itera los bytes del buffer interpretándolos como floats de 4 bytes (IEEE 754 little-endian)
	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lengthOfData = count * 4 * numPerVert;
	for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i += 4)
	{
		unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		floatVec.push_back(value);
	}

	return floatVec;
}

std::vector<GLuint> Model::getIndices(json accessor)
{
	std::vector<GLuint> indices;

	// Propiedades del accessor para índices: bufferView, número de índices, offset y tipo de componente
	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Dependiendo de componentType leemos 4 bytes (UINT), 2 bytes (USHORT) o 2 bytes con signo (SHORT)
	unsigned int beginningOfData = byteOffset + accByteOffset;
	if (componentType == 5125) // UNSIGNED_INT
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i += 4)
		{
			unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5123) // UNSIGNED_SHORT
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2)
		{
			unsigned char bytes[] = { data[i], data[i + 1] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5122) // SHORT (signed)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2)
		{
			unsigned char bytes[] = { data[i], data[i + 1] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((GLuint)value);
		}
	}

	return indices;
}

std::vector<Texture> Model::getTextures()
{
	std::vector<Texture> textures;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	// Recorre todas las entradas en JSON["images"] y carga texturas físicas en GPU
	for (unsigned int i = 0; i < JSON["images"].size(); i++)
	{
		// ruta relativa del archivo de imagen
		std::string texPath = JSON["images"][i]["uri"];

		// Comprueba si la textura ya fue cargada en la caché (loadedTexName)
		bool skip = false;
		for (unsigned int j = 0; j < loadedTexName.size(); j++)
		{
			if (loadedTexName[j] == texPath)
			{
				textures.push_back(loadedTex[j]);
				skip = true;
				break;
			}
		}

		// Si la textura no estaba en caché, la cargamos y determinamos un tipo lógico
		// (diffuse/specular) para usar en los uniforms de material
		if (!skip)
		{
			// Selecciona un tipo por defecto: 'diffuse' salvo que el nombre indique metallic/roughness
			const char* type = "diffuse";
			if (texPath.find("metallic") != std::string::npos || texPath.find("roughness") != std::string::npos || texPath.find("metallicRoughness") != std::string::npos)
				type = "specular";

			Texture t = Texture((fileDirectory + texPath).c_str(), type, loadedTex.size());
			textures.push_back(t);
			loadedTex.push_back(t);
			loadedTexName.push_back(texPath);
		}
	}

	return textures;
}

std::vector<Texture> Model::getTexturesForMaterial(int matIndex)
{
	std::vector<Texture> textures;
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	if (matIndex < 0 || matIndex >= (int)JSON["materials"].size())
		return textures;

	json material = JSON["materials"][matIndex];

	// Intentamos obtener baseColorTexture definido en pbrMetallicRoughness
	if (material.find("pbrMetallicRoughness") != material.end())
	{
		json pbr = material["pbrMetallicRoughness"];
		if (pbr.find("baseColorTexture") != pbr.end())
		{
			int texIndex = pbr["baseColorTexture"]["index"];
			if (texIndex >= 0 && texIndex < (int)JSON["textures"].size())
			{
				int imgIndex = JSON["textures"][texIndex]["source"];
				if (imgIndex >= 0 && imgIndex < (int)JSON["images"].size())
				{
					std::string uri = JSON["images"][imgIndex]["uri"];
					// Comprobamos la caché de texturas ya cargadas
					for (unsigned int j = 0; j < loadedTexName.size(); j++)
					{
						if (loadedTexName[j] == uri)
						{
							textures.push_back(loadedTex[j]);
							break;
						}
					}
					if (textures.empty())
					{
						Texture diff((fileDirectory + uri).c_str(), "diffuse", loadedTex.size());
						textures.push_back(diff);
						loadedTex.push_back(diff);
						loadedTexName.push_back(uri);
					}
				}
			}
		}
		// Texture metallicRoughness si existe
		if (pbr.find("metallicRoughnessTexture") != pbr.end())
		{
			int texIndex = pbr["metallicRoughnessTexture"]["index"];
			if (texIndex >= 0 && texIndex < (int)JSON["textures"].size())
			{
				int imgIndex = JSON["textures"][texIndex]["source"];
				if (imgIndex >= 0 && imgIndex < (int)JSON["images"].size())
				{
					std::string uri = JSON["images"][imgIndex]["uri"];
					bool found = false;
					for (unsigned int j = 0; j < loadedTexName.size(); j++)
					{
						if (loadedTexName[j] == uri)
						{
							textures.push_back(loadedTex[j]);
							found = true;
							break;
						}
					}
					if (!found)
					{
						Texture spec((fileDirectory + uri).c_str(), "specular", loadedTex.size());
						textures.push_back(spec);
						loadedTex.push_back(spec);
						loadedTexName.push_back(uri);
					}
				}
			}
		}
	}

	return textures;
}

std::vector<Vertex> Model::assembleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i]
			}
		);
	}
	return vertices;
}

std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
{
	const unsigned int floatsPerVector = 2;

	std::vector<glm::vec2> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
	{
		vectors.push_back(glm::vec2(0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++)
		{
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
{
	const unsigned int floatsPerVector = 3;

	std::vector<glm::vec3> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
	{
		vectors.push_back(glm::vec3(0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++)
		{
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
{
	const unsigned int floatsPerVector = 4;

	std::vector<glm::vec4> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
	{
		vectors.push_back(glm::vec4(0, 0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++)
		{
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}