#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <glm/glm.hpp>
#include <string>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

struct RigidBodyComponent 
{
    glm::vec2 vel;

    RigidBodyComponent(glm::vec2 velocity = glm::vec2(0.0, 0.0)) : vel{ velocity } {}

};

struct SpriteComponent 
{
	std::string asset_id;
	int width;
	int height;
	SDL_Rect src_rect;

	SpriteComponent(std::string asset_id = "", int width = 0, int height = 0, int src_rectX = 0, int src_rectY = 0) : 
		asset_id{ asset_id }, 
		width{ width }, 
		height{ height }, 
		src_rect{src_rectX, src_rectY, width, height} {}
};

struct TransformComponent
{
	glm::vec2 pos;
	glm::vec2 scale;
	double rot;

	TransformComponent(glm::vec2 position = glm::vec2(0, 0), glm::vec2 scale = glm::vec2(1, 1), double rotation = 0.0) 
	{
		this->pos = position;
		this->scale = scale;
		this->rot = rotation;
	}
};
#endif
