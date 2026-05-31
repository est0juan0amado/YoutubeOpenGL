#version 330 core

// Posiciones/Coordenadas
layout (location = 0) in vec3 aPos;
// Normales (no necesariamente normalizadas)
layout (location = 1) in vec3 aNormal;
// Colores
layout (location = 2) in vec3 aColor;
// Coordenadas de textura
layout (location = 3) in vec2 aTex;


// Pasa la posición actual al Fragment Shader
out vec3 crntPos;
// Pasa la normal al Fragment Shader
out vec3 Normal;
// Pasa el color al Fragment Shader
out vec3 color;
// Pasa las coordenadas de textura al Fragment Shader
out vec2 texCoord;



// Uniform: matriz de cámara (proyección * vista)
uniform mat4 camMatrix;
// Uniforms: matrices de transformación
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;


void main()
{
	// calculates current position
	crntPos = vec3(model * translation * -rotation * scale * vec4(aPos, 1.0));
	// Assigns the normal from the Vertex Data to "Normal"
	Normal = aNormal;
	// Assigns the colors from the Vertex Data to "color"
	color = aColor;
	// Assigns the texture coordinates from the Vertex Data to "texCoord"
	texCoord = aTex;
	
	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * vec4(crntPos, 1.0);
}