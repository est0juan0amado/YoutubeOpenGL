#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include<glad/glad.h>
#include<stb/stb_image.h>

#include"shaderClass.h"

class Texture
{
public:
	GLuint ID; // ID OpenGL de la textura
	const char* type; // tipo lógico ("diffuse", "specular", ...)
	GLuint unit; // unidad de textura asignada

	// Carga una imagen y crea una textura OpenGL en la unidad 'slot'.
	// Nota: se preserva la convención de UV de glTF (no se voltea la imagen al cargar).
	Texture(const char* image, const char* texType, GLuint slot);

	// Asigna al shader la unidad de textura (setea el uniform sampler)
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	// Vincula la textura para uso en dibujo
	void Bind();
	// Desvincula la textura
	void Unbind();
	// Elimina la textura de OpenGL
	void Delete();
};
#endif
