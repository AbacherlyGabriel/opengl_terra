#version 330 core

in vec3 Position;
in vec3 Normal;
in vec3 Color;
in vec2 UV;

uniform vec3 LightDirection;
uniform float LightIntensity;

uniform float Time;

uniform sampler2D EarthTexture;
uniform sampler2D CloudsTexture;

uniform vec2 CloudsRotationSpeed = vec2(0.008, 0.00);

out vec4 OutColor;

void main()
{
	// Renormalizar a normal: a interpola��o do vertex shader para o fragment shader � linear, assim evitamos problemas
	vec3 N = normalize(Normal);

	// Inverter a dire��o da luz para calcular o vetor L (Lambertiano)
	vec3 L = -normalize(LightDirection);
	
	// Dot - produto escalar entre dois vetores unit�rios � equivalente ao cosseno entre esses vetores
	// Quanto maior o �ngulo entre os vetores, menor � o cosseno entre eles
	float Lambertian = dot(N, L);

	// O clamp vai manter o valor do Lambertiano entre 0 e 1
	// Se o valor for negativa isso quer dizer que estamos virados pro lado
	// oposto a dire��o da luz.
	Lambertian = clamp(Lambertian, 0.0, 1.0);

	float SpecularReflection = 0.0;
	if (Lambertian > 0.0)
	{
	    // Vetor V
		// C�mera olhando constantemente para o ponto (0,0) com z negativo (para dentro da tela)
		vec3 ViewDirection = -normalize(Position);

		// Vetor R que determina a dire��o da reflex�o
		vec3 ReflectionDirection = reflect(-L, N);

		// Termo especular: (R . V) ^ alpha - produto escalar dos vetores R e V
		SpecularReflection = pow(dot(ReflectionDirection, ViewDirection), 50.0);

		// Limita o valor da reflex�o especular a n�meros positivos
		SpecularReflection = max(0.0, SpecularReflection);
	}

	vec3 EarthSurfaceColor = texture(EarthTexture, UV).rgb;
	vec3 CloudColor = texture(CloudsTexture, UV + Time * CloudsRotationSpeed).rgb;

	vec3 SurfaceColor = EarthSurfaceColor + CloudColor;

	// A reflex�o difusa � o produto do lambertiano com a intensidade da luz e a cor da textura
	// Simplifica��o da Equa��o de Phong
	vec3 DiffuseReflection = Lambertian * LightIntensity * SurfaceColor + SpecularReflection;

	OutColor = vec4(DiffuseReflection, 1.0);
}