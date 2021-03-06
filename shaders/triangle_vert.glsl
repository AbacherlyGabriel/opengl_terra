#version 330 core

layout (location = 0) in vec3 InPosition; // Vetor de posi??es j? normalizadas recebidas pelo programa
layout (location = 1) in vec3 InNormal; // Vetor normal
layout (location = 2) in vec3 InColor; // Vetor de cores
layout (location = 3) in vec2 InUV; // Vetor de coordenadas de textura

uniform mat4 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjection;

out vec3 Position;
out vec3 Normal;
out vec3 Color;
out vec2 UV;

void main()
{  
	vec4 ViewPosition = ModelViewMatrix * vec4(InPosition, 1.0);

	Position = ViewPosition.xyz / ViewPosition.w;
	Normal = vec3(NormalMatrix * vec4(InNormal, 0.0));	
	Color = InColor;
	UV = InUV;
	gl_Position = ModelViewProjection * vec4(InPosition, 1.0);
}