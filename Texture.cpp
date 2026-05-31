#include"Texture.h"

Texture::Texture(const char* image, const char* texType, GLuint slot)
{
	// Asigna el tipo lógico de la textura
	type = texType;

	// Anchura, altura y número de canales de la imagen
	int widthImg, heightImg, numColCh;
	// No volteamos la imagen al cargar para respetar la convención de UV de glTF
	stbi_set_flip_vertically_on_load(false);
	// Carga la imagen desde disco
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	// Genera el objeto de textura en OpenGL
	glGenTextures(1, &ID);
	// Activa la unidad de textura y vincula
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;
	glBindTexture(GL_TEXTURE_2D, ID);

	// Configuración de filtrado: linear y mipmaps para suavizado
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Comportamiento de repetición de UVs
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Selección de formato interno según canales de la imagen.
	// Se usa sRGB para corregir gamma automáticamente en el pipeline de fragment shader cuando corresponda.
	if (numColCh == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	else if (numColCh == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, widthImg, heightImg, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
	else if (numColCh == 1)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, widthImg, heightImg, 0, GL_RED, GL_UNSIGNED_BYTE, bytes);
	else
		throw std::invalid_argument("Automatic Texture type recognition failed");

	// Genera mipmaps para niveles de detalle y filtrado eficiente
	glGenerateMipmap(GL_TEXTURE_2D);

	// Libera los datos en CPU una vez subidos a GPU
	stbi_image_free(bytes);

	// Desvincula la textura para evitar modificaciones accidentales
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Activa el shader, obtiene la ubicación del sampler y asigna la unidad de textura al uniform.
	// Obtiene la ubicación del uniform sampler en el shader y lo asigna a la unidad
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(texUni, unit);
	Texture::unit = unit;
}

void Texture::Bind()
{
	// Activa la unidad de textura configurada (GL_TEXTURE0 + unit) y vincula esta textura al objetivo GL_TEXTURE_2D.
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}
