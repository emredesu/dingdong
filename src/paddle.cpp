#include "paddle.hpp"

Paddle::Paddle(int screen_height_param, std::string image_path, SDL_Renderer* renderer) {
	sprite = { image_path.c_str(), renderer};

	screen_height = screen_height_param;
}

void Paddle::move(MoveDirection direction, int pixels) {
	switch (direction) {
		case MoveDirection::UP:
			if (sprite.rect.y > 0)
				sprite.rect.y -= pixels;
			break;
		case MoveDirection::DOWN:
			if (sprite.rect.y + sprite.rect.h < screen_height) // Adding paddle height because SDL_Rect represents the upper left corner of the paddle.
				sprite.rect.y += pixels;
			break;
	}
}