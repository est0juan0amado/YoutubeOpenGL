#include"shaderClass.h"

// Lee todo el contenido de un archivo y devuelve una cadena con su contenido
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	// Si falla la apertura del archivo se lanza errno para que el llamador lo gestione
	throw(errno);
}

// Constructor: compila y enlaza un vertex shader y un fragment shader a partir de archivos fuente
// Pasos:
// 1) Lee los archivos fuente
// 2) Compila cada shader individualmente
// 3) Crea un programa, adjunta ambos shaders y realiza el enlace (link)
// 4) Elimina los objetos shader individuales (ya están enlazados en el programa)
Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	// Carga el código fuente de los archivos
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	// Convertimos a C strings para OpenGL
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Compila vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	// Compila fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT");

	// Crea programa y enlaza ambos shaders
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	// Los objetos shader individuales ya no son necesarios después de enlazar
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

// Activa el programa de shader (glUseProgram)
void Shader::Activate()
{
	glUseProgram(ID);
}

// Elimina el programa de shader de OpenGL
void Shader::Delete()
{
	glDeleteProgram(ID);
}

// Comprueba y muestra errores de compilación (VERTEX/FRAGMENT) o de enlace (PROGRAM)
void Shader::compileErrors(unsigned int shader, const char* type)
{
	GLint hasCompiled;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR_COMPILACION_SHADER (" << type << "):\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR_ENLACE_SHADER (" << type << "):\n" << infoLog << std::endl;
		}
	}
}
