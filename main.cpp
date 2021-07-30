#include <iostream>
#include <fstream>
#include <cassert>
#include <array>
#include <vector>

// N�o inclu�mos o GL.h pois nele constam apenas as fun��es do OpenGL 1.0 ou 1.1
// Por isso utilizamos o GLEW (Extension Wrangler) - fun��es mais novas j� incluindo as legadas automaticamente
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION	// Macro necess�ria para ativar o header STB
#include <stb_image.h>

// Constantes que determinam o tamanho da janela de contexto do GLFW
int Width = 800;
int Height = 600;

// Fun��o para leitura de arquivos
std::string ReadFile(const char* FilePath) {
	std::string FileContents;
	
	if (std::ifstream FileStream{ FilePath, std::ios::in }) { // Se entrar no if, foi poss�vel criar a stream de leitura
		// Armazena em FileContents o conte�do do arquivo referenciado por FilePath
		FileContents.assign( std::istreambuf_iterator<char>(FileStream), std::istreambuf_iterator<char>() );
	}

	return FileContents;
}

// Fun��o para verifica��o do log de compila��o do shader (recebe o identificador de um shader compilado como par�metro)
void CheckShader(GLuint ShaderId) {
	GLint Result = GL_TRUE;

	glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &Result);

	if (Result == GL_FALSE) { // Erro de compila��o
		// Impress�o do log para identifica��o da causa do erro
		// Obter tamanho do log
		GLint InfoLogLength = 0;

		glGetShaderiv(ShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength); // Quantidade em bytes a ser alocado na string

		if (InfoLogLength > 0) {
			std::string ShaderInfoLog(InfoLogLength, '\0'); // Inicializar string de tamanho InfoLogLength com todos 
															// os char = '0'
			glGetShaderInfoLog(ShaderId, InfoLogLength, nullptr, &ShaderInfoLog[0]); // Recupera o log armazenando em 
																					 // ShaderInfoLog
			std::cout << "Erro no shader, verificar log abaixo" << std::endl;
			std::cout << ShaderInfoLog << std::endl;
			
			assert(false); // Deu erro no shader, interrompe o programa
		}
	}
}

// Fun��o para carregar os programas de shaders
GLuint LoadShaders(const char* VertexShaderFile, const char* FragmentShaderFile) {
	std::string VertexShaderSource = ReadFile(VertexShaderFile);
	std::string FragmentShaderSource = ReadFile(FragmentShaderFile);

	assert(!VertexShaderSource.empty());
	assert(!FragmentShaderSource.empty());

	// Criar os identificadores do Vertex e do Fragment Shaders
	GLuint VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Utilizar o OpenGL para compilar os shaders
	std::cout << "Compilando " << VertexShaderFile << std::endl;
	const char* VertexShaderSourcePtr = VertexShaderSource.c_str(); // Ponteiro para o fonte do Vertex Shader
	glShaderSource(VertexShaderId, 1, &VertexShaderSourcePtr, nullptr); // Chamada a fun��o que determina os par�metros
		// dos fontes que ser�o compilados, recebendo o Id, a quantidade de fontes a serem compilados (neste exemplo apenas 1),
		// os endere�os dos ponteiros e o comprimento da leitura (como utilizamos a fun��es c_str() ser� uma string com 
		// ponteiro nulo de termina��o)
	glCompileShader(VertexShaderId); // Compila todos os Vertex Shaders parametrizados acima
	CheckShader(VertexShaderId);

	std::cout << "Compilando " << FragmentShaderFile << std::endl;
	const char* FragmentShaderSourcePtr = FragmentShaderSource.c_str();
	glShaderSource(FragmentShaderId, 1, &FragmentShaderSourcePtr, nullptr);
	glCompileShader(FragmentShaderId);
	CheckShader(FragmentShaderId);

	// Feita a compila��o dos shaders, � necess�rio confeccionar o programa a ser carregado na pipeline.
	std::cout << "Associando o programa" << std::endl;
	GLuint ProgramId = glCreateProgram(); // Elencar abaixo todos os shaders que fazem parte desse programa
	glAttachShader(ProgramId, VertexShaderId);
	glAttachShader(ProgramId, FragmentShaderId);
	glLinkProgram(ProgramId); // Conclui o link entre os shaders compilados relacionados

	// Verificar se o programa foi linkado corretamente
	GLint Result = GL_TRUE;
	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Result); // Armazena o status de inicializa��o em Result

	if (Result == GL_FALSE) {
		// Obter o log para compreens�o do problema
		GLint InfoLogLength = 0;

		glGetProgramiv(ProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength); // Quantidade em bytes a ser alocado na string

		if (InfoLogLength > 0) {
			std::string ProgramInfoLog(InfoLogLength, '\0'); // Inicializar string de tamanho InfoLogLength com todos 
															 // os char = '0'
			glGetProgramInfoLog(ProgramId, InfoLogLength, nullptr, &ProgramInfoLog[0]); // Recupera o log armazenando em 
																					    // ProgramInfoLog
			std::cout << "Erro ao linkar programa aos shaders" << std::endl;
			std::cout << ProgramInfoLog << std::endl;
			
			assert(false);
		}
	}

	// Como boa pr�tica, uma vez que utilizamos recursos � bom ilber�-los na sequ�ncia
	// (isso n�o desfaz o link com o programa ap�s a compila��o, apenas libera o uso dos Ids e pilhas de mem�ria para 
	//  evitar comportamentos indesejados e sujeiras de mem�ria)
	glDetachShader(ProgramId, VertexShaderId);
	glDetachShader(ProgramId, FragmentShaderId);

	glDeleteShader(VertexShaderId);
	glDeleteShader(FragmentShaderId);

	std::cout << "Programa associado, shaders carregados com sucesso" << std::endl;

	return ProgramId;
}

GLuint LoadTexture(const char* TextureFile) {
	std::cout << "Carregando textura" << std::endl;

	stbi_set_flip_vertically_on_load(true);

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

	// Gerar o identificador da textura - procedimento para lev�-la para a mem�ria de v�deo
	GLuint TextureId;
	glGenTextures(1, &TextureId); // Apenas uma textura inicializada

	// Habilitar a textura (bind) para ser modificada
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureData);

	// Aplica��o de filtro de magnifica��o e minifica��o
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear para suavizar granula��o com mais zoom
	// Mipmap para contornar aliasing da dist�ncia
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Configurar o Texture Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Extrapola��es das coordenadas normalizadas da img tex
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Coordenadas S e T (x e y)

	// Gerar o Mipmap a partir da textura
	glGenerateMipmap(GL_TEXTURE_2D);

	// Desligar a textura pois j� foi copiada para a GPU
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(TextureData); // Pode liberar a RAM utilizada

	return TextureId;
}

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec2 UV;
};

// Fun��o para carregar toda a geometria desenhada, respons�vel por encapsular os binds e unbinds centralizando a chamada
GLuint LoadGeometry() {
	// Defini��o de um tri�ngulo em coordenadas normalizadas (array com 3 coordenadas/elementos de tipo vec3)
	std::array<Vertex, 6> Quad = {
		Vertex { glm::vec3{ -1.0f, -1.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f } },
		Vertex { glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
		Vertex { glm::vec3{  1.0f,  1.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f } },
		Vertex { glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } }
		//Vertex { glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } },
		//Vertex { glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
	}; // Armazenado na RAM

	// Definir lista de elementos que formam os tri�ngulos
	std::array<glm::ivec3, 2> Indices = {
		glm::ivec3{ 0, 1, 3},
		glm::ivec3{ 3, 1, 2}
	};

	// N�o h� mais necessidade de calcular a MVP manualmente para definir a c�mera. Inv�s disso, basta instanciar o objeto
	// 	   da c�mera a�rea
	//glm::vec3 Eye{ 0, 0, 5 };
	//glm::vec3 Center{ 0, 0, 0 };
	//glm::vec3 Up{ 0, 1, 0 };
	//glm::mat4 ViewMatrix = glm::lookAt(Eye, Center, Up);

	//constexpr float FoV = glm::radians(45.0f);
	//const float AspectRatio = Width / Height;
	//const float Near = 0.001f;
	//const float Far = 1000.0f;
	//glm::mat4 ProjectionMatrix = glm::perspective(FoV, AspectRatio, Near, Far);

	// -------------------------
	// 	   A MVP agora passar a ser calculada no loop de eventos, para que a c�mera (e a perspectiva) seja atualizada
	// 	   conforme as intera��es
	//glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; // Model View Projection Matrix

	// Aplicar a MVP nos v�rtices do tri�ngulo
	// 	   Ao atualizarmos o vertex shader a pipeline gr�fica realizar� esses c�lculos de maneira otimizada, j�
	// 	   dividindo pela coordenada homog�nea, simplificando a programa��o da aplica��o
	//for (Vertex& Vertex : Triangle) {
	//	glm::vec4 ProjectedVertex = MVP * glm::vec4{ Vertex.Position, 1.0f }; // Construindo um vec4 a partir de um vec3,
																			  // acrescentando estaticamente apenas a 
																			  //  �ltima coordenada (w)
	//	ProjectedVertex /= ProjectedVertex.w; // Divide todas as coordenadas do v�rtice projetado pela componente w, 
											  // para que w se torne 1 e os demais valores fiquem 'normalizados'
	//	Vertex.Position = ProjectedVertex; // Altera-se por fim a refer�ncia do v�rtice do tri�ngulo
	//}

	// Copiar v�rtices do tri�ngulo para a mem�ria da GPU
	GLuint VertexBuffer; // Buffer de v�rtices - identificador do VBO - Vertex Buffer Object
	GLuint ElementBuffer = 0; // Solicitar para o OpenGL gerar o identificador do EBO - Element Buffer Object

	// Pedir para o OpenGL gerar o identificador do VBO e do EBO
	glGenBuffers(1, &VertexBuffer); // Qtd e endere�o de quais buffers ser�o inicializados
	glGenBuffers(1, &ElementBuffer);

	// Ativar o VertexBuffer e o Element Buffer como sendo os buffers para onde copiaremos os dados do tri�ngulo
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);

	// Copiar os dados para a mem�ria de v�deo
	//	Recebe um alvo, uma constante que identifica o tipo de estrutura utilizada para armazenamento do buffer,
	//	um tamanho em bytes que o buffer ir� administrar. Como definimos um array � poss�vel utilizar a fun��o sizeof()
	//	ponteiro para a estrutura de dados bufferizados. Utilizada a fun��o data() para retornar o ponteiro
	//	recebe um caso de uso - olhar flags na documenta��o para maiores informa��es
	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad), Quad.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices.data(), GL_STATIC_DRAW);

	// Gerar o Vertex Array Object - VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	// Habilitar o VAO
	glBindVertexArray(VAO);

	// Ativa o atributo de v�rtice para o array. O par�metro 0 representa o �ndice do layout do shader ativo
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Ativa o buffer VertexBuffer e o ElementBuffer para serem utilizados no contexto OpenGL 
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);

	// Informa ao OpenGL onde os v�rtices se encontram dentro do VertexBuffer. Como o array Triangle � cont�guo
	// em mem�ria, basta apenas dizer quantos v�rtices ser�o utilizados para desenhar o tri�ngulo
	//  0 / 1 � o �ndice habilitado, deve coincidir com o especificado na chamada � fun��o glEnableVertexAttribArray()
	//	3 � a quantidade de v�rtices da estrutura de dados (vec3)
	//	GL_FLOAT � o tipo de dados 
	//  GL_FALSE / GL_TRUE para informar se os atributos est�o normalizados ou n�o 
	//	stride - tamanho do Vertex (struct) que definimos
	//	offset - para position � nulo, para color � calculado. O cast � necess�rio para compatibilizar o retorno
	//		     do m�todo offsetof com o par�metro recebido pela fun��o glVertexAttribPointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));

	glBindVertexArray(0); // Desabilita o VAO e retorna o estado anterior do processamento pelo OpenGL

	return VAO;	
}

// Fun��o para gerar v�rtices da esfera
// A equa��o para c�lculo dos v�rtices � expressa por:
// x = x_0 + r sinTheta cosPhi
// y = y_0 + r sinTheta sinPhi
// z = z_0 + r cosTheta
// * Como podemos utilizar a MVP para transladar, rotacionar ou escalar nossa geometria, podemos simplificar a equa��o
//  gerando com origem em (0,0,0) e utilizando raio = 1
void GenerateSphereMesh(GLuint Resolution, std::vector<Vertex>& Vertices, std::vector<glm::ivec3>& Indices) {
	Vertices.clear(); // Apenas garantindo a inicializa��o correta
	Indices.clear();

	constexpr float Pi = glm::pi<float>();
	constexpr float TwoPi = glm::two_pi<float>();
	const float InvResolution = 1.0f / static_cast<float>(Resolution - 1); // Para n�o cair fora do array de resolu��o

	for (GLuint UIndex = 0; UIndex < Resolution; ++UIndex) {
		const float U = UIndex * InvResolution; // Obtemos um n�mero entre 0 (lado esquerdo) e 1 (lado direito) - coord U
		const float Theta = glm::mix(0.0f, Pi, static_cast<float>(U)); // Interpola��o linear para obter um Theta entre 0 e PI

		for (GLuint VIndex = 0; VIndex < Resolution; ++VIndex) {
			const float V = VIndex * InvResolution; // Obtemos um n�mero entre 0 (lado esquerdo) e 1 (lado direito) - coord V
			const float Phi = glm::mix(0.0f, TwoPi, static_cast<float>(V)); // Interpola��o linear para obter um Theta entre 0 e 2PI

			glm::vec3 VertexPosition = {
				glm::sin(Theta) * glm::cos(Phi),
				glm::sin(Theta) * glm::sin(Phi),
				glm::cos(Theta)
			};

			Vertex Vertex{
				VertexPosition,
				glm::vec3{ 1.0f, 1.0f, 1.0f }, // Branco, mas a cor n�o est� sendo utilizada no momento
				glm::vec2{ 1.0f - U, V }
			};

			Vertices.push_back(Vertex); // Carrega no array enviado pelo par�metro
		}
	}

	for (GLuint U = 0; U < Resolution - 1; ++U) {
		for (GLuint V = 0; V < Resolution - 1; ++V) { // Indexando os pontos que formam os quads (e tri�ngulos) da malha
													  // que ir�o compor a esfera
			GLuint P0 = U + V * Resolution;
			GLuint P1 = (U + 1) + V * Resolution;
			GLuint P2 = (U + 1) + (V + 1) * Resolution;
			GLuint P3 = U + (V + 1) * Resolution;

			// O quad ser� formado por dois tri�ngulos cortando a sua diagonal. Tendo (0,0) como origem, os pontos ficariam:
			// Primeiro tri�ngulo: (0, 0), (1,0) e (0,1)
			// Segundo tri�ngulo: (0,1), (1,0) e (1,1)
			// Observar que assim � poss�vel reaproveitar v�rtices de um quad para outro, otimizando o modelo
			Indices.push_back(glm::ivec3{ P0, P1, P3 }); 
			Indices.push_back(glm::ivec3{ P3, P1, P2 });
		}
	}
}

// Fun��o para carregar a geometria da esfera
GLuint LoadSphere(GLuint& NumVertices, GLuint& NumIndices) {
	std::vector<Vertex> Vertices;
	std::vector<glm::ivec3> Triangles; // �ndices das coordenadas dos tri�ngulos que formam os quads da malha da esfera
	
	GenerateSphereMesh(50, Vertices, Triangles);

	NumVertices = Vertices.size();
	NumIndices = Triangles.size() * 3; // Multiplicado por tr�s, pois cada elemento possui 3 �ndices de v�rtices

	// Daqui para baixo apenas repeti��es de comandos para copiar a geometria da esfera da CPU para a GPU
	GLuint ElementBuffer; // EBO
	glGenBuffers(1, &ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumIndices * sizeof(GLuint), Triangles.data(), GL_STATIC_DRAW);

	GLuint VertexBuffer;
	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertex), Vertices.data(), GL_STATIC_DRAW);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));
	glBindVertexArray(0);

	return VAO;
}

class FlyCamera {
public: 
	// Fun��o que permite o movimento frente/tr�s da c�mera
	void MoveForward(float Amount) { // Amount = deslocamento de um frame
		Location += glm::normalize(Direction) * Amount * Speed; // boa pr�tica: normaliza��o vetorial
	}

	// Fun��o que permite o movimento direita/esquerda da c�mera
	void MoveRight(float Amount) {
		glm::vec3 Right = glm::normalize(glm::cross(Direction, Up)); // boa pr�tica: normaliza��o
		Location += Right * Amount * Speed;
	}

	// Fun��o para rota��o da c�mera utilizando o mouse (sobre os eixos y e x)
	void Look(float Yaw, float Pitch) {
		Yaw *= Sensitivity;
		Pitch *= Sensitivity;

		const glm::vec3 Right = glm::normalize(glm::cross(Direction, Up));
		const glm::mat4 IdentityMatrix = glm::identity<glm::mat4>();

		glm::mat4 YawRotation = glm::rotate(IdentityMatrix, glm::radians(Yaw), Up);
		glm::mat4 PitchRotation = glm::rotate(IdentityMatrix, glm::radians(Pitch), Right);

		Up = PitchRotation * glm::vec4{ Up, 0.0f };
		Direction = YawRotation * PitchRotation * glm::vec4{ Direction, 0.0f };
	}

	// Fun��o que retorna a MVP
	glm::mat4 GetViewProjection() const {
		glm::mat4 View = glm::lookAt(Location, Location + Direction, Up);
		glm::mat4 Projection = glm::perspective(FieldOfView, AspectRatio, Near, Far);

		return Projection * View;
	}

	// Par�metros de interatividade
	float Speed = 5.0f;
	float Sensitivity = 0.1f;

	// Defini��o da Matriz de View
	glm::vec3 Location{ 0.0f, 0.0f, 5.0f };
	glm::vec3 Direction{ 0.0f, 0.0f, -1.0f };
	glm::vec3 Up{ 0.0f, 1.0f, 0.0f };

	// Defini��o da Matriz Projection
	float FieldOfView = glm::radians(45.0f);
	float AspectRatio = Width / Height;
	float Near = 0.01f;
	float Far = 1000.0f;
};

FlyCamera Camera;					  // Inst�ncia do objeto c�mera m�vel (a�rea)
bool bEnableMouseMovement = false;    // Booleano para controle da a��o do mouse via cursor
glm::vec2 PreviousCursor{ 0.0, 0.0 }; // Para delta do cursor do mouse

// Fun��o callback para tratamento de eventos com clique do mouse
void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Modifiers) {
	if (Button == GLFW_MOUSE_BUTTON_LEFT) {
		if (Action == GLFW_PRESS) { // Ativa a a��o do cursor do mouse com clique do bot�o esquerdo do mouse
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Cursor desaparece durante o clique

			double X, Y;
			glfwGetCursorPos(Window, &X, &Y);

			PreviousCursor = glm::vec2{ X, Y };

			bEnableMouseMovement = true;
		}
		if (Action == GLFW_RELEASE) {// Desativa a a��o do cursor do mouse ao soltar o bot�o esquerdo do mouse
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Cursor retorna ap�s soltar o bot�o
			
			bEnableMouseMovement = false;
		}
	}
}

// Fun��o callback para tratamento de eventos com o movimento do cursor do mouse
void MouseMotionCallback(GLFWwindow* Window, double X, double Y) {
	if (bEnableMouseMovement) {
		glm::vec2 CurrentCursor{ X, Y };
		glm::vec2 DeltaCursor = CurrentCursor - PreviousCursor;

		Camera.Look(DeltaCursor.x, DeltaCursor.y);

		PreviousCursor = CurrentCursor;
	}
}

// Fun��o callback para redimensionar o modelo conforme a janela mudar
void Resize(GLFWwindow* Window, int NewWidth, int NewHeight) {
	Width = NewWidth;
	Height = NewHeight;

	Camera.AspectRatio = static_cast<float>(Width) / Height;

	glViewport(0, 0, Width, Height); // Fun��o do OepnGL que retorna o tamanho da janela que estamos utilizando
}

int main() {

	assert(glfwInit() == GLFW_TRUE); // Caso d� problema de inicializa��o com a biblioteca GLFW ser� sinalizado com um false

	// Criar uma janela:
	// Recebe como par�metros a largura e altura da janela, um t�tulo, um monitor (caso haja mais de um) e um 
	//	contexto, caso se deseje compartilhar a exibi��o
	GLFWwindow* Window = glfwCreateWindow(Width, Height, "Blue Marble", nullptr, nullptr);
	assert(Window); // Testa se deu certo de criar a janela - se o ponteiro for nulo falhar�

	// Cadastrar as callbacks no GLFW
	glfwSetMouseButtonCallback(Window, MouseButtonCallback);
	glfwSetCursorPosCallback(Window, MouseMotionCallback);
	glfwSetFramebufferSizeCallback(Window, Resize);
	
	// Ativa o contexto criado na janela Window:
	//	Associa um objeto GLFW a um contexto, para que o GLEW possa inicializar com refer�ncia para esse contexto
	glfwMakeContextCurrent(Window); 

	// Habilita ou desabilita o V-Sync
	glfwSwapInterval(1);

	// Inicializar a biblioteca GLEW (deve ser criada ap�s a janela, pois � necess�rio um contexto de 
	//	OpenGL para que a API do OpenGL possa apontar)
	assert(glewInit() == GLEW_OK);

	// Verificar a vers�o do OpenGL
	GLint GLMajorVersion = 0; // GLint � o tipo primitivo int do OpenGL (boa pr�tica para evitar problemas)
	GLint GLMinorVersion = 0;

	glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVersion); // Recupera as vers�es utilizadas pelo Driver
	glGetIntegerv(GL_MINOR_VERSION, &GLMinorVersion);

	std::cout << "OpenGL Version : " << GLMajorVersion << "." << GLMinorVersion << std::endl; // Vers�o
	std::cout << "OpenGL Vendor  : " << glGetString(GL_VENDOR) << std::endl;	  			  // Fabricante
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;				  // Placa de v�deo (renderizador)
	std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;				  // String com a vers�o completa
	std::cout << "GLSL   Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;// Vers�o da linguagem de shader
	std::cout << std::endl;

	// Ajusta a imagem da esfera para sua primeira renderiza��o arredondada (resize corrige o aspect ratio)
	Resize(Window, Width, Height);

	GLuint ProgramId = LoadShaders("shaders/triangle_vert.glsl", "shaders/triangle_frag.glsl");
	GLuint TextureId = LoadTexture("textures/earth_2k.jpg");
	//GLuint TextureId = LoadTexture("textures/earth5400x2700.jpg");

	GLuint QuadVAO = LoadGeometry();

	GLuint SphereNumVertices = 0;
	GLuint SphereNumIndices = 0;
	GLuint SphereVAO = LoadSphere(SphereNumVertices, SphereNumIndices);
	
	// Model Matrix - identidade rotacionada
	glm::mat4 Identity = glm::identity<glm::mat4>(); 
	glm::mat4 ModelMatrix = glm::rotate(Identity, glm::radians(90.0f), glm::vec3{ 1, 1, 0 });

	// Ter em mente que o OpenGL � uma m�quina de estados (quando ativarmos algo, essa coisa permanecer� ativa por padr�o)
	// Definir a cor do fundo (isso � um estado, o driver armazenar� essa informa��o:
	//	Sempre que precisarmos retomar essa informa��o, limparmos o framebuffer, etc, essa estado ser� devidamente restaurado)
	glClearColor(0.2f, 0.2f, 0.2f, 0.8f); // RGBAlpha

	// Armazenamento do frame anterior
	double PreviousTime = glfwGetTime();

	// Otimiza��o de processamento: habilitar o Back Face Culling - n�o renderiza a face traseira dos objetos
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Entra no loop de eventos da aplica��o
	while (!glfwWindowShouldClose(Window)) { // Condi��o de parada, verifica se a janela foi fechada

		// Calcula o Delta Time para suaviza��o da movimenta��o da c�mera
		double CurrentTime = glfwGetTime();
		double DeltaTime = CurrentTime - PreviousTime;

		if (DeltaTime > 0.0) {
			PreviousTime = CurrentTime;
		}

		// glClear via limpar o framebuffer. GL_COLOR_BUFFER_BIT diz para limpar (preencher) o buffer de cor -> esse 
		//	preenchimento ser� realizado de acordo com a �ltima cor configurada via glClearColor
		// Quando formos desenhar geometrias 3D, voltaremos ao glClear pois teremos que limpar o buffer de 
		//	profundidade (depth buffer)
		glClear(GL_COLOR_BUFFER_BIT); // Ativa o bit do buffer que realiza a limpeza (ou mudan�a) do atributo cor

		// Ativa o programa de shader
		glUseProgram(ProgramId);

		// Obten��o da MVP por meio do objeto FlyCamera
		glm::mat4 ViewProjectionMatrix = Camera.GetViewProjection();
		glm::mat4 ModelViewProjection = ViewProjectionMatrix * ModelMatrix;

		// Recupera a localiza��o calculada pelo uniforme do Vertex Shader utilizando a ModelViewProjection
		GLint ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjection));

		// Ativa��o e endere�amento da textura para os shaders
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureId);

		GLint TextureSamplerLoc = glGetUniformLocation(ProgramId, "TextureSampler");
		glUniform1i(TextureSamplerLoc, 0); // Em que 0 est� identificando o ID do uniforme (GL_TEXTURE0)

		//glBindVertexArray(QuadVAO);
		glBindVertexArray(SphereVAO);

		// Para testes de geometrias:
		glPointSize(3.0f);
		glLineWidth(3.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glDrawArrays(GL_POINTS, 0, SphereNumVertices);

		// Utiliza o EBO para desenhar na tela de acordo com os �ndices
		glDrawElements(GL_TRIANGLES, SphereNumIndices, GL_UNSIGNED_INT, nullptr);
		

		// Boa pr�tica em OpenGL: como ele se comporta como uma m�quina de estados, ap�s habilitar o buffer, 
		//	definir um contexto e desenhar coisas em tela, reverter o que foi criado para que as pr�ximas constru��es
		//	em tela sejam organizadas, novos binds rastre�veis e, em suma, o comportamento sist�mico seja controlado e 
		//	previs�vel. 
		// Portanto, para reverter o estado criado:
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);
		//glDisableVertexAttribArray(2);

		glBindVertexArray(0); // Desabilita o VAO
		glUseProgram(0); // Desabilita o programa ativo
		
		// Processa todos os eventos da fila de eventos do GLFW
		//	podem ser est�mulos do teclado, mouse, gamepad, etc
		glfwPollEvents(); 

		// Envia o conte�do do framebuffer da janela para ser desenhado na tela
		// A mem�ria alocada para aplica��o � trocada (swap) para a mem�ria de v�deo que se encarregar� pela
		//	renderiza��o em tela dos pixels da janela
		//	Vale mencionar, portanto, que o tamanho da janela que estipulamos (valor das vari�veis Width e Height)
		//	influencia na quantidade de mem�ria RAM e de v�deo que nossa aplica��o utilizar�
		glfwSwapBuffers(Window);

		// Ap�s desenhar os frames anteriores, processar os Inputs do teclado
		if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
			Camera.MoveForward(1.0f * DeltaTime); // Amount passada nos par�metros
		if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
			Camera.MoveForward(-1.0f * DeltaTime); 
		if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
			Camera.MoveRight(1.0f * DeltaTime); // Amount passada nos par�metros
		if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
			Camera.MoveRight(-1.0f * DeltaTime);
	}

	//glDeleteBuffers(1, &VertexBuffer); // Desaloca (1) VertexBuffer
	glDeleteVertexArrays(1, &QuadVAO); // Desaloca VAO
	glfwTerminate(); // Encerra a biblioteca GLFW

	return 0;
}