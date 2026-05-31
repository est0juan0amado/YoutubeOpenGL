#version 330 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
	// dibuja color sólido sin influencia de iluminación o texturas
	FragColor = vec4(vColor, 1.0);
}
