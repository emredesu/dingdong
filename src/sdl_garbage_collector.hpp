#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

// std smart pointers call this struct as a function with the pointer to an SDL object, and this class takes care of the deletion of the corresponding SDL object type.
struct SDLGarbageCollector {
	void operator () (SDL_Window* window) const { 
		SDL_DestroyWindow(window); 
	};

	void operator () (SDL_Renderer* renderer) const {
		SDL_DestroyRenderer(renderer);
	}

	void operator () (SDL_Texture* texture) const {
		SDL_DestroyTexture(texture);
	}

	void operator () (SDL_Surface* surface) const {
		SDL_FreeSurface(surface);
	}

	void operator () (Mix_Music* music) const {
		Mix_FreeMusic(music);
	}

	void operator () (Mix_Chunk* chunk) const {
		Mix_FreeChunk(chunk);
	}

	void operator () (TTF_Font* font) const {
		TTF_CloseFont(font);
	}
};