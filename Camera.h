#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"shaderClass.h"

class Camera
{
public:
	// Vectores principales que definen la cámara en el mundo
	// Position: posición en coordenadas del mundo
	// Orientation: dirección hacia donde mira la cámara (vector forward)
	// Up: vector 'arriba' en el espacio del mundo usado como referencia
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	// Matriz combinada de proyección * vista que se enviará al shader
	glm::mat4 cameraMatrix = glm::mat4(1.0f);

	// Evita saltos en la primera interacción con el ratón
	bool firstClick = true;

	// Dimensiones actuales de la ventana (necesarias para calcular proyección y normalizar ratón)
	int width;
	int height;

	// Parámetros de control de movimiento y sensibilidad del ratón
	float speed = 0.2f; // velocidad de movimiento (unidades por segundo)
	float sensitivity = 100.0f; // sensibilidad del ratón al rotar la cámara

	// Maneja entradas que dependen del deltaTime del frame
	void Inputs(GLFWwindow* window, float deltaTime);

	// Constructor: inicializa ancho, alto y posición
	Camera(int width, int height, glm::vec3 position);

	// Calcula y actualiza cameraMatrix usando FOV y planos near/far
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
	// Envía la cámara (cameraMatrix) a un shader mediante un uniform
	void Matrix(Shader& shader, const char* uniform);
	// Versión compatibilidad que no recibe deltaTime
	void Inputs(GLFWwindow* window);
};
#endif
