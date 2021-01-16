#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <Windows.h>
#include <SDL.h>
#include <discord.h>
#include "game.hpp"
#include "mainmenu.hpp"
#include "sdl_garbage_collector.hpp"

enum class AppState {
	DUMMY_VALUE,
	MAIN_MENU,
	IN_GAME
};

class DiscordManager {
	private:
		int64_t app_client_id = 799984524766478336;
	public:
		IDiscordCore* core = nullptr;
		IDiscordActivityManager* activity_manager = nullptr;
		IDiscordCoreEvents events = nullptr;

		DiscordManager();
		void update_rpc(AppState app_state, GameMode game_mode);
};

class App {
	protected:
		const int FPS_LIMIT = 60;
	private:
		std::unique_ptr<SDL_Window, SDLGarbageCollector> window = nullptr;
		std::shared_ptr<SDL_Renderer> renderer = nullptr;
	public:
		std::string window_title = "dingdong";
		int window_width = 1000;
		int window_height = 800;

		bool app_active = true;
		bool sound_on = true;
		int end_score_to_pass = UINT_MAX;

		Game game;
		MainMenu main_menu;

		AppState app_state = AppState::MAIN_MENU;

		DiscordManager discord_manager;

		App();
		bool initialize_sdl_subsystems();
		void handle_events();
		void play_if_sound_on(Mix_Chunk* chunk, int loops = 0);
		void process_input(SDL_Keycode pressed_key);
		void update();
		void render();
		void main_loop();
		void quit_all_subsystems();
};