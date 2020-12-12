#pragma once

#include <iostream>
#include <functional>
#include <any>
#include <string>
#include <sstream>
#include <SDL.h>
#include "sprite.hpp"
#include "text.hpp"

class Button {
	public:
		std::shared_ptr<SDL_Renderer> renderer_ptr = nullptr;

		SDL_Rect button_rect = { 0, 0, 0, 0 };
		Sprite unhovered_sprite;
		Sprite hovered_sprite;
		Text button_text;

		std::function<void()> run_on_click = nullptr;

		Button() {};
		Button(int pos_x, int pos_y, std::string text_on_button, std::string unhovered_imgpath, std::string hovered_imgpath, SDL_Renderer* renderer, std::function<void()> on_click, int text_size = 24, bool is_persistent = false);
		bool is_hovered();
};

class TextBox {
	public:
		std::string string = " ";
		std::string getting_input_of_what = "idk";
		Text text;
		Sprite sprite;

		TextBox() {};
		TextBox(int start_x, int start_y, SDL_Renderer* renderer, int text_size = 16);
		void update_text();
};