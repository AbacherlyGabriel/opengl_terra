#include <iostream>
#include <fstream>
#include <cassert>
#include <array>

// N�o inclu�mos o GL.h pois nele constam apenas as fun��es do OpenGL 1.0 ou 1.1
// Por isso utilizamos o GLEW (Extension Wrangler) - fun��es mais novas j� incluindo as legadas automaticamente
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION	// Macro necess�ria para ativar o header STB
#include <stb_image.h>

// Constantes que determinam o tamanho da janela de contexto do GLFW
const int Width = 800;
const int Height = 600;

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP); // Mipmap para contornar aliasing da dist�ncia

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

int main() {

	assert(glfwInit() == GLFW_TRUE); // Caso d� problema de inicializa��o com a biblioteca GLFW ser� sinalizado com um false

	// Criar uma janela:
	// Recebe como par�metros a largura e altura da janela, um t�tulo, um monitor (caso haja mais de um) e um 
	//	contexto, caso se deseje compartilhar a exibi��o
	GLFWwindow* Window = glfwCreateWindow(Width, Height, "Blue Marble", nullptr, nullptr);
	assert(Window); // Testa se deu certo de criar a janela - se o ponteiro for nulo falhar�
	
	// Ativa o contexto criado na janela Window:
	//	Associa um objeto GLFW a um contexto, para que o GLEW possa inicializar com refer�ncia para esse contexto
	glfwMakeContextCurrent(Window); 

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

	GLuint ProgramId = LoadShaders("shaders/triangle_vert.glsl", "shaders/triangle_frag.glsl");
	// GLuint TextureId = LoadTexture("textures/earth_2k.jpg");
	GLuint TextureId = LoadTexture("textures/earth5400x2700.jpg");
	
	// Defini��o de um tri�ngulo em coordenadas normalizadas (array com 3 coordenadas/elementos de tipo vec3)
	std::array<Vertex, 6> Quad = { 
		Vertex { glm::vec3{ -1.0f, -1.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f } },
		Vertex { glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
		Vertex { glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } },
		Vertex { glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } },
		Vertex { glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
		Vertex { glm::vec3{  1.0f,  1.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f } }
	}; // Armazenado na RAM

	// MVP realizada manualmente nesse momento, para aprendizado, depois usaremos shaders para sua otimiza��o na GPU
	glm::mat4 ModelMatrix = glm::identity<glm::mat4>(); 

	glm::vec3 Eye{ 0, 0, 5 };
	glm::vec3 Center{ 0, 0, 0 };
	glm::vec3 Up{ 0, 1, 0 };
	glm::mat4 ViewMatrix = glm::lookAt(Eye, Center, Up);

	constexpr float FoV = glm::radians(45.0f);
	const float AspectRatio = Width / Height;
	const float Near = 0.001f;
	const float Far = 1000.0f;
	glm::mat4 ProjectionMatrix = glm::perspective(FoV, AspectRatio, Near, Far);

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; // Model View Projection Matrix

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
	GLuint VertexBuffer; // Buffer de v�rtices

	// Pedir para o OpenGL gerar o identificador do VertexBuffer
	glGenBuffers(1, &VertexBuffer); // Qtd e endere�o de quais buffers ser�o inicializados

	// Ativar o VertecBuffer como sendo o buffer para onde vamos copiar os dados do tri�ngulo
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	// Copiar os dados para a mem�ria de v�deo
	//	Recebe um alvo, uma constante que identifica o tipo de estrutura utilizada para armazenamento do buffer
	//	um tamanho em bytes que o buffer ir� administrar. Como definimos um array � poss�vel utilizar a fun��o sizeof()
	//	ponteiro para a estrutura de dados bufferizados. Utilizada a fun��o data() para retornar o ponteiro
	//	recebe um caso de uso - olhar flags na documenta��o para maiores informa��es
	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad), Quad.data(), GL_STATIC_DRAW);

	// Ter em mente que o OpenGL � uma m�quina de estados (quando ativarmos algo, essa coisa permanecer� ativa por padr�o)
	// Definir a cor do fundo (isso � um estado, o driver armazenar� essa informa��o:
	//	Sempre que precisarmos retomar essa informa��o, limparmos o framebuffer, etc, essa estado ser� devidamente restaurado)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // RGBAlpha

	// Entra no loop de eventos da aplica��o
	while (!glfwWindowShouldClose(Window)) { // Condi��o de parada, verifica se a janela foi fechada

		// glClear via limpar o framebuffer. GL_COLOR_BUFFER_BIT diz para limpar (preencher) o buffer de cor -> esse 
		//	preenchimento ser� realizado de acordo com a �ltima cor configurada via glClearColor
		// Quando formos desenhar geometrias 3D, voltaremos ao glClear pois teremos que limpar o buffer de 
		//	profundidade (depth buffer)
		glClear(GL_COLOR_BUFFER_BIT); // Ativa o bit do buffer que realiza a limpeza (ou mudan�a) do atributo cor

		// Ativa o programa de shader
		glUseProgram(ProgramId);

		// Recupera a localiza��o calculada pelo uniforme do Vertex Shader utilizando a ModelViewProjection
		GLint ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(MVP));

		// Ativa��o e endere�amento da textura para os shaders
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureId);

		GLint TextureSamplerLoc = glGetUniformLocation(ProgramId, "TextureSampler");
		glUniform1i(TextureSamplerLoc, 0); // Em que 0 est� identificando o ID do uniforme (GL_TEXTURE0)

		// Ativa o atributo de v�rtice para o array. O par�metro 0 representa o �ndice do layout do shader ativo
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// Ativa o buffer VertexBuffer para ser utilizado no contexto OpenGL 
		glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

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

		// Desenha em tela de acordo com os arrays atrelados ao buffer de dados (VertexBuffer), interpretando-os como 
		// coordenadas normalizadas. Especificamos o tipo pela constante do OpenGL (tri�ngulos), dizemos qual � o ponto de 
		// origem (0) e especificamos a quantidade de v�rtices que a estrutura possui (3)
		glDrawArrays(GL_TRIANGLES, 0, Quad.size()); 

		// Boa pr�tica em OpenGL: como ele se comporta como uma m�quina de estados, ap�s habilitar o buffer, 
		//	definir um contexto e desenhar coisas em tela, reverter o que foi criado para que as pr�ximas constru��es
		//	em tela sejam organizadas, novos binds rastre�veis e, em suma, o comportamento sist�mico seja controlado e 
		//	previs�vel. 
		// Portanto, para reverter o estado criado:
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Desabilita o programa ativo
		glUseProgram(0);
		
		// Processa todos os eventos da fila de eventos do GLFW
		//	podem ser est�mulos do teclado, mouse, gamepad, etc
		glfwPollEvents(); 

		// Envia o conte�do do framebuffer da janela para ser desenhado na tela
		// A mem�ria alocada para aplica��o � trocada (swap) para a mem�ria de v�deo que se encarregar� pela
		//	renderiza��o em tela dos pixels da janela
		//	Vale mencionar, portanto, que o tamanho da janela que estipulamos (valor das vari�veis Width e Height)
		//	influencia na quantidade de mem�ria RAM e de v�deo que nossa aplica��o utilizar�
		glfwSwapBuffers(Window);
	}

	glDeleteBuffers(1, &VertexBuffer); // Desaloca (1) VertexBuffer
	glfwTerminate(); // Encerra a biblioteca GLFW

	return 0;
}