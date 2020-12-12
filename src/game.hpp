#pragma once

#include <cstdio>
#include <array>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <memory>
#include <SDL.h>
#include "connection_manager.hpp"
#include "paddle.hpp"
#include "ball.hpp"
#include "text.hpp"

enum class GameMode {
	DUMMY_VALUE,
	SINGLE_PLAYER,
	PRACTICE,
	LOCAL_MULTIPLAYER,
	ONLINE_MULTIPLAYER
};

class Game {
	public:
		ConnectionManager connection_manager;

		GameMode game_mode = GameMode::DUMMY_VALUE;

		std::shared_ptr<SDL_Renderer> renderer_ptr = nullptr;

		int screen_width = 0;
		int screen_height = 0;

		Paddle player_1;
		Paddle player_2;

		Ball ball;

		unsigned int game_start_time = 0;
		unsigned int countdown_time = 1714;

		bool sound_on = true;
		bool is_fast = false;
		bool has_played_countdown = false;
		bool has_played_slowplusfast = false;
		bool has_ended = false;
		bool has_won = false;

		std::shared_ptr<Mix_Music> slow_theme = nullptr;
		std::shared_ptr<Mix_Music> fast_theme = nullptr;
		std::shared_ptr<Mix_Music> slowplusfast_theme = nullptr;
		std::shared_ptr<Mix_Music> lose_theme = nullptr;
		std::shared_ptr<Mix_Music> win_theme = nullptr;

		std::shared_ptr<Mix_Chunk> ding_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> dong_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> bounce_ymax_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> bounce_ymin_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> countdown_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> score_sfx = nullptr;
		std::shared_ptr<Mix_Chunk> uwu_sfx = nullptr;

		int player_1_score = 0;
		int player_2_score = 0;
		int end_score = 10;

		int ball_hit_count = 0;

		Sprite game_background;
		Sprite you_won_screen;
		Sprite you_lost_screen;
		Sprite middle_line;

		Text player_1_score_text;
		Text player_2_score_text;

		Text server_awaiting_connection_text;
		Text client_connecting_text;

		bool online_has_moved = false;
		uint32_t server_send_interval = 50;
		uint32_t server_send_timer = 0;

		Game() {};
		Game(int screen_width_param, int screen_height_param, GameMode game_mode, SDL_Renderer* renderer);
		void init_game(GameMode init_mode, int screen_width, int screen_height, bool is_sound_on, int end_score_param);
		void process_input(const Uint8* keyboard_state);
		std::vector<std::string> split_string(const std::string& text, char seperator);
		int generate_random_number(int range_begin, int range_end, int multiples = 1);
		void push_event(std::string data1, std::string data2 = "none");
		void process_received_data(std::string received_data);
		void reset_paddle_positions();
		void reset_ball_position();
		void update_scores(SDL_Renderer* renderer);
		void center_scores();
		void increase_ball_speed();
		void bounce_ball();
		void start_new_round(std::string winner);
		void ai();
		void tick();
		void reset_game();
		void play_if_sound_on(Mix_Chunk* chunk, int loops = 0);
		std::string get_nethelpmsgstr(int errcode);
};