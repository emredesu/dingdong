#pragma once

#include <vector>
#include <string>
#include "uielements.hpp"
#include "sprite.hpp"

class MainMenu {
	public:
		int screen_width = 0;
		int screen_height = 0;

		std::shared_ptr<SDL_Renderer> renderer_ptr = nullptr;
		
		std::shared_ptr<Mix_Music> main_menu_music = nullptr;

		std::shared_ptr<Mix_Chunk> button_click_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> type_sfx = nullptr;

		bool had_error = false;

		Sprite background;
		Sprite game_sign;

		unsigned int current_frame_on_main_menu = 0;

		Sprite city_back;
		SDL_Rect city_back_current_rect = { 0, 0, 1000, 800 };
		unsigned int frames_to_city_back_swap = 2;

		Sprite city_front;
		SDL_Rect city_front_current_rect = { 0, 0, 1000, 800 };

		Button start_button;
		Button credits_button;
		Button quit_button;

		Button github_button;
		Button sound_toggle_button;

		Button single_player_button;
		Button practice_button;
		Button local_multiplayer_button;
		Button online_multiplayer_button;
		Button server_button;
		Button client_button;
		Button credits_inside_button;
		Button back_to_main_menu_button;

		TextBox textbox;
		Button textbox_ok_button;

		Text give_input_text;
		Text client_timeout_text;
		Text server_timeout_text;
		Text disconnection_text;
		Text error_text;

		std::vector<Button> buttons;
		std::vector<Button> persistent_buttons;

		inline static std::string game_mode_to_pass;

		static void list_game_modes();
		static void show_credits();
		static void send_exit();
		static void start_single_player();
		static void start_practice();
		static void start_local_multiplayer();
		static void start_server();
		static void start_client();
		static void textbox_ok_button_func();
		static void show_multiplayer_options();
		static void get_end_score();
		static void get_ipaddr();
		static void toggle_sound();
		static void open_github_link();
		static void init_main_menu();
		static void open_ulasyt();
		static void push_event(std::string data1, std::string data2 = "none");
		MainMenu() {};
		MainMenu(int screen_width_param, int screen_height_param, SDL_Renderer* renderer);
};