#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <optional>
#include <SDL.h>
#include <SDL_image.h>
#include "sdl_garbage_collector.hpp"

class Sprite {
	public:
		SDL_Rect rect = { 0, 0, 0, 0 };
		std::shared_ptr<SDL_Texture> texture = nullptr;

		Sprite() {};
		Sprite(std::string path, SDL_Renderer* renderer, std::optional<int> start_x = 0, std::optional<int> start_y = 0);
		void swap_texture(std::string path, SDL_Renderer* renderer);
};