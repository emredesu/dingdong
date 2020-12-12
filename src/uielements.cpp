#include "uielements.hpp"

Button::Button(int pos_x, int pos_y, std::string text_on_button, std::string unhovered_imgpath, std::string hovered_imgpath, SDL_Renderer* renderer, std::function<void()> on_click, int text_size, bool is_persistent) {
	renderer_ptr = std::shared_ptr<SDL_Renderer>(renderer, SDL_DestroyRenderer);

	run_on_click = on_click;

	unhovered_sprite = { unhovered_imgpath, renderer };

	if (!is_persistent) {
		hovered_sprite = { hovered_imgpath, renderer };
		button_text = { text_on_button, renderer, text_size, {0, 0, 0} };
	}

	button_rect.w = hovered_sprite.rect.w = unhovered_sprite.rect.w;
	button_rect.h = hovered_sprite.rect.h = unhovered_sprite.rect.h;
	button_rect.x = hovered_sprite.rect.x = unhovered_sprite.rect.x = pos_x;
	button_rect.y = hovered_sprite.rect.y = unhovered_sprite.rect.y = pos_y;

	// Center the button text at the middle of the button.
	button_text.rect.x = (button_rect.x + button_rect.w / 2) - (button_text.rect.w / 2);
	button_text.rect.y = (button_rect.y + button_rect.h / 2) - (button_text.rect.h / 2);
}

bool Button::is_hovered() {
	SDL_Rect mouse_rect = { 0, 0, 1, 1 };
	SDL_GetMouseState(&mouse_rect.x, &mouse_rect.y);

	return SDL_HasIntersection(&mouse_rect, &button_rect);
}

TextBox::TextBox(int start_x, int start_y, SDL_Renderer* renderer, int text_size) {
	sprite = { "sprites/text_box.png", renderer};
	text = {string, renderer, text_size};

	sprite.rect.x = start_x;
	sprite.rect.y = start_y;

	text.rect.x = sprite.rect.x + 10;
	text.rect.y = sprite.rect.y + 30;
}

void TextBox::update_text() {
	text.swap_text(string);
}