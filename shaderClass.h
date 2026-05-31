#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

// Lee todo el contenido de un archivo de texto y devuelve una cadena
// Uso: get_file_contents("shader.vert") -> devuelve el contenido del archivo como std::string
std::string get_file_contents(const char* filename);

class Shader
{
public:
	// Identificador del programa de shaders compilado
	GLuint ID;
	// Constructor: compila y enlaza un vertex y fragment shader a partir de sus rutas
	Shader(const char* vertexFile, const char* fragmentFile);

	// Activa el shader para uso (glUseProgram)
	void Activate();
	// Elimina el programa de shader
	void Delete();
private:
	// Comprueba errores de compilación/enlace y los muestra por consola
	void compileErrors(unsigned int shader, const char* type);
};


#endif
