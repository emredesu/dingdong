#define SDL_MAIN_HANDLED

#include "app.hpp"
#include "ball.hpp"
#include "connection_manager.hpp"
#include "paddle.hpp"
#include "sdl_garbage_collector.hpp"
#include "sprite.hpp"
#include "text.hpp"

int main(int argc, char* argv[]) {
	ShowWindow(GetConsoleWindow(), SW_HIDE); // Hides the console.

	App dingdong;
	dingdong.main_loop();

	return 0;
}