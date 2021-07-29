#include <iostream>
#include <iomanip> // Facilita impressão de matrizes

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

// Função para impressão personalizada de matrizes
void PrintMatrix(const glm::mat4& M) {
	for (int i = 0; i < M.length(); i++) {
		for (int j = 0; j < M[i].length(); j++) {
			std::cout
				<< std::setw(10)		// Espaçamento
				<< std::setprecision(4) // Precisão
				<< std::fixed			// Formato
				<< M[j][i] << " ";		// Conteúdo da matriz (i e j invertidos para resultados de translações)
		}
		std::cout << std::endl;			// Quebra a linha
	}
}

// Função para demonstração de translação matricial
void TranslationMatrix() {
	std::cout << std::endl;
	std::cout << "------------------" << std::endl;
	std::cout << "TRANSLATION MATRIX" << std::endl;
	std::cout << "------------------" << std::endl;
	
	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicialização
	glm::vec3 TranslateVec = { 10, 10, 10 };		 // Vetor de translação
	glm::mat4 Translation = glm::translate(Identity, TranslateVec);

	// std::cout << glm::to_string(M) << std::endl; Imprime em linha, visual menos inteligível
	PrintMatrix(Translation);

	glm::vec4 Position{ 10, 10, 10, 1 }; // A componente w = 1 do vetor o caracteriza como posicional
	glm::vec4 Direction{ 10, 10, 10, 0 };// A componente w = 0 do vetor o caracteriza como direcional

	std::cout << std::endl;
	Position = Translation * Position;
	Direction = Translation * Direction;

	std::cout << glm::to_string(Position) << std::endl; // Conforme o previsto
	std::cout << glm::to_string(Direction) << std::endl;// Inalterado, conceitualmente não faz sentido transladar uma direção
		// Afinal a operação consiste em deslocar-se no espaço vetorial, seguindo uma direção. Se o vetor apenas estabelecer
		// essa direção, ele não informa exatamente a propriedade essencial para o deslocamento.
		// Isso também pode ser confirmado matematicamente realizando operações de multiplicações entre matrizes
}

// Função para demonstração de proporcionalidades aplicadas à matrizes (mudanças escalares)
void ScaleMatrix() {
	std::cout << std::endl;
	std::cout << "------------" << std::endl;
	std::cout << "SCALE MATRIX" << std::endl;
	std::cout << "------------" << std::endl;

	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicialização
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

// Função para demonstração de rotações matriciais
void RotationMatrix() {
	std::cout << std::endl;
	std::cout << "---------------" << std::endl;
	std::cout << "ROTATION MATRIX" << std::endl;
	std::cout << "---------------" << std::endl;

	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicialização
	constexpr float Angle = glm::radians(90.0f); // Rotação depende de um ângulo que é recebido em radianos por padrão - essa 
												 // função está, portanto, recebendo um valor em graus e armazenando sua 
												 // conversão na variável
	glm::vec3 Axis = { 0, 0, 1 }; // Define o eixo de rotação x, y, z. (0,0,1 significa que a rotação ocorrerá apenas no 
								  // eixo z)
	glm::mat4 Rotation = glm::rotate(Identity, Angle, Axis);

	PrintMatrix(Rotation);

	glm::vec4 Position{ 100, 0, 0, 1 }; // A componente w = 1 do vetor o caracteriza como posicional
	glm::vec4 Direction{ 100, 0, 0, 0 };// A componente w = 0 do vetor o caracteriza como direcional

	std::cout << std::endl;
	Position = Rotation * Position;
	Direction = Rotation * Direction;

	std::cout << glm::to_string(Position) << std::endl; // Conforme o previsto
	std::cout << glm::to_string(Direction) << std::endl;// Altera, pois alterações rotacionais justamente mudam a direção de
														// um vetor
}

// Função para demonstrar o processo de composição de matrizes
void ComposedMatrix() {
	std::cout << std::endl;
	std::cout << "---------------" << std::endl;
	std::cout << "COMPOSED MATRIX" << std::endl;
	std::cout << "---------------" << std::endl;

	glm::mat4 Identity = glm::identity<glm::mat4>(); // Matriz identidade para inicialização

	glm::vec3 T{ 0, 10, 0 }; // Translação
	glm::mat4 Translation = glm::translate(Identity, T);

	constexpr float Angle = glm::radians(45.0f); // Rotação
	glm::vec3 Axis = { 0, 0, 1 }; 
	glm::mat4 Rotation = glm::rotate(Identity, Angle, Axis);

	glm::vec3 ScaleAmount{ 2, 2, 0 }; // Escala
	glm::mat4 Scale = glm::scale(Identity, ScaleAmount);

	std::cout << "Translation: " << std::endl; // Impressões individuais
	PrintMatrix(Translation);
	std::cout << "Rotation: " << std::endl;
	PrintMatrix(Rotation);
	std::cout << "Scale: " << std::endl;
	PrintMatrix(Scale);

	glm::vec4 Position{ 1, 1, 0, 1 };
	glm::vec4 Direction{ 1, 1, 0, 0 };

	// Composição de matrizes permite que os cálculos sejam realizados em uma única matriz - produzimos assim uma Model Matrix
	// Lembrar que multiplicação de matrizes NÃO É COMUTATIVA (a ordem importa)
	// Neste caso, a ordem das operações é, na prática, realizada da direita para a esquerda:
	//	Primeiro é realizada a escala, depois a rotação e por fim a matriz é translada.
	glm::mat4 ModelMatrix = Translation * Rotation * Scale;

	std::cout << "Model Matrix: " << std::endl;
	PrintMatrix(ModelMatrix);

	Position = ModelMatrix * Position;
	Direction = ModelMatrix * Direction;

	std::cout << glm::to_string(Position) << std::endl;
	std::cout << glm::to_string(Direction) << std::endl;
}

// Função para demonstrar o uso das funções lookAt e perspective do GLM - construção das matrizes VIEW e PROJECTION 
// (perspective)
// *** Model View Project é também popularmente conhecido como MVP dentro da literatura da Computação Gráfica
void ModelViewProjection() {
	std::cout << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << "MODEL VIEW PROJECTION" << std::endl;
	std::cout << "---------------------" << std::endl;

	// Model será uma matriz formada pelas transformações de transalação, rotação e escala. Portanto, é uma matriz composta
	glm::mat4 ModelMatrix = glm::identity<glm::mat4>(); // Inicialização como identidade

	// View (definições da câmera)
	glm::vec3 Eye{ 0, 0, 10 };   // Origem da câmera - imaginando a tela do computador sendo x, y; o valor 10 no eixo z
							     // retornaria a visão do usuário do computador
	glm::vec3 Center{ 0, 0, 0 }; // Direção do olhar - a câmera olha para o centro do monitor (0,0,0)
	glm::vec3 Up{ 0, 1, 0 };     // Orientação do olhar - direcionado levemente para cima (valores negativos retornariam o
							     // conteúdo de cabeça para baixo)
	glm::mat4 ViewMatrix = glm::lookAt(Eye, Center, Up);

	std::cout << "View Matrix: " << std::endl;
	PrintMatrix(ViewMatrix);

	// Projection - transformação projetiva (0 no elemento inferior direito da matriz composta) - se fosse 1 seria homogênea
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

	// O que o OpenGL fará para nós automaticamente via shaders, dentro da placa gráfica, é dividir todos os componentes 
	// pela coordenada homogênea (w) para levar o resultado da projeção para dentro daquele cubo de tamanho |1|.
	// Essa operação pode ser ilustrada como:
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