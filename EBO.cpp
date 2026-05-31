#include"EBO.h"

// Constructor: crea el EBO y sube los índices al buffer de elementos (GPU)
EBO::EBO(std::vector<GLuint>& indices)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

// Vincula el EBO para que los llamados de dibujo usen estos índices
void EBO::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

// Desvincula el EBO
void EBO::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Elimina el recurso OpenGL asociado al EBO
void EBO::Delete()
{
	glDeleteBuffers(1, &ID);
}
