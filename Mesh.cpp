#include "Mesh.h"

// Constructor: recibe los vértices, índices, texturas y factor de color base del material
Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures, glm::vec4 baseColorFactor)
{
	// Guarda los datos en los miembros de la clase
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	// Guarda el factor de color base del material (RGBA)
	Mesh::baseColorFactor = baseColorFactor;

	VAO.Bind();
	// Crea VBO y EBO y configura atributos del VAO
	VBO VBO(vertices);
	EBO EBO(indices);
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// Desvinculamos para evitar modificaciones accidentales
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

// Dibuja la malla usando el shader y cámara proporcionados, aplicando transformaciones de modelo
void Mesh::Draw
(
	// Recibe el shader, cámara, matriz de modelo y transformaciones de traslación, rotación y escala
	Shader& shader,
	Camera& camera,
	glm::mat4 matrix,
	glm::vec3 translation,
	glm::quat rotation,
	glm::vec3 scale
)
{
	// Activa el shader y vincula el VAO para dibujar
	shader.Activate();
	VAO.Bind();

	// Contadores para sufijos de uniforms de texturas (diffuse0, specular0, ...)
	// El shader espera samplers nombrados diffusionX/specularX; aquí asociamos cada textura a su unidad
	unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;
		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
		}
		// Asigna la unidad de textura y la enlaza
		textures[i].texUnit(shader, (type + num).c_str(), i);
		textures[i].Bind();
	}
	// Envía posición de cámara y matriz de cámara al shader
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(shader, "camMatrix");

	// Envía factor de color base del material
	glUniform4f(glGetUniformLocation(shader.ID, "baseColorFactor"), baseColorFactor.x, baseColorFactor.y, baseColorFactor.z, baseColorFactor.w);

	// Preparamos matrices de transformación (traslación, rotación, escala)
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Subimos las matrices al shader
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(trans));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(rot));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(sca));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));

	// Dibuja la malla usando los índices cargados en el EBO
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
