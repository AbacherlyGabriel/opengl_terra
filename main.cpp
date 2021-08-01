
#include <array>
#include <iostream>
#include <fstream>
#include <vector>

// N�o inclu�mos o GL.h pois nele constam apenas as fun��es do OpenGL 1.0 ou 1.1
// Por isso utilizamos o GLEW (Extension Wrangler) - fun��es mais novas j� incluindo as legadas automaticamente
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION // Macro necess�ria para ativar o header STB
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

// Fun��o para gerar v�rtices e a malha triangular da geometria da esfera
// A equa��o para c�lculo dos v�rtices � expressa por:
// x = x_0 + r sinPhi cosTheta
// y = y_0 + r sinPhi sinTheta
// z = z_0 + r cosPhi
// * Como podemos utilizar a MVP para transladar, rotacionar ou escalar nossa geometria, podemos simplificar a equa��o
//  gerando esfera com origem em (0,0,0) e utilizando raio = 1
void GenerateSphere(GLuint Resolution, std::vector<Vertex>& Vertices, std::vector<Triangle>& Indices)
{
	Vertices.clear(); // Apenas garantindo a inicializa��o correta
	Indices.clear();

	constexpr float Pi = glm::pi<float>();
	constexpr float TwoPi = glm::two_pi<float>();
	float InvResolution = 1.0f / static_cast<float>(Resolution - 1); // Para n�o cair fora do array de resolu��o

	for (GLuint UIndex = 0; UIndex < Resolution; ++UIndex)
	{
		const float U = UIndex * InvResolution; 
		const float Theta = glm::mix(0.0f, TwoPi, static_cast<float>(U)); // Interpola��o linear para obter um Theta entre 0 e 2PI

		for (GLuint VIndex = 0; VIndex < Resolution; ++VIndex)
		{
			const float V = VIndex * InvResolution;
			const float Phi = glm::mix(0.0f, Pi, static_cast<float>(V)); // Interpola��o linear para obter um Phi entre 0 e PI

			// Defini��o da posi��o dos v�rtices do tri�ngulo
			glm::vec3 VertexPosition =
			{
				glm::cos(Theta) * glm::sin(Phi),
				glm::sin(Theta) * glm::sin(Phi),
				glm::cos(Phi)
			};

			glm::vec3 VertexNormal = glm::normalize(VertexPosition);

			// Carrega no array enviado pelo par�metro:
			//  posi��es do v�rtice, sua normal, um vetor de cor (inutilizado) e as coordenadas UV ajustadas para orientar
			//  as texturas para cima
			Vertices.push_back(Vertex{
				VertexPosition,
				VertexNormal,
				glm::vec3{ 1.0f, 1.0f, 1.0f },
				glm::vec2{ 1.0f - U, 1.0f - V }
			});
		}
	}

	// Indexando os pontos que formam os quads(e tri�ngulos) da malha que ir�o compor a esfera
	for (GLuint U = 0; U < Resolution - 1; ++U)
	{
		for (GLuint V = 0; V < Resolution - 1; ++V)
		{
			GLuint P0 = U + V * Resolution;
			GLuint P1 = U + 1 + V * Resolution;
			GLuint P2 = U + (V + 1) * Resolution;
			GLuint P3 = U + 1 + (V + 1) * Resolution;

			// O quad ser� formado por dois tri�ngulos cortando a sua diagonal. Tendo (0,0) como origem, os pontos ficariam:
			// Primeiro tri�ngulo: (0,0), (1,0) e (0,1)
			// Segundo tri�ngulo: (0,1), (1,0) e (1,1)
			// Observar que assim � poss�vel reaproveitar v�rtices de um quad para outro, otimizando o modelo
			Indices.push_back(Triangle{ P3, P2, P0 });
			Indices.push_back(Triangle{ P1, P3, P0 });
		}
	}
}

// Fun��o para leitura de arquivos
std::string ReadFile(const char* FilePath)
{
	std::string FileContents;
	if (std::ifstream FileStream{ FilePath, std::ios::in }) // Se entrar no if, foi poss�vel criar a stream de leitura
	{
		FileContents.assign((std::istreambuf_iterator<char>(FileStream)), std::istreambuf_iterator<char>());
	}
	return FileContents;
}

// Fun��o para verifica��o do log de compila��o do shader (recebe o identificador de um shader compilado como par�metro)
void CheckShader(GLuint ShaderId)
{
	// Verificar se o shader foi compilado
	GLint Result = GL_TRUE;
	glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &Result);

	if (Result == GL_FALSE)
	{
		// Erro ao compilar o shader, imprimir o log para saber o que est� errado
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

// Fun��o para carregar os programas de shaders
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
	glShaderSource(VertShaderId, 1, &VertexShaderSourcePtr, nullptr); // Chamada a fun��o que determina os par�metros
		// dos fontes que ser�o compilados, recebendo o Id, a quantidade de fontes a serem compilados (neste exemplo apenas 1),
		// os endere�os dos ponteiros e o comprimento da leitura (como utilizamos a fun��es c_str() ser� uma string com 
		// ponteiro nulo de termina��o)
	glCompileShader(VertShaderId); // Compila todos os Vertex Shaders parametrizados acima
	CheckShader(VertShaderId);

	std::cout << "Compilando " << FragmentShaderFile << std::endl;
	const char* FragmentShaderSourcePtr = FragmentShaderSource.c_str();
	glShaderSource(FragShaderId, 1, &FragmentShaderSourcePtr, nullptr);
	glCompileShader(FragShaderId);
	CheckShader(FragShaderId);

	// Feita a compila��o dos shaders, � necess�rio confeccionar o programa a ser carregado na pipeline.
	std::cout << "Linkando Programa" << std::endl;
	GLuint ProgramId = glCreateProgram(); // Elencar abaixo todos os shaders que fazem parte desse programa
	glAttachShader(ProgramId, VertShaderId);
	glAttachShader(ProgramId, FragShaderId);
	glLinkProgram(ProgramId); // Conclui o link entre os shaders compilados acima relacionados

	// Verificar resultado do link
	GLint Result = GL_TRUE;
	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Result); // Armazena o status de inicializa��o em Result
	
	if (Result == GL_FALSE) // Obter o log para compreens�o do problema
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

	// Como boa pr�tica, uma vez que utilizamos recursos � bom ilber�-los na sequ�ncia
	// (isso n�o desfaz o link com o programa ap�s a compila��o, apenas libera o uso dos Ids e pilhas de mem�ria para 
	//  evitar comportamentos indesejados e sujeiras de mem�ria)
	glDetachShader(ProgramId, VertShaderId);
	glDetachShader(ProgramId, FragShaderId);

	glDeleteShader(VertShaderId);
	glDeleteShader(FragShaderId);

	std::cout << "Programa associado, shaders carregados com sucesso" << std::endl;

	return ProgramId;
}

// Fun��o para carregar texturas a partir de arquivos com imagens
GLuint LoadTexture(const char* TextureFile)
{
	std::cout << "Carregando Textura " << TextureFile << std::endl;

	int TextureWidth = 0;
	int TextureHeight = 0;
	int NumberOfComponents = 0;

	// Recebe por par�metro um ponteiro para um arquivo, endere�os de tr�s vari�veis para armazenar os tamanhos da largura
	//	e da altura da imagem carregada, o n�mero de componentes dispon�vel e por fim � necess�rio especificar a quantidade
	//	de componentes que desejamos retornar (3 = RGB)
	// Textura em RAM
	unsigned char* TextureData = stbi_load(TextureFile, &TextureWidth, &TextureHeight, &NumberOfComponents, 3);
	
	assert(TextureData); // Caso algo d� errado durante o carregamento da textura, interrompe o processamento

	std::cout << "Textura carregada com sucesso" << std::endl;

	// Gerar o Identifador da Textura + procedimento para lev�-la para a mem�ria de v�deo
	GLuint TextureId;
	glGenTextures(1, &TextureId);

	// Habilita a textura para ser modificada (bind)
	glBindTexture(GL_TEXTURE_2D, TextureId); // 2D por ser uma imagem

	// Copiar a textura para a mem�ria de v�deo (GPU)
	// 	   Recebe por par�metro um alvo ou tipo de textura;
	//	   Um level de texturiza��o;
	// 	   Um formato interno de armazenamento da estrutura de dados;
	// 	   Largura;
	// 	   Altura;
	// 	   Uso de bordas;
	// 	   Formato novamente - para encapsulamento;
	// 	   Tipo de refer�ncia para os dados carregados - tipo de TextureData (char* = bytes)
	//	   Ponteiro para os dados (pixels) a serem transferidos para a GPU
	GLint Level = 0;
	GLint Border = 0;
	glTexImage2D(GL_TEXTURE_2D, Level, GL_RGB, TextureWidth, TextureHeight, Border, GL_RGB, GL_UNSIGNED_BYTE, TextureData);

	// Aplica��o de filtro de magnifica��o e minifica��o
	// Parametriza��o linear para suavizar granula��o com aumento de zoom
	// Mipmap para contornar aliasing da dist�ncia
	// Configurar o Texture Wrapping - extrapola��es das coordenadas normalizadas da img texturizada nas coordenadas S e T
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0); // Desligar a textura pois j� foi copiada para a GPU

	stbi_image_free(TextureData); // Pode liberar a RAM utilizada
	return TextureId;
}

// Fun��o callback para tratamento de eventos com clique do mouse
void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Modifiers)
{
	// std::cout << "Button: " << Button << " Action: " << Action << " Modifiers: " << Modifiers << std::endl;

	if (Button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (Action == GLFW_PRESS) // Ativa a a��o do cursor do mouse com clique do bot�o esquerdo do mouse
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Cursor desaparece durante o clique

			double X, Y;
			glfwGetCursorPos(Window, &X, &Y);
			
			Camera.PreviousCursor = glm::vec2{ X, Y };
			Camera.bEnableMouseMovement = true;
		}
		else if (Action == GLFW_RELEASE) // Desativa a a��o do cursor do mouse ao soltar o bot�o esquerdo do mouse
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Cursor retorna ap�s soltar o bot�o

			Camera.bEnableMouseMovement = false;
		}
	}
}

// Fun��o callback para tratamento de eventos com o movimento do cursor do mouse
// Essencialmente transmite � c�mera as coordenadas X, Y do monitor no ponto em que o cursor se encontra
void MouseMotionCallback(GLFWwindow* Window, double X, double Y)
{
	// std::cout << "X: " << X << " Y: " << Y << std::endl;
	Camera.MouseMove(X, Y);
}

// Fun��o callback para tratamento do movimento da c�mera utilizando teclado
// Escape para fechar a janela
// W,A,S,D para movimentar a c�mera para frente, esquerda, tr�s e direita, respectivamente 
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
	// Recebe como par�metros a largura e altura da janela, um t�tulo, um monitor (caso haja mais de um) e um 
	//	contexto, caso se deseje compartilhar a exibi��o
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
	//	Associa um objeto GLFW a um contexto, para que o GLEW possa inicializar com refer�ncia para esse contexto
	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1); // Habilita ou desabilita o V-Sync

	// Inicializar a biblioteca GLEW (deve ser criada ap�s a janela, pois � necess�rio um contexto de 
	//	OpenGL para que a API do OpenGL possa apontar)
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Erro ao inicializar o GLEW" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Verificar a vers�o do OpenGL
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

	// Escolhe a fun��o de teste de profundidade.
	glDepthFunc(GL_ALWAYS);

	// Otimiza��o de processamento: habilitar o Back Face Culling - n�o renderiza a face traseira dos objetos
	glDisable(GL_CULL_FACE);
	glEnable(GL_CULL_FACE);

	// Compilar o vertex e o fragment shader
	GLuint ProgramId = LoadShaders("shaders/triangle_vert.glsl", "shaders/triangle_frag.glsl");

	// Gera a Geometria da esfera e copia os dados para a GPU (mem�ria da placa de v�deo)
	std::vector<Vertex> SphereVertices;
	std::vector<Triangle> SphereIndices;
	GenerateSphere(100, SphereVertices, SphereIndices);
	GLuint SphereVertexBuffer, SphereElementBuffer; // VBO e EBO (Vertex e Element Buffer Objects)
	glGenBuffers(1, &SphereVertexBuffer); // Pedir para o OpenGL gerar o identificador do VBO e do EBO
	glGenBuffers(1, &SphereElementBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, SphereVertexBuffer); // Linkar/ativar o buffer ao seu tipo para o OpenGL
	// Copia efetivamente do buffer (mem�ria RAM) para a GPU (mem�ria de v�deo)
	glBufferData(GL_ARRAY_BUFFER, SphereVertices.size() * sizeof(Vertex), SphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereElementBuffer);	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, SphereIndices.size() * sizeof(Triangle), SphereIndices.data(), GL_STATIC_DRAW);

	// Criar uma fonte de luz direcional
	DirectionalLight Light;
	Light.Direction = glm::vec3(0.0f, 0.0f, -1.0f); // Z negativo, apontando para dentro da tela
	Light.Intensity = 1.0f;

	// Model Matrix - identidade rotacionada para viabilizar c�lculos com a Model View Projection - MVP
	glm::mat4 ModelMatrix = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });

	// Carregar as texturas para a mem�ria de v�deo
	//GLuint EarthTextureId = LoadTexture("textures/earth_2k.jpg");
	GLuint EarthTextureId = LoadTexture("textures/earth5400x2700.jpg");
	GLuint CloudsTextureId = LoadTexture("textures/earth_clouds_2k.jpg");

	// Configura a cor de fundo
	// **Ter em mente que o OpenGL � uma m�quina de estados (quando ativarmos algo, essa coisa permanecer� ativa por padr�o)
	//  Definir a cor do fundo (isso � um estado, o driver armazenar� essa informa��o:
	//	Sempre que precisarmos retomar essa informa��o, limparmos o framebuffer, etc, esse estado ser� devidamente restaurado)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0); // RGBA

	// Identificador do Vertex Array Object (VAO)
	GLuint SphereVAO;

	// Gera o identificador do VAO
	glGenVertexArrays(1, &SphereVAO);

	// Habilita o VAO
	glBindVertexArray(SphereVAO);

	// Ativa o atributo de v�rtice para o array. O par�metro representa o �ndice (location) do layout do shader ativo
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Ativa os buffers de v�rtice e de elemento para serem utilizados no contexto OpenGL 
	glBindBuffer(GL_ARRAY_BUFFER, SphereVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereElementBuffer);

	// Informa ao OpenGL onde os v�rtices se encontram dentro do VertexBuffer. 
	//  [0;3] s�o os �ndices habilitados, coincidindo com os especificados em glEnableVertexAttribArray() / location nos shaders
	//	[2;3] s�o as dimens�es (qtd) de v�rtices das estruturas de dados utilizadas (vec2 e vec3)
	//	GL_FLOAT � o tipo primitivo
	//  GL_FALSE / GL_TRUE para informar se os atributos est�o normalizados ou n�o 
	//	stride - tamanho do Vertex (struct) que definimos
	//	offset - para position � nulo, para color e os demais � calculado. O cast � necess�rio para compatibilizar o retorno
	//		     do m�todo offsetof com o par�metro recebido pela fun��o glVertexAttribPointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));	

	// Disabilitar o VAO
	glBindVertexArray(0);

	double PreviousTime = glfwGetTime(); // Tempo do frame anterior

	// Entra no loop de eventos da aplica��o tendo a janela fechada como condi��o de parada
	while (!glfwWindowShouldClose(Window))
	{	
		// Calcula o Delta Time para suaviza��o da movimenta��o da c�mera
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
		
		// C�lculos matriciais para determina��o da Matriz Normal (utilizada para a ilumina��o) e para a Model View Projection
		// MVP (utilizada para transladar, rotacionar e escalar os objetos no espa�o euclidiano)
		glm::mat4 ViewMatrix = Camera.GetView();
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrix));
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat4 ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrix;

		// Abaixo recupera��o dos uniformes calculados pelos shaders
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

		// Determina��o 
		glm::vec4 LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		GLint LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		// Ativa��o e endere�amento da textura para os shaders
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
		// Utiliza o EBO para desenhar na tela de acordo com os �ndices
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		// Processa todos os eventos da fila de eventos do GLFW podem ser est�mulos do teclado, mouse, gamepad, etc
		glfwPollEvents();

		// Envia o conte�do do framebuffer da janela para ser desenhado na tela
		// A mem�ria alocada para aplica��o � trocada (swap) para a mem�ria de v�deo que se encarregar� pela
		//	renderiza��o em tela dos pixels da janela
		//	Vale mencionar, portanto, que o tamanho da janela que estipulamos (valor das vari�veis Width e Height)
		//	influencia na quantidade de mem�ria RAM e de v�deo que nossa aplica��o utilizar�
		glfwSwapBuffers(Window);		
	}

	// Boa pr�tica em OpenGL: como ele se comporta como uma m�quina de estados, ap�s habilitar o buffer, 
	//	definir um contexto e desenhar coisas em tela, reverter o que foi criado para que as pr�ximas constru��es
	//	em tela sejam organizadas, novos binds rastre�veis e, em suma, o comportamento sist�mico seja controlado e 
	//	previs�vel. 
	glDeleteBuffers(1, &SphereElementBuffer);
	glDeleteBuffers(1, &SphereVertexBuffer);
	glDeleteVertexArrays(1, &SphereVAO);
	glDeleteProgram(ProgramId);
	glDeleteTextures(1, &EarthTextureId);

	glfwDestroyWindow(Window);
	glfwTerminate();

	return 0;
}
