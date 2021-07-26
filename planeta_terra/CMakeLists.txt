# Definindo Versao
cmake_minimum_required(VERSION 3.20)

# Criando Projeto
project(PlanetaTerra)

# Definindo Executaveis
add_executable(PlanetaTerra main.cpp)
#add_executable(Teste main.cpp)

# Adicionando Dependencias
target_include_directories(PlanetaTerra PRIVATE dependencias/glm)
target_include_directories(PlanetaTerra PRIVATE dependencias/stb)
target_include_directories(PlanetaTerra PRIVATE dependencias/glfw/include)
target_include_directories(PlanetaTerra PRIVATE dependencias/glew/include)

# Configurando Diretorios
target_link_directories(PlanetaTerra PRIVATE dependencias/glfw/lib-vc2019)
target_link_directories(PlanetaTerra PRIVATE dependencias/glew/lib/Release/x64)

# Configurando Bibliotecas
target_link_libraries(PlanetaTerra PRIVATE glfw3.lib)
target_link_libraries(PlanetaTerra PRIVATE glew32.lib)

# Criando copia do arquivo grew32.dll para o diretorio de output
add_custom_command(
	TARGET PlanetaTerra POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/dependencias/glew/bin/Release/x64/glew32.dll" "${CMAKE_BINARY_DIR}/glew32.dll"
)