#include <iostream>
#include <fstream>
#include <cassert>
#include <array>

// Não incluímos o GL.h pois nele constam apenas as funções do OpenGL 1.0 ou 1.1
// Por isso utilizamos o GLEW (Extension Wrangler) - funções mais novas já incluindo as legadas automaticamente
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION	// Macro necessária para ativar o header STB
#include <stb_image.h>

// Constantes que determinam o tamanho da janela de contexto do GLFW
const int Width = 800;
const int Height = 600;

// Função para leitura de arquivos
std::string ReadFile(const char* FilePath) {
	std::string FileContents;
	
	if (std::ifstream FileStream{ FilePath, std::ios::in }) { // Se entrar no if, foi possível criar a stream de leitura
		// Armazena em FileContents o conteúdo do arquivo referenciado por FilePath
		FileContents.assign( std::istreambuf_iterator<char>(FileStream), std::istreambuf_iterator<char>() );
	}

	return FileContents;
}

// Função para verificação do log de compilação do shader (recebe o identificador de um shader compilado como parâmetro)
void CheckShader(GLuint ShaderId) {
	GLint Result = GL_TRUE;

	glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &Result);

	if (Result == GL_FALSE) { // Erro de compilação
		// Impressão do log para identificação da causa do erro
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

// Função para carregar os programas de shaders
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
	glShaderSource(VertexShaderId, 1, &VertexShaderSourcePtr, nullptr); // Chamada a função que determina os parâmetros
		// dos fontes que serão compilados, recebendo o Id, a quantidade de fontes a serem compilados (neste exemplo apenas 1),
		// os endereços dos ponteiros e o comprimento da leitura (como utilizamos a funções c_str() será uma string com 
		// ponteiro nulo de terminação)
	glCompileShader(VertexShaderId); // Compila todos os Vertex Shaders parametrizados acima
	CheckShader(VertexShaderId);

	std::cout << "Compilando " << FragmentShaderFile << std::endl;
	const char* FragmentShaderSourcePtr = FragmentShaderSource.c_str();
	glShaderSource(FragmentShaderId, 1, &FragmentShaderSourcePtr, nullptr);
	glCompileShader(FragmentShaderId);
	CheckShader(FragmentShaderId);

	// Feita a compilação dos shaders, é necessário confeccionar o programa a ser carregado na pipeline.
	std::cout << "Associando o programa" << std::endl;
	GLuint ProgramId = glCreateProgram(); // Elencar abaixo todos os shaders que fazem parte desse programa
	glAttachShader(ProgramId, VertexShaderId);
	glAttachShader(ProgramId, FragmentShaderId);
	glLinkProgram(ProgramId); // Conclui o link entre os shaders compilados relacionados

	// Verificar se o programa foi linkado corretamente
	GLint Result = GL_TRUE;
	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Result); // Armazena o status de inicialização em Result

	if (Result == GL_FALSE) {
		// Obter o log para compreensão do problema
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

	// Como boa prática, uma vez que utilizamos recursos é bom ilberá-los na sequência
	// (isso não desfaz o link com o programa após a compilação, apenas libera o uso dos Ids e pilhas de memória para 
	//  evitar comportamentos indesejados e sujeiras de memória)
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

	// Recebe por parâmetro um ponteiro para um arquivo, endereços de três variáveis para armazenar os tamanhos da largura
	//	e da altura da imagem carregada, o número de componentes disponível e por fim é necessário especificar a quantidade
	//	de componentes que desejamos retornar (3 = RGB)
	// Textura em RAM
	unsigned char* TextureData = stbi_load(TextureFile, &TextureWidth, &TextureHeight, &NumberOfComponents, 3);

	assert(TextureData); // Caso algo dê errado durante o carregamento da textura, interrompe o processamento

	std::cout << "Textura carregada com sucesso" << std::endl;

	// Gerar o identificador da textura - procedimento para levá-la para a memória de vídeo
	GLuint TextureId;
	glGenTextures(1, &TextureId); // Apenas uma textura inicializada

	// Habilitar a textura (bind) para ser modificada
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureData);

	// Aplicação de filtro de magnificação e minificação
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear para suavizar granulação com mais zoom
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP); // Mipmap para contornar aliasing da distância

	// Configurar o Texture Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Extrapolações das coordenadas normalizadas da img tex
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Coordenadas S e T (x e y)

	// Gerar o Mipmap a partir da textura
	glGenerateMipmap(GL_TEXTURE_2D);

	// Desligar a textura pois já foi copiada para a GPU
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

	assert(glfwInit() == GLFW_TRUE); // Caso dê problema de inicialização com a biblioteca GLFW será sinalizado com um false

	// Criar uma janela:
	// Recebe como parâmetros a largura e altura da janela, um título, um monitor (caso haja mais de um) e um 
	//	contexto, caso se deseje compartilhar a exibição
	GLFWwindow* Window = glfwCreateWindow(Width, Height, "Blue Marble", nullptr, nullptr);
	assert(Window); // Testa se deu certo de criar a janela - se o ponteiro for nulo falhará
	
	// Ativa o contexto criado na janela Window:
	//	Associa um objeto GLFW a um contexto, para que o GLEW possa inicializar com referência para esse contexto
	glfwMakeContextCurrent(Window); 

	// Inicializar a biblioteca GLEW (deve ser criada após a janela, pois é necessário um contexto de 
	//	OpenGL para que a API do OpenGL possa apontar)
	assert(glewInit() == GLEW_OK);

	// Verificar a versão do OpenGL
	GLint GLMajorVersion = 0; // GLint é o tipo primitivo int do OpenGL (boa prática para evitar problemas)
	GLint GLMinorVersion = 0;

	glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVersion); // Recupera as versões utilizadas pelo Driver
	glGetIntegerv(GL_MINOR_VERSION, &GLMinorVersion);

	std::cout << "OpenGL Version : " << GLMajorVersion << "." << GLMinorVersion << std::endl; // Versão
	std::cout << "OpenGL Vendor  : " << glGetString(GL_VENDOR) << std::endl;	  			  // Fabricante
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;				  // Placa de vídeo (renderizador)
	std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;				  // String com a versão completa
	std::cout << "GLSL   Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;// Versão da linguagem de shader
	std::cout << std::endl;

	GLuint ProgramId = LoadShaders("shaders/triangle_vert.glsl", "shaders/triangle_frag.glsl");
	// GLuint TextureId = LoadTexture("textures/earth_2k.jpg");
	GLuint TextureId = LoadTexture("textures/earth5400x2700.jpg");
	
	// Definição de um triângulo em coordenadas normalizadas (array com 3 coordenadas/elementos de tipo vec3)
	std::array<Vertex, 6> Quad = { 
		Vertex { glm::vec3{ -1.0f, -1.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f } },
		Vertex { glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
		Vertex { glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } },
		Vertex { glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } },
		Vertex { glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
		Vertex { glm::vec3{  1.0f,  1.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f } }
	}; // Armazenado na RAM

	// MVP realizada manualmente nesse momento, para aprendizado, depois usaremos shaders para sua otimização na GPU
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

	// Aplicar a MVP nos vértices do triângulo
	// 	   Ao atualizarmos o vertex shader a pipeline gráfica realizará esses cálculos de maneira otimizada, já
	// 	   dividindo pela coordenada homogênea, simplificando a programação da aplicação
	//for (Vertex& Vertex : Triangle) {
	//	glm::vec4 ProjectedVertex = MVP * glm::vec4{ Vertex.Position, 1.0f }; // Construindo um vec4 a partir de um vec3,
																			  // acrescentando estaticamente apenas a 
																			  //  última coordenada (w)
	//	ProjectedVertex /= ProjectedVertex.w; // Divide todas as coordenadas do vértice projetado pela componente w, 
											  // para que w se torne 1 e os demais valores fiquem 'normalizados'
	//	Vertex.Position = ProjectedVertex; // Altera-se por fim a referência do vértice do triângulo
	//}

	// Copiar vértices do triângulo para a memória da GPU
	GLuint VertexBuffer; // Buffer de vértices

	// Pedir para o OpenGL gerar o identificador do VertexBuffer
	glGenBuffers(1, &VertexBuffer); // Qtd e endereço de quais buffers serão inicializados

	// Ativar o VertecBuffer como sendo o buffer para onde vamos copiar os dados do triângulo
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	// Copiar os dados para a memória de vídeo
	//	Recebe um alvo, uma constante que identifica o tipo de estrutura utilizada para armazenamento do buffer
	//	um tamanho em bytes que o buffer irá administrar. Como definimos um array é possível utilizar a função sizeof()
	//	ponteiro para a estrutura de dados bufferizados. Utilizada a função data() para retornar o ponteiro
	//	recebe um caso de uso - olhar flags na documentação para maiores informações
	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad), Quad.data(), GL_STATIC_DRAW);

	// Ter em mente que o OpenGL é uma máquina de estados (quando ativarmos algo, essa coisa permanecerá ativa por padrão)
	// Definir a cor do fundo (isso é um estado, o driver armazenará essa informação:
	//	Sempre que precisarmos retomar essa informação, limparmos o framebuffer, etc, essa estado será devidamente restaurado)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // RGBAlpha

	// Entra no loop de eventos da aplicação
	while (!glfwWindowShouldClose(Window)) { // Condição de parada, verifica se a janela foi fechada

		// glClear via limpar o framebuffer. GL_COLOR_BUFFER_BIT diz para limpar (preencher) o buffer de cor -> esse 
		//	preenchimento será realizado de acordo com a última cor configurada via glClearColor
		// Quando formos desenhar geometrias 3D, voltaremos ao glClear pois teremos que limpar o buffer de 
		//	profundidade (depth buffer)
		glClear(GL_COLOR_BUFFER_BIT); // Ativa o bit do buffer que realiza a limpeza (ou mudança) do atributo cor

		// Ativa o programa de shader
		glUseProgram(ProgramId);

		// Recupera a localização calculada pelo uniforme do Vertex Shader utilizando a ModelViewProjection
		GLint ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(MVP));

		// Ativação e endereçamento da textura para os shaders
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureId);

		GLint TextureSamplerLoc = glGetUniformLocation(ProgramId, "TextureSampler");
		glUniform1i(TextureSamplerLoc, 0); // Em que 0 está identificando o ID do uniforme (GL_TEXTURE0)

		// Ativa o atributo de vértice para o array. O parâmetro 0 representa o índice do layout do shader ativo
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// Ativa o buffer VertexBuffer para ser utilizado no contexto OpenGL 
		glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

		// Informa ao OpenGL onde os vértices se encontram dentro do VertexBuffer. Como o array Triangle é contíguo
		// em memória, basta apenas dizer quantos vértices serão utilizados para desenhar o triângulo
		//  0 / 1 é o índice habilitado, deve coincidir com o especificado na chamada à função glEnableVertexAttribArray()
		//	3 é a quantidade de vértices da estrutura de dados (vec3)
		//	GL_FLOAT é o tipo de dados 
		//  GL_FALSE / GL_TRUE para informar se os atributos estão normalizados ou não 
		//	stride - tamanho do Vertex (struct) que definimos
		//	offset - para position é nulo, para color é calculado. O cast é necessário para compatibilizar o retorno
		//		     do método offsetof com o parâmetro recebido pela função glVertexAttribPointer
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));

		// Desenha em tela de acordo com os arrays atrelados ao buffer de dados (VertexBuffer), interpretando-os como 
		// coordenadas normalizadas. Especificamos o tipo pela constante do OpenGL (triângulos), dizemos qual é o ponto de 
		// origem (0) e especificamos a quantidade de vértices que a estrutura possui (3)
		glDrawArrays(GL_TRIANGLES, 0, Quad.size()); 

		// Boa prática em OpenGL: como ele se comporta como uma máquina de estados, após habilitar o buffer, 
		//	definir um contexto e desenhar coisas em tela, reverter o que foi criado para que as próximas construções
		//	em tela sejam organizadas, novos binds rastreáveis e, em suma, o comportamento sistêmico seja controlado e 
		//	previsível. 
		// Portanto, para reverter o estado criado:
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Desabilita o programa ativo
		glUseProgram(0);
		
		// Processa todos os eventos da fila de eventos do GLFW
		//	podem ser estímulos do teclado, mouse, gamepad, etc
		glfwPollEvents(); 

		// Envia o conteúdo do framebuffer da janela para ser desenhado na tela
		// A memória alocada para aplicação é trocada (swap) para a memória de vídeo que se encarregará pela
		//	renderização em tela dos pixels da janela
		//	Vale mencionar, portanto, que o tamanho da janela que estipulamos (valor das variáveis Width e Height)
		//	influencia na quantidade de memória RAM e de vídeo que nossa aplicação utilizará
		glfwSwapBuffers(Window);
	}

	glDeleteBuffers(1, &VertexBuffer); // Desaloca (1) VertexBuffer
	glfwTerminate(); // Encerra a biblioteca GLFW

	return 0;
}