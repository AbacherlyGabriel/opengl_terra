# OpenGL: Blue Marble

## Proposta
Para o desenvolvimento do exercício-programa, buscamos reproduzir uma das imagens mais distribuídas da história. Trata-se da fotografia Blue Marble, feita em 1972 durante a missão Apollo 17.
A imagem representa a vista da terra na perspectiva da citada tripulação.
Buscando alcançar tal objetivo, foram utilizadas texturas divulgadas pela NASA, que representam a superfície do planeta e algumas nuvens.

### Ambiente de Desenvolvimento
O código foi projetado para funcionar em qualquer sistema operacional, sendo construído no Windows 10 por meio da IDE Visual Studio Community 2019. O projeto foi criado com CMake, utilizando-se do OpenGL, com o auxílio de algumas de suas dependências.

#### Em resumo, o ambiente de desenvolvimento foi composto por:

- Sistema Operacional: Windows 10
- IDE: Visual Studio Community 2019
- Linguagem de Programação: C++
- Compilação/Build: CMake
- API Gráfica: OpenGL

#### As dependências utilizadas em conjunto com o OpenGL foram:

- GLM (OpenGL Mathematics): Matrizes e Vetores em C++
- GLFW (OpenGL Framework): Janela e Interação com o OpenGL
- GLEW (OpenGL Extension Wrangler): Funções do OpenGL
- STB (Single File Library): Carregamento de Imagens como Texturas 

Com relação às técnicas de Computação Gráfica, buscou-se implementar uma aplicação completa, empregando Pipelines, Vetores e Matrizes, Shaders, Câmera (Interação), além de conceitos relacionados à Geometria e Iluminação.

## Referências

#### Blue Marble:
- https://pt.wikipedia.org/wiki/The_Blue_Marble
- https://www.nasa.gov/content/blue-marble-image-of-the-earth-from-apollo-17
- https://visibleearth.nasa.gov/

#### Dependências utilizadas:

- GLM: https://glm.g-truc.net/0.9.9/index.html
- GLFW: https://www.glfw.org/
- GLEW: http://glew.sourceforge.net/
- STB: https://github.com/nothings/stb

#### Projeto:
- Vídeo: https://www.youtube.com/watch?v=-ROSM6rGLPs
