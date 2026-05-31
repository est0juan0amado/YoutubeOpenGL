#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include<glad/glad.h>
#include"VBO.h"

class VAO
{
public:
	// Identificador del Vertex Array Object
	GLuint ID;
	// Constructor: crea el VAO
	VAO();

	// Enlaza un atributo del VBO (posición, normal, etc.) al layout index especificado
	// layout: ubicación del atributo en el shader (layout(location = X) in vec3 ...)
	void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	// Vincula el VAO actual
	void Bind();
	// Desvincula el VAO
	void Unbind();
	// Elimina el VAO
	void Delete();
};

#endif
