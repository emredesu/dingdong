#include "text.hpp"

Text::Text(std::string text, SDL_Renderer* renderer_param, int text_size_param, SDL_Colour text_colour_param, int start_x, int start_y) {
	std::unique_ptr<TTF_Font, SDLGarbageCollector> font = std::unique_ptr<TTF_Font, SDLGarbageCollector>(TTF_OpenFont(font_file.c_str(), text_size_param));
	if (font == NULL)
		std::cerr << "Failed to open font file \"" << font_file << "\", error: " << TTF_GetError() << "\n";

	TTF_SizeText(font.get(), text.c_str(), &rect.w, &rect.h); // Set text rect width and height in pixels based on font file and passed text.

	std::unique_ptr<SDL_Surface, SDLGarbageCollector> text_surface = std::unique_ptr<SDL_Surface, SDLGarbageCollector>(TTF_RenderText_Solid(font.get(), text.c_str(), text_colour_param)); // Render text on a temporary surface.

	texture = std::unique_ptr<SDL_Texture, SDLGarbageCollector>(SDL_CreateTextureFromSurface(renderer_param, text_surface.get())); // Turn the temporary surface into a texture.
	if (texture == nullptr)
		std::cerr << "Failed to create texture for the text, error: " << SDL_GetError() << "\n";

	rect.x = start_x;
	rect.y = start_y;

	// Save the attributes of the current text for reusing them when we want to swap the text.
	renderer = std::shared_ptr<SDL_Renderer>(renderer_param, SDL_DestroyRenderer);
	text_size = text_size_param;
	text_colour = text_colour_param;
}

// Basically does the same thing as the constructor except that it reuses the values (size, colour etc.) of the existing text without altering its position.
void Text::swap_text(std::string text) {
	std::unique_ptr<TTF_Font, SDLGarbageCollector> font = std::unique_ptr<TTF_Font, SDLGarbageCollector>(TTF_OpenFont(font_file.c_str(), text_size));
	if (font == NULL)
		std::cerr << "Failed to open font file \"" << font_file << "\" while trying to swap text, error: " << TTF_GetError() << "\n";

	TTF_SizeText(font.get(), text.c_str(), &rect.w, &rect.h);

	std::unique_ptr<SDL_Surface, SDLGarbageCollector> text_surface = std::unique_ptr<SDL_Surface, SDLGarbageCollector>(TTF_RenderText_Solid(font.get(), text.c_str(), text_colour));

	texture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), text_surface.get()), SDLGarbageCollector()); // Swap the texture with the newly created texture for the new text.
	if (texture == nullptr)
		std::cerr << "Failed to create new texture for text swap, error: " << SDL_GetError() << "\n";
}