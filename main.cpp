
#include <array>
#include <iostream>
#include <fstream>
#include <vector>

// Não incluímos o GL.h pois nele constam apenas as funções do OpenGL 1.0 ou 1.1
// Por isso utilizamos o GLEW (Extension Wrangler) - funções mais novas já incluindo as legadas automaticamente
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION // Macro necessária para ativar o header STB
#include <stb_image.h>

#include "Camera.h"

const int Width = 800; // Constantes que determinam o tamanho da janela de contexto do GLFW
const int Height = 600;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Color;
	glm::vec2 UV;
};

struct Triangle
{
	GLuint V0;
	GLuint V1;
	GLuint V2;
};

struct DirectionalLight
{
	glm::vec3 Direction;
	GLfloat Intensity;
};

SimpleCamera Camera;

// Função para gerar vértices e a malha triangular da geometria da esfera
// A equação para cálculo dos vértices é expressa por:
// x = x_0 + r sinPhi cosTheta
// y = y_0 + r sinPhi sinTheta
// z = z_0 + r cosPhi
// * Como podemos utilizar a MVP para transladar, rotacionar ou escalar nossa geometria, podemos simplificar a equação
//  gerando esfera com origem em (0,0,0) e utilizando raio = 1
void GenerateSphere(GLuint Resolution, std::vector<Vertex>& Vertices, std::vector<Triangle>& Indices)
{
	Vertices.clear(); // Apenas garantindo a inicialização correta
	Indices.clear();

	constexpr float Pi = glm::pi<float>();
	constexpr float TwoPi = glm::two_pi<float>();
	float InvResolution = 1.0f / static_cast<float>(Resolution - 1); // Para não cair fora do array de resolução

	for (GLuint UIndex = 0; UIndex < Resolution; ++UIndex)
	{
		const float U = UIndex * InvResolution; 
		const float Theta = glm::mix(0.0f, TwoPi, static_cast<float>(U)); // Interpolação linear para obter um Theta entre 0 e 2PI

		for (GLuint VIndex = 0; VIndex < Resolution; ++VIndex)
		{
			const float V = VIndex * InvResolution;
			const float Phi = glm::mix(0.0f, Pi, static_cast<float>(V)); // Interpolação linear para obter um Phi entre 0 e PI

			// Definição da posição dos vértices do triângulo
			glm::vec3 VertexPosition =
			{
				glm::cos(Theta) * glm::sin(Phi),
				glm::sin(Theta) * glm::sin(Phi),
				glm::cos(Phi)
			};

			glm::vec3 VertexNormal = glm::normalize(VertexPosition);

			// Carrega no array enviado pelo parâmetro:
			//  posições do vértice, sua normal, um vetor de cor (inutilizado) e as coordenadas UV ajustadas para orientar
			//  as texturas para cima
			Vertices.push_back(Vertex{
				VertexPosition,
				VertexNormal,
				glm::vec3{ 1.0f, 1.0f, 1.0f },
				glm::vec2{ 1.0f - U, 1.0f - V }
			});
		}
	}

	// Indexando os pontos que formam os quads(e triângulos) da malha que irão compor a esfera
	for (GLuint U = 0; U < Resolution - 1; ++U)
	{
		for (GLuint V = 0; V < Resolution - 1; ++V)
		{
			GLuint P0 = U + V * Resolution;
			GLuint P1 = U + 1 + V * Resolution;
			GLuint P2 = U + (V + 1) * Resolution;
			GLuint P3 = U + 1 + (V + 1) * Resolution;

			// O quad será formado por dois triângulos cortando a sua diagonal. Tendo (0,0) como origem, os pontos ficariam:
			// Primeiro triângulo: (0,0), (1,0) e (0,1)
			// Segundo triângulo: (0,1), (1,0) e (1,1)
			// Observar que assim é possível reaproveitar vértices de um quad para outro, otimizando o modelo
			Indices.push_back(Triangle{ P3, P2, P0 });
			Indices.push_back(Triangle{ P1, P3, P0 });
		}
	}
}

// Função para leitura de arquivos
std::string ReadFile(const char* FilePath)
{
	std::string FileContents;
	if (std::ifstream FileStream{ FilePath, std::ios::in }) // Se entrar no if, foi possível criar a stream de leitura
	{
		FileContents.assign((std::istreambuf_iterator<char>(FileStream)), std::istreambuf_iterator<char>());
	}
	return FileContents;
}

// Função para verificação do log de compilação do shader (recebe o identificador de um shader compilado como parâmetro)
void CheckShader(GLuint ShaderId)
{
	// Verificar se o shader foi compilado
	GLint Result = GL_TRUE;
	glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &Result);

	if (Result == GL_FALSE)
	{
		// Erro ao compilar o shader, imprimir o log para saber o que está errado
		GLint InfoLogLength = 0; // Obter tamanho do log
		glGetShaderiv(ShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength); // Quantidade em bytes a ser alocado na string

		std::string ShaderInfoLog(InfoLogLength, '\0'); // Inicializar string de tamanho InfoLogLength com todos 
													    // os char = '0'
		glGetShaderInfoLog(ShaderId, InfoLogLength, nullptr, &ShaderInfoLog[0]); // Recupera o log armazenando em 
																				 // ShaderInfoLog
		if (InfoLogLength > 0)
		{
			std::cout << "Erro no Vertex Shader: " << std::endl;
			std::cout << ShaderInfoLog << std::endl;

			assert(false); // Erro no shader, interrompe o programa
		}
	}
}

// Função para carregar os programas de shaders
GLuint LoadShaders(const char* VertexShaderFile, const char* FragmentShaderFile)
{
	// Criar os identificadores do Vertex e do Fragment Shaders
	GLuint VertShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	std::string VertexShaderSource = ReadFile(VertexShaderFile);
	std::string FragmentShaderSource = ReadFile(FragmentShaderFile);

	assert(!VertexShaderSource.empty());
	assert(!FragmentShaderSource.empty());

	// Utilizar o OpenGL para compilar os shaders
	std::cout << "Compilando " << VertexShaderFile << std::endl;
	const char* VertexShaderSourcePtr = VertexShaderSource.c_str(); // Ponteiro para o fonte do Vertex Shader
	glShaderSource(VertShaderId, 1, &VertexShaderSourcePtr, nullptr); // Chamada a função que determina os parâmetros
		// dos fontes que serão compilados, recebendo o Id, a quantidade de fontes a serem compilados (neste exemplo apenas 1),
		// os endereços dos ponteiros e o comprimento da leitura (como utilizamos a funções c_str() será uma string com 
		// ponteiro nulo de terminação)
	glCompileShader(VertShaderId); // Compila todos os Vertex Shaders parametrizados acima
	CheckShader(VertShaderId);

	std::cout << "Compilando " << FragmentShaderFile << std::endl;
	const char* FragmentShaderSourcePtr = FragmentShaderSource.c_str();
	glShaderSource(FragShaderId, 1, &FragmentShaderSourcePtr, nullptr);
	glCompileShader(FragShaderId);
	CheckShader(FragShaderId);

	// Feita a compilação dos shaders, é necessário confeccionar o programa a ser carregado na pipeline.
	std::cout << "Linkando Programa" << std::endl;
	GLuint ProgramId = glCreateProgram(); // Elencar abaixo todos os shaders que fazem parte desse programa
	glAttachShader(ProgramId, VertShaderId);
	glAttachShader(ProgramId, FragShaderId);
	glLinkProgram(ProgramId); // Conclui o link entre os shaders compilados acima relacionados

	// Verificar resultado do link
	GLint Result = GL_TRUE;
	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Result); // Armazena o status de inicialização em Result
	
	if (Result == GL_FALSE) // Obter o log para compreensão do problema
	{
		GLint InfoLogLength = 0;
		glGetProgramiv(ProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength); // Quantidade em bytes a ser alocado na string

		if (InfoLogLength > 0) // Se gerou log de erro, o recupera e exibe em tela
		{
			std::string ProgramInfoLog(InfoLogLength, '\0');
			glGetProgramInfoLog(ProgramId, InfoLogLength, nullptr, &ProgramInfoLog[0]);

			std::cout << "Erro ao linkar programa" << std::endl;
			std::cout << ProgramInfoLog << std::endl;

			assert(false);
		}
	}

	// Como boa prática, uma vez que utilizamos recursos é bom ilberá-los na sequência
	// (isso não desfaz o link com o programa após a compilação, apenas libera o uso dos Ids e pilhas de memória para 
	//  evitar comportamentos indesejados e sujeiras de memória)
	glDetachShader(ProgramId, VertShaderId);
	glDetachShader(ProgramId, FragShaderId);

	glDeleteShader(VertShaderId);
	glDeleteShader(FragShaderId);

	std::cout << "Programa associado, shaders carregados com sucesso" << std::endl;

	return ProgramId;
}

// Função para carregar texturas a partir de arquivos com imagens
GLuint LoadTexture(const char* TextureFile)
{
	std::cout << "Carregando Textura " << TextureFile << std::endl;

	int TextureWidth = 0;
	int TextureHeight = 0;
	int NumberOfComponents = 0;

	// Recebe por parâmetro um ponteiro para um arquivo, endereços de três variáveis para armazenar os tamanhos da largura
	//	e da altura da imagem carregada, o número de componentes disponível e por fim é necessário especificar a quantidade
	//	de componentes que desejamos retornar (3 = RGB)
	// Textura em RAM
	unsigned char* TextureData = stbi_load(TextureFile, &TextureWidth, &TextureHeight, &NumberOfComponents, 3);
	
	assert(TextureData); // Caso algo dê errado durante o carregamento da textura, interrompe o processamento

	std::cout << "Textura carregada com sucesso" << std::endl;

	// Gerar o Identifador da Textura + procedimento para levá-la para a memória de vídeo
	GLuint TextureId;
	glGenTextures(1, &TextureId);

	// Habilita a textura para ser modificada (bind)
	glBindTexture(GL_TEXTURE_2D, TextureId); // 2D por ser uma imagem

	// Copiar a textura para a memória de vídeo (GPU)
	// 	   Recebe por parâmetro um alvo ou tipo de textura;
	//	   Um level de texturização;
	// 	   Um formato interno de armazenamento da estrutura de dados;
	// 	   Largura;
	// 	   Altura;
	// 	   Uso de bordas;
	// 	   Formato novamente - para encapsulamento;
	// 	   Tipo de referência para os dados carregados - tipo de TextureData (char* = bytes)
	//	   Ponteiro para os dados (pixels) a serem transferidos para a GPU
	GLint Level = 0;
	GLint Border = 0;
	glTexImage2D(GL_TEXTURE_2D, Level, GL_RGB, TextureWidth, TextureHeight, Border, GL_RGB, GL_UNSIGNED_BYTE, TextureData);

	// Aplicação de filtro de magnificação e minificação
	// Parametrização linear para suavizar granulação com aumento de zoom
	// Mipmap para contornar aliasing da distância
	// Configurar o Texture Wrapping - extrapolações das coordenadas normalizadas da img texturizada nas coordenadas S e T
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0); // Desligar a textura pois já foi copiada para a GPU

	stbi_image_free(TextureData); // Pode liberar a RAM utilizada
	return TextureId;
}

// Função callback para tratamento de eventos com clique do mouse
void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Modifiers)
{
	// std::cout << "Button: " << Button << " Action: " << Action << " Modifiers: " << Modifiers << std::endl;

	if (Button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (Action == GLFW_PRESS) // Ativa a ação do cursor do mouse com clique do botão esquerdo do mouse
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Cursor desaparece durante o clique

			double X, Y;
			glfwGetCursorPos(Window, &X, &Y);
			
			Camera.PreviousCursor = glm::vec2{ X, Y };
			Camera.bEnableMouseMovement = true;
		}
		else if (Action == GLFW_RELEASE) // Desativa a ação do cursor do mouse ao soltar o botão esquerdo do mouse
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Cursor retorna após soltar o botão

			Camera.bEnableMouseMovement = false;
		}
	}
}

// Função callback para tratamento de eventos com o movimento do cursor do mouse
// Essencialmente transmite à câmera as coordenadas X, Y do monitor no ponto em que o cursor se encontra
void MouseMotionCallback(GLFWwindow* Window, double X, double Y)
{
	// std::cout << "X: " << X << " Y: " << Y << std::endl;
	Camera.MouseMove(X, Y);
}

// Função callback para tratamento do movimento da câmera utilizando teclado
// Escape para fechar a janela
// W,A,S,D para movimentar a câmera para frente, esquerda, trás e direita, respectivamente 
void KeyCallback(GLFWwindow* Window, int Key, int ScanCode, int Action, int Modifers)
{
	// std::cout << "Key: " << Key << " ScanCode: " << ScanCode << " Action: " << Action << " Modifiers: " << Modifers << std::endl;	

	if (Action == GLFW_PRESS)
	{
		switch (Key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(Window, true);
				break;

			case GLFW_KEY_W:
				Camera.MoveForward(1.0f);
				break;

			case GLFW_KEY_S:
				Camera.MoveForward(-1.0f);
				break;

			case GLFW_KEY_A:
				Camera.MoveRight(-1.0f);
				break;

			case GLFW_KEY_D:
				Camera.MoveRight(1.0f);
				break;

			default:
				break;
		}
	}
	else if (Action == GLFW_RELEASE)
	{
		switch (Key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(Window, true);
				break;

			case GLFW_KEY_W:
				Camera.MoveForward(0.0f);
				break;

			case GLFW_KEY_S:
				Camera.MoveForward(0.0f);
				break;

			case GLFW_KEY_A:
				Camera.MoveRight(0.0f);
				break;

			case GLFW_KEY_D:
				Camera.MoveRight(0.0f);
				break;

			default:
				break;
		}
	}
}

int main()
{	
	if (!glfwInit())
	{
		std::cout << "Erro ao inicializar o GLFW" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	// Criar uma janela:
	// Recebe como parâmetros a largura e altura da janela, um título, um monitor (caso haja mais de um) e um 
	//	contexto, caso se deseje compartilhar a exibição
	GLFWwindow* Window = glfwCreateWindow(Width, Height, "Blue Marble", nullptr, nullptr);	

	if (!Window)
	{
		std::cout << "Erro ao criar janela" << std::endl;
		glfwTerminate();
		return 1;
	}	
	
	// Cadastrar as callbacks no GLFW
	glfwSetMouseButtonCallback(Window, MouseButtonCallback);
	glfwSetCursorPosCallback(Window, MouseMotionCallback);
	glfwSetKeyCallback(Window, KeyCallback);

	// Ativa o contexto criado na janela Window:
	//	Associa um objeto GLFW a um contexto, para que o GLEW possa inicializar com referência para esse contexto
	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1); // Habilita ou desabilita o V-Sync

	// Inicializar a biblioteca GLEW (deve ser criada após a janela, pois é necessário um contexto de 
	//	OpenGL para que a API do OpenGL possa apontar)
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Erro ao inicializar o GLEW" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Verificar a versão do OpenGL
	GLint GLMajorVersion = 0;
	GLint GLMinorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &GLMinorVersion);	
	std::cout << "OpenGL Version  : " << GLMajorVersion << "." << GLMinorVersion << std::endl;
	std::cout << "OpenGL Vendor   : " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "OpenGL Renderer : " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL Version  : " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Habilita o Buffer de Profundidade (Z-buffer)
	glEnable(GL_DEPTH_TEST);

	// Escolhe a função de teste de profundidade.
	glDepthFunc(GL_ALWAYS);

	// Otimização de processamento: habilitar o Back Face Culling - não renderiza a face traseira dos objetos
	glDisable(GL_CULL_FACE);
	glEnable(GL_CULL_FACE);

	// Compilar o vertex e o fragment shader
	GLuint ProgramId = LoadShaders("shaders/triangle_vert.glsl", "shaders/triangle_frag.glsl");

	// Gera a Geometria da esfera e copia os dados para a GPU (memória da placa de vídeo)
	std::vector<Vertex> SphereVertices;
	std::vector<Triangle> SphereIndices;
	GenerateSphere(100, SphereVertices, SphereIndices);
	GLuint SphereVertexBuffer, SphereElementBuffer; // VBO e EBO (Vertex e Element Buffer Objects)
	glGenBuffers(1, &SphereVertexBuffer); // Pedir para o OpenGL gerar o identificador do VBO e do EBO
	glGenBuffers(1, &SphereElementBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, SphereVertexBuffer); // Linkar/ativar o buffer ao seu tipo para o OpenGL
	// Copia efetivamente do buffer (memória RAM) para a GPU (memória de vídeo)
	glBufferData(GL_ARRAY_BUFFER, SphereVertices.size() * sizeof(Vertex), SphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereElementBuffer);	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, SphereIndices.size() * sizeof(Triangle), SphereIndices.data(), GL_STATIC_DRAW);

	// Criar uma fonte de luz direcional
	DirectionalLight Light;
	Light.Direction = glm::vec3(0.0f, 0.0f, -1.0f); // Z negativo, apontando para dentro da tela
	Light.Intensity = 1.0f;

	// Model Matrix - identidade rotacionada para viabilizar cálculos com a Model View Projection - MVP
	glm::mat4 ModelMatrix = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });

	// Carregar as texturas para a memória de vídeo
	//GLuint EarthTextureId = LoadTexture("textures/earth_2k.jpg");
	GLuint EarthTextureId = LoadTexture("textures/earth5400x2700.jpg");
	GLuint CloudsTextureId = LoadTexture("textures/earth_clouds_2k.jpg");

	// Configura a cor de fundo
	// **Ter em mente que o OpenGL é uma máquina de estados (quando ativarmos algo, essa coisa permanecerá ativa por padrão)
	//  Definir a cor do fundo (isso é um estado, o driver armazenará essa informação:
	//	Sempre que precisarmos retomar essa informação, limparmos o framebuffer, etc, esse estado será devidamente restaurado)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0); // RGBA

	// Identificador do Vertex Array Object (VAO)
	GLuint SphereVAO;

	// Gera o identificador do VAO
	glGenVertexArrays(1, &SphereVAO);

	// Habilita o VAO
	glBindVertexArray(SphereVAO);

	// Ativa o atributo de vértice para o array. O parâmetro representa o índice (location) do layout do shader ativo
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Ativa os buffers de vértice e de elemento para serem utilizados no contexto OpenGL 
	glBindBuffer(GL_ARRAY_BUFFER, SphereVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereElementBuffer);

	// Informa ao OpenGL onde os vértices se encontram dentro do VertexBuffer. 
	//  [0;3] são os índices habilitados, coincidindo com os especificados em glEnableVertexAttribArray() / location nos shaders
	//	[2;3] são as dimensões (qtd) de vértices das estruturas de dados utilizadas (vec2 e vec3)
	//	GL_FLOAT é o tipo primitivo
	//  GL_FALSE / GL_TRUE para informar se os atributos estão normalizados ou não 
	//	stride - tamanho do Vertex (struct) que definimos
	//	offset - para position é nulo, para color e os demais é calculado. O cast é necessário para compatibilizar o retorno
	//		     do método offsetof com o parâmetro recebido pela função glVertexAttribPointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));	

	// Disabilitar o VAO
	glBindVertexArray(0);

	double PreviousTime = glfwGetTime(); // Tempo do frame anterior

	// Entra no loop de eventos da aplicação tendo a janela fechada como condição de parada
	while (!glfwWindowShouldClose(Window))
	{	
		// Calcula o Delta Time para suavização da movimentação da câmera
		double CurrentTime = glfwGetTime();
		double DeltaTime = CurrentTime - PreviousTime;
		if (DeltaTime > 0.0)
		{
			Camera.Update(static_cast<float>(DeltaTime));
			PreviousTime = CurrentTime;
		}		

		// Ativa o bit do buffer que realiza a limpeza dos buffers de cor e de profundidade
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		glUseProgram(ProgramId); // Ativa o programa de shaders
		
		// Cálculos matriciais para determinação da Matriz Normal (utilizada para a iluminação) e para a Model View Projection
		// MVP (utilizada para transladar, rotacionar e escalar os objetos no espaço euclidiano)
		glm::mat4 ViewMatrix = Camera.GetView();
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrix));
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat4 ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrix;

		// Abaixo recuperação dos uniformes calculados pelos shaders
		GLint TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		GLint NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		GLint ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelViewMatrix));

		GLint ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		GLint LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");
		glUniform1f(LightIntensityLoc, Light.Intensity);

		// Determinação 
		glm::vec4 LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		GLint LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		// Ativação e endereçamento da textura para os shaders
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, EarthTextureId);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, CloudsTextureId);

		GLint TextureSamplerLoc = glGetUniformLocation(ProgramId, "EarthTexture");
		glUniform1i(TextureSamplerLoc, 0);

		GLint CloudsTextureSamplerLoc = glGetUniformLocation(ProgramId, "CloudsTexture");
		glUniform1i(CloudsTextureSamplerLoc, 1);

		// Para testes de geometrias:
		//glPointSize(3.0f);
		//glLineWidth(3.0f);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glDrawArrays(GL_POINTS, 0, SphereNumVertices);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		// Utiliza o EBO para desenhar na tela de acordo com os índices
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		// Processa todos os eventos da fila de eventos do GLFW podem ser estímulos do teclado, mouse, gamepad, etc
		glfwPollEvents();

		// Envia o conteúdo do framebuffer da janela para ser desenhado na tela
		// A memória alocada para aplicação é trocada (swap) para a memória de vídeo que se encarregará pela
		//	renderização em tela dos pixels da janela
		//	Vale mencionar, portanto, que o tamanho da janela que estipulamos (valor das variáveis Width e Height)
		//	influencia na quantidade de memória RAM e de vídeo que nossa aplicação utilizará
		glfwSwapBuffers(Window);		
	}

	// Boa prática em OpenGL: como ele se comporta como uma máquina de estados, após habilitar o buffer, 
	//	definir um contexto e desenhar coisas em tela, reverter o que foi criado para que as próximas construções
	//	em tela sejam organizadas, novos binds rastreáveis e, em suma, o comportamento sistêmico seja controlado e 
	//	previsível. 
	glDeleteBuffers(1, &SphereElementBuffer);
	glDeleteBuffers(1, &SphereVertexBuffer);
	glDeleteVertexArrays(1, &SphereVAO);
	glDeleteProgram(ProgramId);
	glDeleteTextures(1, &EarthTextureId);

	glfwDestroyWindow(Window);
	glfwTerminate();

	return 0;
}
