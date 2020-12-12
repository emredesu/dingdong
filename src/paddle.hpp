#pragma once

#include <SDL.h>
#include <string>
#include "sprite.hpp"

enum class MoveDirection {
	UP,
	DOWN
};

class Paddle {
	public:
		Sprite sprite;

		int screen_height = 0;

		const static int paddle_speed = 5;
		
		Paddle() {};
		Paddle(int screen_height_param, std::string image_path, SDL_Renderer* renderer);
		void move(MoveDirection direction, int pixels = paddle_speed);
};