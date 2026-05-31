#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "shaderClass.h"
#include "Camera.h"

// Vértice usado para dibujar el tubo que sigue la curva
struct LineVertex {
	glm::vec3 position; // posición en el espacio
	glm::vec3 normal;   // normal para iluminación
	glm::vec3 color;    // color del vértice (utilizado por shader simple)
	glm::vec2 texUV;    // coordenadas de textura
};

class Curve
{
public:
	// Construye una curva tubular a partir de puntos de control Bezier
	// worldOffset desplaza toda la curva en el mundo, radius controla el grosor,
	// ringSegments el número de segmentos por anillo y samples cuántas muestras a lo largo de la curva
	Curve(const std::vector<glm::vec3>& controlPoints, const glm::vec3& worldOffset = glm::vec3(0.0f), float radius = 0.03f, int ringSegments = 12, int samples = 256);
	~Curve();

	// Dibuja la malla del tubo con un shader que soporta iluminación/texturas
	void Draw(Shader& shader, Camera& camera);
	// Dibuja la malla usando un shader simple que usa solo el color de vértice (sin iluminación avanzada)
	void DrawPlain(Shader& plainShader, Camera& camera);

	// Muestra punto y tangente en la curva para t en [0,1]
	glm::vec3 getPoint(float t) const;
	glm::vec3 getTangent(float t) const;
	void getPointAndTangent(float t, glm::vec3& outPoint, glm::vec3& outTangent) const;

private:
	std::vector<LineVertex> vertices;
	VAO VAO;
	EBO* ebo = nullptr;
	int vertexCount = 0;
	int indexCount = 0;

	// Evalúa el polinomio de Bezier para los puntos de control
	glm::vec3 evaluateBezier(const std::vector<glm::vec3>& ctrl, float t) const;
	std::vector<glm::vec3> controlPoints;
};
