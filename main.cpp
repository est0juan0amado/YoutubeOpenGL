#// Proyecto: YoutubeOpenGL
#// Archivo: main.cpp
#// Propósito: punto de entrada de la aplicación. Inicializa GLFW/GLAD, crea la cámara,
#// carga el modelo glTF, construye la curva Bézier y ejecuta el bucle principal de render.
#// Contiene la lógica de gestión de ventana, cálculo de FOV dinámico y animación simple.
//------- Ignorar esto ----------
#include<filesystem>
namespace fs = std::filesystem;
//------------------------------

#include"Model.h"
#include"Curve.h"
#include <glm/gtc/constants.hpp>


const unsigned int width = 800;
const unsigned int height = 800;


int main()
{
	// Inicializa GLFW (librería para ventana y contexto OpenGL)
	// Crea el contexto OpenGL, la ventana y gestiona eventos del sistema
	glfwInit();

	// Indica a GLFW la versión de OpenGL requerida
	// En este proyecto se solicita OpenGL 3.3 (perfil core)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Indicamos a GLFW que use el perfil CORE
	// Esto habilita las funciones modernas de OpenGL
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Crea la ventana GLFW (800x800) y su contexto OpenGL asociado
	GLFWwindow* window = glfwCreateWindow(width, height, "YoutubeOpenGL", NULL, NULL);
	// Comprobación de error: si la ventana no se crea, salir de la aplicación
	if (window == NULL)
	{
		std::cout << "No se pudo crear la ventana GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Hace la ventana y su contexto OpenGL actuales en este hilo
	glfwMakeContextCurrent(window);

	// Preparación para permitir que los callbacks de la ventana accedan a la cámara
	// El puntero de usuario se asignará después de crear el objeto Camera

	// Carga GLAD para resolver las entradas de OpenGL
	gladLoadGL();
	// Configura el viewport de OpenGL en la ventana
	// En este caso el viewport cubre x = 0..800, y = 0..800
	glViewport(0, 0, width, height);





// Genera el objeto Shader usando los archivos default.vert y default.frag
Shader shaderProgram("default.vert", "default.frag");





	// Habilita la prueba de profundidad (Depth Test)
	glEnable(GL_DEPTH_TEST);

	// Crea el objeto Camera
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));
	// establece las dimensiones en la cámara y adjunta el puntero para callbacks
	glfwSetWindowUserPointer(window, &camera);
	// Orienta inicialmente la cámara hacia el origen (centro esperado del modelo)
	camera.Orientation = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - camera.Position);


	/*
	* Uso rutas relativas para centralizar recursos en una carpeta y evitar duplicados
	* (requiere C++17: Project Properties -> C/C++ -> Language -> C++17)
	*/
	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string modelPath = "/YoutubeOpenGL/models/sword/scene.gltf";

	// Carga un modelo desde disco
	Model model((parentDir + modelPath).c_str());

	// (la curva se creará tras calcular los bounds del modelo)

	// Calcula el centro y radio del modelo para posicionar la curva y la cámara adecuadamente
	glm::vec3 modelCenter;
	float modelRadius;
	model.computeBounds(modelCenter, modelRadius);
	// Crea una curva Bézier cúbica con forma de arco usando puntos de control P0..P3
	std::vector<glm::vec3> ctrl;
	float length = modelRadius * 2.5f; // span of the curve
	// extremos P0 (izquierda) y P3 (derecha)
	glm::vec3 P0 = modelCenter + glm::vec3(-length * 0.5f, 0.0f, 0.0f);
	glm::vec3 P3 = modelCenter + glm::vec3(length * 0.5f, 0.0f, 0.0f);
	// manejadores de control por encima de los extremos para formar el arco
	float handleHeight = modelRadius * 0.9f; // controls how high the arch is
	glm::vec3 P1 = P0 + glm::vec3(length * 0.25f, handleHeight, 0.0f);
	glm::vec3 P2 = P3 + glm::vec3(-length * 0.25f, handleHeight * 0.9f, 0.0f);

	ctrl.push_back(P0);
	ctrl.push_back(P1);
	ctrl.push_back(P2);
	ctrl.push_back(P3);

	// instancia el tubo a lo largo de la curva (ligeramente más grueso para destacar)
	Curve bezier(ctrl, glm::vec3(0.0f), modelRadius * 0.04f, 28, 512);

	// carga shader simple para la tubería (color sólido)
	Shader plainShader("curve.vert", "curve.frag");

	// Coloca la cámara en modelCenter + offset en Z para que el modelo quepa en la vista
	float distance = modelRadius * 3.2f; // retrocede para que modelo y curva quepan cómodamente
	camera.Position = modelCenter + glm::vec3(0.0f, 0.0f, distance);
	camera.Orientation = glm::normalize(modelCenter - camera.Position);

	// Código original del tutorial (ejemplo comentado)
	// Model model("models/bunny/scene.gltf");

	// Bucle principal de la aplicación
	while (!glfwWindowShouldClose(window))
	{
		// Especifica el color de fondo
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Limpia el color y el buffer de profundidad
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Maneja la entrada de la cámara (teclado + ratón)
		camera.Inputs(window);
		// Ajusta dinámicamente viewport y FOV para que el modelo quepa al redimensionar/pasara a fullscreen
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		if (w <= 0) w = 1; if (h <= 0) h = 1;
		glViewport(0, 0, w, h);
		camera.width = w; camera.height = h;
		// Calcula la distancia cámara-centro del modelo y deriva un FOV vertical para encuadrarlo
		float camDist = glm::length(camera.Position - modelCenter);
		float safeRadius = glm::max(modelRadius, 0.001f);
		// ángulo subtendido por el radio: 2*asin(r/d)
		float theta = 2.0f * asinf(glm::min(safeRadius / camDist, 0.999f));
		float fovRad = theta * 1.25f; // add margin
		float fovDeg = glm::degrees(fovRad);
		// Limita el FOV a valores razonables
		if (fovDeg < 20.0f) fovDeg = 20.0f;
		if (fovDeg > 80.0f) fovDeg = 80.0f;
		camera.updateMatrix(fovDeg, 0.01f, 500.0f);

		// Anima la espada a lo largo de la curva: calcula t normalizado a partir del tiempo
		static float timeAccum = 0.0f;
		static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;
		if (deltaTime <= 0.0f) deltaTime = 0.001f;
		if (deltaTime > 0.1f) deltaTime = 0.1f; // limita para evitar saltos grandes
		// velocidad (fracciones por segundo a lo largo de la curva). Menor = más lento
		float speed = 0.12f; // movimiento más lento
		timeAccum += speed * deltaTime;
		if (timeAccum > 1.0f) timeAccum -= 1.0f;
		// obtiene posición y tangente en la curva
		glm::vec3 pos, tan;
		bezier.getPointAndTangent(timeAccum, pos, tan);

		// calcula la orientación: alinea el forward local (Z+) del modelo con la tangente
		glm::vec3 forward = glm::normalize(tan);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(up, forward));
		glm::vec3 correctedUp = glm::normalize(glm::cross(forward, right));

		// Construye la matriz de modelo a partir de la orientación y posición calculadas
		glm::mat4 rotMat(1.0f);
		rotMat[0] = glm::vec4(right, 0.0f);
		rotMat[1] = glm::vec4(correctedUp, 0.0f);
		rotMat[2] = glm::vec4(forward, 0.0f);

		// Aplica la traslación a la posición en la curva
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
		model.modelTransform = trans * rotMat * scale;

		// Dibuja el modelo
		model.Draw(shaderProgram, camera);

		// Dibuja la curva encima usando shader simple para que aparezca en amarillo puro
		bezier.DrawPlain(plainShader, camera);

		// Intercambia el back buffer con el front buffer
		glfwSwapBuffers(window);
		// Procesa todos los eventos de GLFW
		glfwPollEvents();
	}



	// Elimina los objetos creados
	shaderProgram.Delete();
	// Destruye la ventana antes de terminar
	glfwDestroyWindow(window);
	// Termina GLFW antes de finalizar la aplicación
	glfwTerminate();
	return 0;
}