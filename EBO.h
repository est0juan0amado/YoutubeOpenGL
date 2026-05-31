#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include<glad/glad.h>
#include<vector>

class EBO
{
public:
	// Identificador del Element Buffer Object (índices)
	GLuint ID;
	// Constructor: crea el buffer de índices y sube los datos a GPU
	EBO(std::vector<GLuint>& indices);

	// Vincula el EBO como GL_ELEMENT_ARRAY_BUFFER
	void Bind();
	// Desvincula el EBO
	void Unbind();
	// Elimina el objeto OpenGL asociado
	void Delete();
};

#endif
