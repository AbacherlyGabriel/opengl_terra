#include <iostream>
#include <iomanip> // Facilita impress�o de matrizes

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

// Fun��o para impress�o personalizada de matrizes
void PrintMatrix(const glm::mat4& M) {
	for (int i = 0; i < M.length(); i++) {
		for (int j = 0; j < M[i].length(); j++) {
			std::cout
				<< std::setw(10)		// Espa�amento
				<< std::setprecision(4) // Precis�o
				<< std::fixed			// Formato
				<< M[j][i] << " ";		// Conte�do da matriz (i e j invertidos para resultados de transla��es)
		}
		std::cout << std::endl;			// Quebra a linha
	}
}

// Fun��o para demonstra��o de transla��o matricial
void TranslationMatrix() {
	std::cout << std::endl;
	std::cout << "------------------" << std::endl;
	std::cout << "TRANSLATION MATRIX" << std::endl;
	std::cout << "------------------" << std::endl;
	
	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicializa��o
	glm::vec3 TranslateVec = { 10, 10, 10 };		 // Vetor de transla��o
	glm::mat4 Translation = glm::translate(Identity, TranslateVec);

	// std::cout << glm::to_string(M) << std::endl; Imprime em linha, visual menos intelig�vel
	PrintMatrix(Translation);

	glm::vec4 Position{ 10, 10, 10, 1 }; // A componente w = 1 do vetor o caracteriza como posicional
	glm::vec4 Direction{ 10, 10, 10, 0 };// A componente w = 0 do vetor o caracteriza como direcional

	std::cout << std::endl;
	Position = Translation * Position;
	Direction = Translation * Direction;

	std::cout << glm::to_string(Position) << std::endl; // Conforme o previsto
	std::cout << glm::to_string(Direction) << std::endl;// Inalterado, conceitualmente n�o faz sentido transladar uma dire��o
		// Afinal a opera��o consiste em deslocar-se no espa�o vetorial, seguindo uma dire��o. Se o vetor apenas estabelecer
		// essa dire��o, ele n�o informa exatamente a propriedade essencial para o deslocamento.
		// Isso tamb�m pode ser confirmado matematicamente realizando opera��es de multiplica��es entre matrizes
}

// Fun��o para demonstra��o de proporcionalidades aplicadas � matrizes (mudan�as escalares)
void ScaleMatrix() {
	std::cout << std::endl;
	std::cout << "------------" << std::endl;
	std::cout << "SCALE MATRIX" << std::endl;
	std::cout << "------------" << std::endl;

	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicializa��o
	glm::vec3 ScaleAmount{ 2, 2, 2 };
	glm::mat4 Scale = glm::scale(Identity, ScaleAmount);

	PrintMatrix(Scale);

	glm::vec4 Position{ 100, 100, 0, 1 }; // A componente w = 1 do vetor o caracteriza como posicional
	glm::vec4 Direction{ 100, 100, 0, 0 };// A componente w = 0 do vetor o caracteriza como direcional

	std::cout << std::endl;
	Position = Scale * Position;
	Direction = Scale * Direction;

	std::cout << glm::to_string(Position) << std::endl; // Conforme o previsto
	std::cout << glm::to_string(Direction) << std::endl;// Altera, como a escala interfere na magnitude do vetor, ainda que 
		// ele seja direcional, a intensidade ou magnitude das componentes pode escalar conforme o multiplicador
}

// Fun��o para demonstra��o de rota��es matriciais
void RotationMatrix() {
	std::cout << std::endl;
	std::cout << "---------------" << std::endl;
	std::cout << "ROTATION MATRIX" << std::endl;
	std::cout << "---------------" << std::endl;

	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicializa��o
	constexpr float Angle = glm::radians(90.0f); // Rota��o depende de um �ngulo que � recebido em radianos por padr�o - essa 
												 // fun��o est�, portanto, recebendo um valor em graus e armazenando sua 
												 // convers�o na vari�vel
	glm::vec3 Axis = { 0, 0, 1 }; // Define o eixo de rota��o x, y, z. (0,0,1 significa que a rota��o ocorrer� apenas no 
								  // eixo z)
	glm::mat4 Rotation = glm::rotate(Identity, Angle, Axis);

	PrintMatrix(Rotation);

	glm::vec4 Position{ 100, 0, 0, 1 }; // A componente w = 1 do vetor o caracteriza como posicional
	glm::vec4 Direction{ 100, 0, 0, 0 };// A componente w = 0 do vetor o caracteriza como direcional

	std::cout << std::endl;
	Position = Rotation * Position;
	Direction = Rotation * Direction;

	std::cout << glm::to_string(Position) << std::endl; // Conforme o previsto
	std::cout << glm::to_string(Direction) << std::endl;// Altera, pois altera��es rotacionais justamente mudam a dire��o de
														// um vetor
}

// Fun��o para demonstrar o processo de composi��o de matrizes
void ComposedMatrix() {
	std::cout << std::endl;
	std::cout << "---------------" << std::endl;
	std::cout << "COMPOSED MATRIX" << std::endl;
	std::cout << "---------------" << std::endl;

	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicializa��o

	glm::vec3 T{ 0, 10, 0 }; // Transla��o
	glm::mat4 Translation = glm::translate(Identity, T);

	constexpr float Angle = glm::radians(45.0f); // Rota��o
	glm::vec3 Axis = { 0, 0, 1 }; 
	glm::mat4 Rotation = glm::rotate(Identity, Angle, Axis);

	glm::vec3 ScaleAmount{ 2, 2, 0 }; // Escala
	glm::mat4 Scale = glm::scale(Identity, ScaleAmount);

	std::cout << "Translation: " << std::endl; // Impress�es individuais
	PrintMatrix(Translation);
	std::cout << "Rotation: " << std::endl;
	PrintMatrix(Rotation);
	std::cout << "Scale: " << std::endl;
	PrintMatrix(Scale);

	glm::vec4 Position{ 1, 1, 0, 1 };
	glm::vec4 Direction{ 1, 1, 0, 0 };

	// Composi��o de matrizes permite que os c�lculos sejam realizados em uma �nica matriz - produzimos assim uma Model Matrix
	// Lembrar que multiplica��o de matrizes N�O � COMUTATIVA (a ordem importa)
	// Neste caso, a ordem das opera��es �, na pr�tica, realizada da direita para a esquerda:
	//	Primeiro � realizada a escala, depois a rota��o e por fim a matriz � translada.
	glm::mat4 ModelMatrix = Translation * Rotation * Scale;

	std::cout << "Model Matrix: " << std::endl;
	PrintMatrix(ModelMatrix);

	Position = ModelMatrix * Position;
	Direction = ModelMatrix * Direction;

	std::cout << glm::to_string(Position) << std::endl;
	std::cout << glm::to_string(Direction) << std::endl;
}

// Fun��o para demonstrar o uso das fun��es lookAt e perspective do GLM - constru��o das matrizes VIEW e PROJECTION 
// (perspective)
// *** Model View Project � tamb�m popularmente conhecido como MVP dentro da literatura da Computa��o Gr�fica
void ModelViewProjection() {
	std::cout << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << "MODEL VIEW PROJECTION" << std::endl;
	std::cout << "---------------------" << std::endl;

	// Model ser� uma matriz formada pelas transforma��es de transala��o, rota��o e escala. Portanto, � uma matriz composta
	glm::mat4 ModelMatrix = glm::identity<glm::mat4>(); // Inicializa��o como identidade

	// View (defini��es da c�mera)
	glm::vec3 Eye{ 0, 0, 10 };   // Origem da c�mera - imaginando a tela do computador sendo x, y; o valor 10 no eixo z
							     // retornaria a vis�o do usu�rio do computador
	glm::vec3 Center{ 0, 0, 0 }; // Dire��o do olhar - a c�mera olha para o centro do monitor (0,0,0)
	glm::vec3 Up{ 0, 1, 0 };     // Orienta��o do olhar - direcionado levemente para cima (valores negativos retornariam o
							     // conte�do de cabe�a para baixo)
	glm::mat4 ViewMatrix = glm::lookAt(Eye, Center, Up);

	std::cout << "View Matrix: " << std::endl;
	PrintMatrix(ViewMatrix);

	// Projection - transforma��o projetiva (0 no elemento inferior direito da matriz composta) - se fosse 1 seria homog�nea
	constexpr float FoV = glm::radians(45.0f);
	const float AspectRatio = 800.0f / 600.0f; // Largura da tela / Altura da tela
	const float Near = 1.0f;
	const float Far = 100000.0f;
	glm::mat4 ProjectionMatrix = glm::perspective(FoV, AspectRatio, Near, Far);

	std::cout << "Projection Matrix: " << std::endl;
	PrintMatrix(ProjectionMatrix);

	glm::mat4 ModelViewProjection = ProjectionMatrix * ViewMatrix * ModelMatrix;

	std::cout << "Model View Projection: " << std::endl;
	PrintMatrix(ModelViewProjection);

	glm::vec4 Position{ 0, 0, 0, 1 };
	
	Position = ModelViewProjection * Position;

	std::cout << glm::to_string(Position) << std::endl;

	// O que o OpenGL far� para n�s automaticamente via shaders, dentro da placa gr�fica, � dividir todos os componentes 
	// pela coordenada homog�nea (w) para levar o resultado da proje��o para dentro daquele cubo de tamanho |1|.
	// Essa opera��o pode ser ilustrada como:
	Position = Position / Position.w;

	std::cout << glm::to_string(Position) << std::endl;
}

int main() {

	TranslationMatrix();
	ScaleMatrix();
	RotationMatrix();
	ComposedMatrix();
	ModelViewProjection();

	return 0;
}