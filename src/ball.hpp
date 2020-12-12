#pragma once

#include <SDL.h>
#include <string>
#include "sprite.hpp"

class Ball {
	public:
		Sprite sprite;

		int velocity_x = 5;
		int velocity_y = 5;

		Ball() {};
		Ball(std::string image_path, SDL_Renderer* renderer);
		void move();
};