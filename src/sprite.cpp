#include "sprite.hpp"

Sprite::Sprite(std::string path, SDL_Renderer* renderer, std::optional<int> start_x, std::optional<int> start_y) {
	texture = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(renderer, path.c_str()), SDLGarbageCollector());

	if (texture.get() == nullptr) {
		std::cerr << "Failed to load texture \"" << path << "\", error: " << IMG_GetError() << "\n";
	}

	SDL_QueryTexture(texture.get(), NULL, NULL, &rect.w, &rect.h); // Query the loaded image file and put its width and height values to Sprite's rect.

	rect.x = start_x.value();
	rect.y = start_y.value();
}

void Sprite::swap_texture(std::string path, SDL_Renderer* renderer) {
	texture = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(renderer, path.c_str()), SDLGarbageCollector());

	if (texture.get() == nullptr) {
		std::cerr << "Failed to swap texture with \"" << path << "\", error: " << IMG_GetError() << "\n";
	}
	else
		SDL_QueryTexture(texture.get(), NULL, NULL, &rect.w, &rect.h);
}