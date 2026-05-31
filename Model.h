#pragma once
#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

// Utiliza la cabecera única nlohmann json incluida localmente
#include "json/json.hpp"
#include "Mesh.h"

using json = nlohmann::json;

class Model
{
public:
	// Carga un modelo glTF (.gltf) y mantiene su JSON y datos binarios
	Model(const char* file);

	// Dibuja todas las mallas del modelo con la cámara y shader provistos
	void Draw(Shader& shader, Camera& camera);

	// Transformación externa que se aplica a todo el modelo (útil para posicionar/orientar)
	glm::mat4 modelTransform = glm::mat4(1.0f);
	// Calcula una esfera envolvente del modelo (centro en espacio mundial y radio)
	void computeBounds(glm::vec3& outCenter, float& outRadius);

private:
	const char* file; // ruta del archivo
	std::vector<unsigned char> data; // datos binarios (.bin)
	json JSON; // representación JSON del .gltf

	// Mallas y sus transformaciones locales
	std::vector<Mesh> meshes;
	std::vector<glm::vec3> translationsMeshes;
	std::vector<glm::quat> rotationsMeshes;
	std::vector<glm::vec3> scalesMeshes;
	std::vector<glm::mat4> matricesMeshes;

	// Cache para evitar recargar texturas duplicadas
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;

	// Carga una malla por su índice en el JSON
	void loadMesh(unsigned int indMesh);
	// Obtiene las texturas asociadas a un material
	std::vector<Texture> getTexturesForMaterial(int matIndex);

	// Recorre nodos recursivamente y construye mallas y matrices
	void traverseNode(unsigned int nextNode, glm::mat4 matrix = glm::mat4(1.0f));

	// Lectura e interpretación de datos binarios
	std::vector<unsigned char> getData();
	std::vector<float> getFloats(json accessor);
	std::vector<GLuint> getIndices(json accessor);
	std::vector<Texture> getTextures();

	// Ensambla posiciones/normales/UVs en una lista de vértices
	std::vector<Vertex> assembleVertices
	(
		std::vector<glm::vec3> positions,
		std::vector<glm::vec3> normals,
		std::vector<glm::vec2> texUVs
	);

	// Helpers para agrupar floats planos en vectores de tamaño adecuado
	std::vector<glm::vec2> groupFloatsVec2(std::vector<float> floatVec);
	std::vector<glm::vec3> groupFloatsVec3(std::vector<float> floatVec);
	std::vector<glm::vec4> groupFloatsVec4(std::vector<float> floatVec);
};
#endif
