#include"VAO.h"

// Constructor: genera el identificador del VAO
// Un VAO encapsula la configuración de atributos de vértices y los vincula a los VBOs.
VAO::VAO()
{
	glGenVertexArrays(1, &ID);
}

// Enlaza un atributo del VBO al layout del VAO
void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
	VBO.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

// Vincula el VAO
void VAO::Bind()
{
	glBindVertexArray(ID);
}

// Desvincula el VAO
void VAO::Unbind()
{
	glBindVertexArray(0);
}

// Elimina el VAO de OpenGL
void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}
