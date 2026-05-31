#version 330 core

// Vertex shader para renderizar la curva
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTex;

out vec3 vColor;// variable de salida para el color interpolado

// Matrices de transformación
uniform mat4 camMatrix;
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;

// Función principal del shader
void main()
{
// calcula la posición en espacio mundial (sin transformaciones de iluminación)
	vec3 pos = vec3(model * translation * -rotation * scale * vec4(aPos, 1.0));
	vColor = aColor;
	gl_Position = camMatrix * vec4(pos, 1.0);
}
