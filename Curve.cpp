#include "Curve.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// Implementación de De Casteljau para curvas Bezier (optimizado para el caso cúbico)
// Devuelve la posición en la curva para un parámetro t en [0,1].
// - Si hay exactamente 4 puntos de control se usa la forma cerrada cúbica (fórmula polinómica).
// - Para cualquier otro número de puntos se aplica el algoritmo iterativo de De Casteljau.
glm::vec3 Curve::evaluateBezier(const std::vector<glm::vec3>& ctrl, float t) const
{
	// Caso común: 4 puntos (cúbico). Si no es cúbico, usar el algoritmo general iterativo
	if (ctrl.size() == 4)
	{
		float u = 1.0f - t;
		return u*u*u*ctrl[0] + 3.0f*u*u*t*ctrl[1] + 3.0f*u*t*t*ctrl[2] + t*t*t*ctrl[3];
	}
	// De Casteljau general (iterativo) para cualquier número de puntos de control
	std::vector<glm::vec3> tmp = ctrl;
	int n = (int)tmp.size();
	for (int r = 1; r < n; ++r)
	{
		for (int i = 0; i < n - r; ++i)
		{
			tmp[i] = tmp[i] * (1.0f - t) + tmp[i + 1] * t;
		}
	}
	return tmp[0];
}

Curve::Curve(const std::vector<glm::vec3>& controlPoints, const glm::vec3& worldOffset, float radius, int ringSegments, int samples)
{
	// Validación y ajustes de parámetros para evitar resultados inválidos
	if (ringSegments < 3) ringSegments = 12;
	if (samples < 8) samples = 256;
	if (radius <= 0.0f) radius = 0.03f;

	// Guardamos los puntos de control
	Curve::controlPoints = controlPoints;

	// Muestreamos la curva en 'samples' posiciones y calculamos las tangentes por diferencias finitas.
	// Este muestreo genera una lista de posiciones en espacio mundo (ya con worldOffset aplicado)
	// que luego usaremos para construir anillos de vértices alrededor de la curva (tubo).
	std::vector<glm::vec3> positions;
	positions.reserve(samples);
	for (int i = 0; i < samples; ++i)
	{
		float t = i / float(samples - 1);
		positions.push_back(evaluateBezier(controlPoints, t) + worldOffset);
	}

	// Tangentes aproximadas (central differences cuando es posible)
	// Las tangentes definen la dirección local (tangent vector) en cada muestra y son
	// críticas para construir un frame local (T, N, B) que orienta el anillo transversal del tubo.
	std::vector<glm::vec3> tangents(samples);
	for (int i = 0; i < samples; ++i)
	{
		if (i == 0) tangents[i] = glm::normalize(positions[i+1] - positions[i]);
		else if (i == samples-1) tangents[i] = glm::normalize(positions[i] - positions[i-1]);
		else tangents[i] = glm::normalize(positions[i+1] - positions[i-1]);
	}

	// Construcción del tubo: para cada muestra construimos un anillo de vértices usando un frame local
	// - T: tangente (forward)
	// - N: un vector normal ortogonal a T (se elige a partir de 'up' y se normaliza)
	// - B: binormal (completando la base ortonormal T,N,B)
	// Elegimos inicialmente 'up' como (0,1,0) pero si T es casi colineal con up buscamos un eje alternativo
	// para evitar un frame degenerado.
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	for (int i = 0; i < samples; ++i)
	{
		glm::vec3 T = tangents[i];
		glm::vec3 N = glm::normalize(glm::cross(up, T));
		// Si N es casi cero (T casi colineal con up), usamos otro eje para evitar degeneración.
		// Esto evita giros bruscos en el frame y artefactos en la malla del tubo.
		if (glm::length(N) < 0.001f) N = glm::normalize(glm::cross(glm::vec3(1,0,0), T));
		glm::vec3 B = glm::normalize(glm::cross(T, N));

		for (int s = 0; s < ringSegments; ++s)
		{
			// Para cada segmento angular 'a' construimos un offset en el plano normal (N,B)
			// que define la circunferencia del anillo en esa muestra.
			float a = (float)s / ringSegments * glm::two_pi<float>();
			glm::vec3 offset = (glm::cos(a) * N + glm::sin(a) * B) * radius;
			LineVertex v;
			// Posición del vértice en espacio mundo
			v.position = positions[i] + offset;
			// Normal aproximada del vértice (orientada radialmente desde el centro del tubo)
			v.normal = glm::normalize(offset);
			// Color por defecto (se usa para el shader 'plain' de la curva)
			v.color = glm::vec3(1.0f, 1.0f, 0.0f); // amarillo
			// Coordenadas UV simples: u = segmento angular, v = índice de muestra a lo largo de la curva
			v.texUV = glm::vec2(s / float(ringSegments), i / float(samples));
			vertices.push_back(v);
		}
	}

	// Índices que forman triángulos entre anillos adyacentes
	// Cada par de anillos consecutivos se conecta mediante quads (dos triángulos por segmento angular).
	std::vector<GLuint> indices;
	for (int i = 0; i < samples - 1; ++i)
	{
		for (int s = 0; s < ringSegments; ++s)
		{
			int cur = i * ringSegments + s;
			int next = (i+1) * ringSegments + s;
			int curNext = i * ringSegments + ((s+1) % ringSegments);
			int nextNext = (i+1) * ringSegments + ((s+1) % ringSegments);

			// Dos triángulos por segmento entre anillos (forman un quad cuando se juntan)
			indices.push_back(cur);
			indices.push_back(next);
			indices.push_back(curNext);

			indices.push_back(curNext);
			indices.push_back(next);
			indices.push_back(nextNext);
		}
	}

	// Subimos los datos a GPU: convertimos LineVertex -> Vertex (estructura usada por VBO)
	// La conversión mantiene la información esencial (posición, normal, color, UV)
	std::vector<Vertex> vtx;
	vtx.reserve(vertices.size());
	for (auto &lv : vertices) {
		Vertex vv;
		vv.position = lv.position;
		vv.normal = lv.normal;
		vv.color = lv.color;
		vv.texUV = lv.texUV;
		vtx.push_back(vv);
	}

	VBO buffer(vtx);
	ebo = new EBO(indices);

	// Configuramos VAO/VBO/EBO para dibujar la malla del tubo en draw calls posteriores
	VAO.Bind();
	VAO.LinkAttrib(buffer, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(buffer, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.LinkAttrib(buffer, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttrib(buffer, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	VAO.Unbind();
	buffer.Unbind();

	vertexCount = (int)vertices.size();
	indexCount = (int)indices.size();
}

// Devuelve la posición en la curva para un parámetro t en [0,1] usando la función de evaluación de Bezier
glm::vec3 Curve::getPoint(float t) const { return evaluateBezier(controlPoints, glm::clamp(t, 0.0f, 1.0f)); }
glm::vec3 Curve::getTangent(float t) const { float eps = 0.001f; float ta = glm::clamp(t - eps, 0.0f, 1.0f); float tb = glm::clamp(t + eps, 0.0f, 1.0f); return glm::normalize(evaluateBezier(controlPoints, tb) - evaluateBezier(controlPoints, ta)); }
void Curve::getPointAndTangent(float t, glm::vec3& outPoint, glm::vec3& outTangent) const { outPoint = getPoint(t); outTangent = getTangent(t); }

Curve::~Curve()
{
	VAO.Delete();
	if (ebo) { ebo->Delete(); delete ebo; ebo = nullptr; }
}

// Dibuja la malla tubular con iluminación y texturas (si existieran)
void Curve::Draw(Shader& shader, Camera& camera)
{
	shader.Activate();
	camera.Matrix(shader, "camMatrix");

	// Usamos matrices identidad: la curva ya está en posiciones de mundo cuando se construyó
	glm::mat4 I = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(I));

	// Dibuja la malla del tubo usando los índices subidos al EBO
	VAO.Bind();
	if (ebo) ebo->Bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	if (ebo) ebo->Unbind();
	VAO.Unbind();
}

// Dibuja la curva con un shader sencillo que usa el color de vértice (ignora iluminación)
void Curve::DrawPlain(Shader& plainShader, Camera& camera)
{
	// Usa un shader plano que ignora iluminación/texturas y dibuja el color por vértice
	plainShader.Activate();
	camera.Matrix(plainShader, "camMatrix");

	glm::mat4 I = glm::mat4(1.0f);// La curva ya está en posiciones de mundo, así que usamos matrices identidad para model/translation/rotation/scale
	glUniformMatrix4fv(glGetUniformLocation(plainShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(glGetUniformLocation(plainShader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(glGetUniformLocation(plainShader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(glGetUniformLocation(plainShader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(I));

	VAO.Bind();
	if (ebo) ebo->Bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	if (ebo) ebo->Unbind();
	VAO.Unbind();
}