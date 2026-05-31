#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>

// Estructura estándar para vértices usados en las mallas. Mantener en sincronía con layout en shaders.
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

class VBO
{
public:
	// Identificador del Vertex Buffer Object
	GLuint ID;
	// Constructor: crea un VBO y sube el vector de vértices a GPU
	VBO(std::vector<Vertex>& vertices);

	// Vincula el VBO como GL_ARRAY_BUFFER
	void Bind();
	// Desvincula el VBO
	void Unbind();
	// Elimina el VBO
	void Delete();
};

#endif