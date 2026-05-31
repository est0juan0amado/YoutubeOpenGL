#include"VBO.h"

// Constructor: crea el VBO y sube los datos de vértices a GPU
// Nota: usa GL_STATIC_DRAW porque los datos de vértices no cambian dinámicamente en este proyecto.
VBO::VBO(std::vector<Vertex>& vertices)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

// Vincula el VBO
void VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

// Desvincula el VBO
void VBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Elimina el VBO
void VBO::Delete()
{
	glDeleteBuffers(1, &ID);
}
