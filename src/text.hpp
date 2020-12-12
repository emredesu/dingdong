#pragma once

#include <memory>
#include <string>
#include <optional>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "sdl_garbage_collector.hpp"

class Text { // todo - fix the usage of \n
	public:
		SDL_Rect rect = { 0, 0, 0, 0 };
		std::shared_ptr<SDL_Texture> texture = nullptr;
		std::string font_file = "fonts/dogicapixelbold.ttf";

		// Save the variables of the current text so that we can use their values when we want to swap the text.
		std::shared_ptr<SDL_Renderer> renderer = nullptr;
		int text_size = 0;
		SDL_Colour text_colour = { 6, 6, 6 };

		Text() {};
		Text(std::string text, SDL_Renderer* renderer_param, int text_size_param = 24, SDL_Colour text_colour_param = {0, 0, 0}, int start_x = 0, int start_y = 0);
		void swap_text(std::string text);
};