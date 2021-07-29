#include <iostream>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


// Fun��o para demonstra��o de exemplos de constru��o de vetores
void Constructors() {
	std::cout << std::endl;

	glm::vec2 Point0{ 10.0f, 10.0f }; // Menor vetor, com duas coordenadas
	glm::vec3 Point1{ 10.0f, 0.0f, 0.0f }; // Vetor mais comum de se utilizar com GLM (coordenadas x, y, z)
	glm::vec4 Point2{ 10.0f, 0.0f, 0.0f, 10.0f }; // Vetor quadridimensional (coordenadas x, y, z, w)
	glm::ivec2 Point3{ 3, 4 }; // Armazena inteiros
	glm::bvec2 Point4{ false, true }; // Armazena booleanos
	glm::dvec2 Point5{ 10.0f, 10.0f }; // Armazena doubles
	glm::vec3 Point6{ Point0, 1.0f }; // � poss�vel declarar um vetor de maior dimens�o partindo de um de menor
	glm::vec2 Point7{ Point2 }; // De maior para menor tamb�m, fazendo o de/para das coordenadas x e y nesse caso

	// O C++ n�o possui esse tratamento de vetores naturalmente, o GLM tenta aproximar para o C++ a programa��o
	//	realizada via GLSL (programa��o utiliza para os shaders) 

	std::cout << "Point0: " << glm::to_string(Point0) << std::endl;
	std::cout << "Point1: " << glm::to_string(Point1) << std::endl;
	std::cout << "Point2: " << glm::to_string(Point2) << std::endl;
	std::cout << "Point3: " << glm::to_string(Point3) << std::endl;
	std::cout << "Point4: " << glm::to_string(Point4) << std::endl;
	std::cout << "Point5: " << glm::to_string(Point5) << std::endl;
	std::cout << "Point6: " << glm::to_string(Point6) << std::endl;
	std::cout << "Point7: " << glm::to_string(Point7) << std::endl;
}

// Fun��o para exemplos de acesso a componentes de vetores
void Components() {
	std::cout << std::endl;

	glm::vec3 P{ 1, 2, 3 }; // Valores inteiros s�o convertidos automaticamente para float
	// O acesso via P. permite acesso a uma s�rie de componentes do vetor, a saber:
	//	x, y e z (+ w se vec4)
	//  r, g e b (+ a se vec4)
	//  s, t e p (+ q se vec4)
	// O uso depende do dom�nio, se coordenadas no espa�o vetorial, cores ou texturas - definidos como a uni�o desses 
	//	componentes pela documenta��o

	std::cout << "X: " << P.x << " Y: " << P.y << " Z: " << P.z << std::endl;
	std::cout << "R: " << P.r << " G: " << P.g << " B: " << P.b << std::endl;
	std::cout << "S: " << P.s << " T: " << P.t << " P: " << P.p << std::endl;

	// Tamb�m � poss�vel acessar como um array, via �ndice:
	std::cout << "0: " << P[0] << " 1: " << P[1] << " 2: " << P[2] << std::endl;
}

// Fun��o para acessar componentes de vetores via Mix and Match
void Swizzle() {
	std::cout << std::endl;

	glm::vec3 P{ 1, 2, 3 };

	// Para o swizzle, utilizar a macro/extens�o GLM_SWIZZLE, caso contr�rio essas declara��es n�o s�o habilitadas por default
	// 	   O compilador informou que GLM_SWIZZLE fora depreciado, sendo utilizado GLM_FORCE_SWIZZLE no lugar
	// Mas normalmente o Swizzle n�o � utilizado em C++, pois onera consideravelmente a performance
	// * � bastante usado no GLSL, que roda diretamente na placa gr�fica mesmo
	glm::vec3 Q = P.xxx; // Ponto Q gerado a partir do P, sendo que suas tr�s componentes possuem o valor de P.x
	glm::vec3 R = P.xyx;
	glm::vec4 T = P.xyyy;

	std::cout << "P: " << glm::to_string(P) << std::endl;
	std::cout << "Q: " << glm::to_string(Q) << std::endl;
	std::cout << "R: " << glm::to_string(R) << std::endl;
	std::cout << "T: " << glm::to_string(T) << std::endl;
}

// Fun��o para demonstrar opera��es com vetores
void Operations() {
	std::cout << std::endl;

	glm::vec3 P0{ 10.0f, 10.0f, 0.0f };
	glm::vec3 P1{ 10.0f, 10.0f, 10.0f };

	std::cout << "P0: " << glm::to_string(P0) << std::endl;
	std::cout << "P1: " << glm::to_string(P1) << std::endl;

	glm::vec3 Resultado;

	// Soma
	Resultado = P0 + P1;
	std::cout << "Resultado soma: " << glm::to_string(Resultado) << std::endl;

	// Subtra��o
	Resultado = P0 - P1;
	std::cout << "Resultado subtra��o: " << glm::to_string(Resultado) << std::endl;

	// Escala
	Resultado = P0 * 5.0f;
	// Resultado = P0 / 5.0f
	std::cout << "Resultado escala * 5: " << glm::to_string(Resultado) << std::endl;

	// Multiplica��o
	Resultado = P0 * P1;
	std::cout << "Resultado multiplica��o: " << glm::to_string(Resultado) << std::endl;

	// Divis�o - cuidado com divis�es por zero
	Resultado = P0 / P1;
	std::cout << "Resultado divis�o: " << glm::to_string(Resultado) << std::endl;

	// Comprimento de um vetor
	float Comprimento = glm::length(P0); // sqrt( x^2 + y^2 + z^2 )
	std::cout << "Comprimento P0: " << Comprimento << std::endl;

	// N�o confundir com quantidade de componentes de um vetor - declara��es semelhantes:
	std::cout << "Qtd Componentes P0: " << P0.length() << std::endl;

	// Normalizar um vetor
	glm::vec3 Norma = glm::normalize(P0);
	std::cout << "P0 normalizado: " << glm::to_string(Norma) << std::endl;

	// Produto escalar (Dot Product)
	float DotProd = glm::dot(P0, P1);
	std::cout << "Produto escalar: " << DotProd << std::endl;

	// Produto vetorial (Cross Product)
	glm::vec3 CrossProd = glm::cross(P0, P1);
	std::cout << "Produto vetorial: " << glm::to_string(CrossProd) << std::endl;

	// Dist�ncia entre pontos
	float Distance = glm::distance(P0, P1);
	std::cout << "Dist�ncia entre pontos: " << Distance << std::endl;

	// Refra��o - um vetor, sua normaliza��o e o �ndice de refra��o
	glm::vec3 Refract = glm::refract(P0, Norma, 1.0f); 
	std::cout << "Refra��o de P0: " << glm::to_string(Refract) << std::endl;

	// Reflex�o - um vetor e sua normaliza��o
	glm::vec3 Reflect = glm::reflect(P0, Norma);
	std::cout << "Reflex�o de P0: " << glm::to_string(Reflect) << std::endl;
}

int main() {
	Constructors();
	Components();
	Swizzle();
	Operations();
	
	return 0;
}