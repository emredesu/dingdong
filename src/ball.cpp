#include "ball.hpp"

Ball::Ball(std::string image_path, SDL_Renderer* renderer) {
	sprite = { image_path.c_str(), renderer };
}

void Ball::move() {
	sprite.rect.x += velocity_x;
	sprite.rect.y += velocity_y;
}