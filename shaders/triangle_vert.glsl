#version 330 core

layout (location = 0) in vec3 InPosition; // Recebidos já normalizados pelo programa
layout (location = 1) in vec3 InColor; // Recebe uma cor
layout (location = 2) in vec2 InUV; // Recebe uma textura para mapear

uniform mat4 ModelViewProjection; // Ou MVP

out vec3 Color; // Declara uma cor de saída
out vec2 UV;

void main() {
	Color = InColor;
	UV = InUV;
	gl_Position = ModelViewProjection * vec4(InPosition, 1.0); // Essa variável é vec4, por isso o casting
}