#version 330 core

in vec3 Color;
in vec2 UV;

uniform sampler2D TextureSampler;

out vec4 OutColor;

void main() {
	vec3 TextureColor = texture(TextureSampler, UV).rgb;
	//OutColor = vec4(TextureColor, 1.0);
	vec3 FinalColor = Color * TextureColor;
	OutColor = vec4(FinalColor, 1.0);
}