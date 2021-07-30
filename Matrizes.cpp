#include <iostream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

void PrintMatrix(const glm::mat4& M)
{	
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			std::cout 
				<< std::setw(10) 
				<< std::setprecision(4) 
				<< std::fixed
				<< M[i][j] << " ";
		}

		std::cout << std::endl;
	}
}

void TranslationMatrix()
{
	std::cout << std::endl;
	std::cout << "==================" << std::endl;
	std::cout << "Translation Matrix" << std::endl;
	std::cout << "==================" << std::endl;
	

	glm::vec4 Position{ 10, 10, 10, 1 };
	glm::vec4 Direction{ 10, 10, 10, 0 };
	glm::vec3 Translate{ 10, 10, 10 };
	glm::mat4 Translation = glm::translate(glm::identity<glm::mat4>(), Translate);

	PrintMatrix(Translation);

	std::cout << std::endl;
	Position = Translation * Position;
	Direction = Translation * Direction;
	std::cout << glm::to_string(Position) << std::endl;
	std::cout << glm::to_string(Direction) << std::endl;
}

void RotationMatrix()
{
	std::cout << std::endl;
	std::cout << "==================" << std::endl;
	std::cout << "Rotation Matrix" << std::endl;
	std::cout << "==================" << std::endl;

	glm::vec4 Position{ 100, 0, 0, 1 };
	glm::vec4 Direction{ 100, 0, 0, 0 };
	glm::vec3 Axis{ 0, 0, 1 };
	glm::mat4 Rotation = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), Axis);

	PrintMatrix(Rotation);

	std::cout << std::endl;
	Position = Rotation * Position;
	Direction = Rotation * Direction;
	std::cout << glm::to_string(Position) << std::endl;
	std::cout << glm::to_string(Direction) << std::endl;
}

void ScaleMatrix()
{
	std::cout << std::endl;
	std::cout << "==================" << std::endl;
	std::cout << "Scale Matrix" << std::endl;
	std::cout << "==================" << std::endl;

	glm::vec4 Position{ 100, 100, 0, 1 };
	glm::vec4 Direction{ 100, 100, 0, 0 };
	glm::vec3 ScaleAmount{ 2, 2, 2 };
	glm::mat4 Scale = glm::scale(glm::identity<glm::mat4>(), ScaleAmount);

	PrintMatrix(Scale);

	std::cout << std::endl;
	Position = Scale * Position;
	Direction = Scale * Direction;
	std::cout << glm::to_string(Position) << std::endl;
	std::cout << glm::to_string(Direction) << std::endl;
}

void ComposedMatrix()
{
	std::cout << std::endl;
	std::cout << "==================" << std::endl;
	std::cout << "Composed Matrix" << std::endl;
	std::cout << "==================" << std::endl;

	glm::vec4 Position{ 1, 1, 0, 1 };
	glm::vec4 Direction{ 1, 1, 0, 0 };

	glm::vec3 Translate{ 0, 10, 0 };
	glm::mat4 Translation = glm::translate(glm::identity<glm::mat4>(), Translate);

	glm::vec3 Axis{ 0, 0, 1 };
	glm::mat4 Rotation = glm::rotate(glm::identity<glm::mat4>(), glm::radians(45.0f), Axis);

	glm::vec3 ScaleAmount{ 2, 2, 0 };
	glm::mat4 Scale = glm::scale(glm::identity<glm::mat4>(), ScaleAmount);

	std::cout << "Translation: " << std::endl;
	PrintMatrix(Translation);
	std::cout << std::endl;

	std::cout << "Rotation: " << std::endl;
	PrintMatrix(Rotation);
	std::cout << std::endl;

	std::cout << "Scale: " << std::endl;
	PrintMatrix(Scale);
	std::cout << std::endl;

	glm::mat4 Transform = Translation * Rotation * Scale;
	
	std::cout << "Transform: " << std::endl;
	PrintMatrix(Transform);
	std::cout << std::endl;

	Position = Transform * Position;
	Direction = Transform * Direction;

	std::cout << std::endl;
	std::cout << glm::to_string(Position) << std::endl;
	std::cout << glm::to_string(Direction) << std::endl;	
}

void ModelViewProjection()
{
	std::cout << std::endl;
	std::cout << "==========================" << std::endl;
	std::cout << "Model, View and Projection" << std::endl;
	std::cout << "==========================" << std::endl;

	// Model é a matriz formada pelas transformações de Escala, Rotação e Translação!
	glm::mat4 Model = glm::identity<glm::mat4>();

	// View
	glm::vec3 Eye{ 0.0f, 0.0f, 10.0f };
	glm::vec3 Center{ 0.0f, 0.0f, 0.0 };
	glm::vec3 Up{ 0.0f, 1.0f, 0.0f };
	glm::mat4 View = glm::lookAt(Eye, Center, Up);

	std::cout << "View: " << std::endl;
	PrintMatrix(View);

	constexpr float FoV = glm::radians(45.0f);
	const float AspectRatio = 800.0f / 600.0f;
	const float Near = 0.001f;
	const float Far = 1000.0f;
	glm::mat4 Projection = glm::perspective(FoV, AspectRatio, Near, Far);

	std::cout << "View: " << std::endl;
	PrintMatrix(Projection);

	glm::mat4 ModelViewProjection = Projection * View * Model;

	std::cout << "ModelViewProjection: " << std::endl;
	PrintMatrix(ModelViewProjection);
}

int main()
{		
	TranslationMatrix();
	RotationMatrix();
	ScaleMatrix();
	ComposedMatrix();
	ModelViewProjection();

	return 0;
}