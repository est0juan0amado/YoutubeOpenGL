#version 330 core

// Salida de color en RGBA
out vec4 FragColor;

// Recibe desde el Vertex Shader
in vec3 crntPos;   // posición en espacio mundial
in vec3 Normal;    // normal interpolada
in vec3 color;     // color de vértice
in vec2 texCoord;  // coordenadas de textura

// Samplers y factoria de color base (material)
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform vec4 baseColorFactor;

// Fragment shader: si no hay textura válida, usar color de vértice
void main()
{
	// Muestra la textura diffuse si está disponible; en caso contrario, se usará el color del vértice
	vec2 uv = texCoord;
	vec4 tex = texture(diffuse0, uv);

	// Combina la textura con el color del vértice (textura tiene prioridad)
	vec3 col = tex.rgb * color * baseColorFactor.rgb;

	// Tonemapping simple (evita que las altas luces se saturen en blanco)
	float exposure = 0.6;
	vec3 mapped = col * exposure / (col * exposure + vec3(1.0));

	// Corrección gamma (sRGB)
	mapped = pow(mapped, vec3(1.0/2.2));

	FragColor = vec4(mapped, 1.0);
}
