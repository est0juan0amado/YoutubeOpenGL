#include"Camera.h"



Camera::Camera(int width, int height, glm::vec3 position)
{
	// Inicializa las dimensiones de la ventana y la posición inicial de la cámara
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::Inputs(GLFWwindow* window, float deltaTime)
{
	// Calcula la velocidad de movimiento teniendo en cuenta si se mantiene la tecla SHIFT
	float actualSpeed = speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		actualSpeed *= 1.5f; // multiplicador para sprint

	// Movimiento en los ejes locales de la cámara
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Position += actualSpeed * deltaTime * Orientation; // avanzar
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Position += actualSpeed * deltaTime * -glm::normalize(glm::cross(Orientation, Up)); // izquierda (strafe)
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Position += actualSpeed * deltaTime * -Orientation; // retroceder
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Position += actualSpeed * deltaTime * glm::normalize(glm::cross(Orientation, Up)); // derecha (strafe)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		Position += actualSpeed * deltaTime * Up; // subir
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		Position += actualSpeed * deltaTime * -Up; // bajar

	// Gestión del ratón para rotar la cámara cuando el cursor está desactivado o se mantiene pulsado el botón izquierdo
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Centra el cursor la primera vez para evitar un salto brusco en la orientación
		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			mouseX = (width / 2);
			mouseY = (height / 2);
			firstClick = false;
		}

		// Convertimos el desplazamiento del ratón en ángulos de rotación (en grados)
		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		// Rotación vertical: rotamos alrededor del eje perpendicular a Orientation y Up
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));
		// Limitamos la rotación vertical para evitar que la cámara se invierta (gimbal lock visual)
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			Orientation = newOrientation;

		// Rotación horizontal: rotamos alrededor del eje Up
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);
		// Recolocamos el cursor al centro para seguir midiendo desplazamientos relativos
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else
	{
		// Si no se está rotando, permitimos que la próxima pulsación comience desde el centro
		firstClick = true;
	}
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
	// Calcula las matrices de vista y proyección y las combina en cameraMatrix
	glm::mat4 view = glm::lookAt(Position, Position + Orientation, Up); // vista desde la posición hacia Position+Orientation
	glm::mat4 projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane); // proyección en perspectiva

	// Matriz final: proyección * vista (orden importante para shaders)
	cameraMatrix = projection * view;
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
	// Envía cameraMatrix al shader en el uniform indicado
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}



void Camera::Inputs(GLFWwindow* window)
{
	// Versión de conveniencia que asume ~60 FPS si no se proporciona deltaTime
	Inputs(window, 0.016f); // ~16 ms por frame

	// Manejo del ratón cuando se pulsa el botón izquierdo
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		// Oculta el cursor para controlar la cámara por movimiento relativo
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Evita salto en la primera pulsación
		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Convertimos desplazamientos de cursor a ángulos y actualizamos la orientación
		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Mantiene el cursor en el centro para lecturas relativas continuas
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		// Si no se rota, mostramos el cursor y restauramos el estado para la próxima interacción
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}
