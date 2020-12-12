#include "game.hpp"

Game::Game(int screen_width_param, int screen_height_param, GameMode game_mode_param, SDL_Renderer* renderer) {
	renderer_ptr = std::shared_ptr<SDL_Renderer>(renderer, SDL_DestroyRenderer);
	
	game_mode = game_mode_param;

	screen_width = screen_width_param;
	screen_height = screen_height_param;

	player_1 = { screen_height, "sprites/paddle_1.png", renderer };
	player_2 = { screen_height, "sprites/paddle_2.png", renderer };
	ball = { "sprites/ball.png", renderer };

	game_background = { "sprites/game_background.png", renderer };
	you_won_screen = { "sprites/you_won_screen.png", renderer };
	you_lost_screen = { "sprites/you_lost_screen.png", renderer };
	middle_line = { "sprites/middle_line.png", renderer };

	player_1_score_text = { std::to_string(player_1_score), renderer, 30, {255, 255, 255} };
	player_2_score_text = { std::to_string(player_2_score), renderer, 30, {255, 255, 255} };

	server_awaiting_connection_text = { "Awaiting connection, the game may become unresponsive during this time...", renderer_ptr.get(), 14, {255, 0, 0} };
	client_connecting_text = { "Attempting to connect, the game may become unresponsive during this time...", renderer_ptr.get(), 14, {255, 0, 0} };

	// Center the middle line on the middle of the screen.
	middle_line.rect.x = (screen_width / 2) - (middle_line.rect.w / 2);
	middle_line.rect.y = 0;

	// Center the texts.
	server_awaiting_connection_text.rect.x = (screen_width_param / 2) - (server_awaiting_connection_text.rect.w / 2);
	client_connecting_text.rect.x = (screen_width_param / 2) - (client_connecting_text.rect.w / 2);
	server_awaiting_connection_text.rect.y = client_connecting_text.rect.y = screen_height_param / 2;

	// Prepare sfx and music files.
	slow_theme = std::shared_ptr<Mix_Music>(Mix_LoadMUS("music/play_chill_bro.wav"), SDLGarbageCollector());
	fast_theme = std::shared_ptr<Mix_Music>(Mix_LoadMUS("music/faster_you_mortal.wav"), SDLGarbageCollector());
	slowplusfast_theme = std::shared_ptr<Mix_Music>(Mix_LoadMUS("music/slowplusfast.wav"), SDLGarbageCollector());
	lose_theme = std::shared_ptr<Mix_Music>(Mix_LoadMUS("music/Better_luck_next_time.wav"), SDLGarbageCollector());
	win_theme = std::shared_ptr<Mix_Music>(Mix_LoadMUS("music/you_won.wav"), SDLGarbageCollector());

	ding_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/Ding.wav"), SDLGarbageCollector());
	dong_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/Dong.wav"), SDLGarbageCollector());
	bounce_ymax_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/bounce_1.wav"), SDLGarbageCollector());
	bounce_ymin_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/bounce_2.wav"), SDLGarbageCollector());
	countdown_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/ready.wav"), SDLGarbageCollector());
	score_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/score.wav"), SDLGarbageCollector());
	uwu_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/uwu.wav"), SDLGarbageCollector());
}

void Game::init_game(GameMode init_mode, int screen_width, int screen_height, bool is_sound_on, int end_score_param) {
	sound_on = is_sound_on;
	end_score = end_score_param;

	game_mode = init_mode;

	// Reset game in case we have left over stuff from a possible previous game.
	reset_game();

	// Center paddles on opposite sides of the screen.
	player_1.sprite.rect.x = 5;
	player_1.sprite.rect.y = (screen_height / 2) - (player_1.sprite.rect.h / 2);

	player_2.sprite.rect.x = (screen_width - player_2.sprite.rect.w) - 5;
	player_2.sprite.rect.y = (screen_height / 2) - (player_2.sprite.rect.h / 2);

	// Center ball on the middle of the screen.
	ball.sprite.rect.x = (screen_width / 2) - (ball.sprite.rect.w / 2);
	ball.sprite.rect.y = (screen_height / 2) - (ball.sprite.rect.h / 2);

	center_scores();

	// Make the ball go towards a random direction at the start of the match.
	std::vector<int> velocities = { -1, 1 };
	ball.velocity_x *= velocities[generate_random_number(0, 1)];
	ball.velocity_y *= velocities[generate_random_number(0, 1)];

	if (Mix_PlayingMusic())
		Mix_HaltMusic();

	if (game_mode == GameMode::ONLINE_MULTIPLAYER) {
		int connection_result = connection_manager.init(connection_manager.type);

		if (connection_result != 0) {
			switch (connection_result) {
				case 9999: // Server timeout.
					push_event("main_menu_err", "No one connected to the server before timeout.");
					break;
				case 8888: // Client timeout.
					push_event("main_menu_err", "Could not connect to the server.");
					break;
				default:
					push_event("main_menu_err", get_nethelpmsgstr(connection_result));
			}
		}
	}

	game_start_time = SDL_GetTicks();
}

void Game::play_if_sound_on(Mix_Chunk* chunk, int loops) {
	if (sound_on)
		Mix_PlayChannel(-1, chunk, loops);
}

std::string Game::get_nethelpmsgstr(int errcode) {
	std::string command = "net helpmsg " + std::to_string(errcode);

	std::array<char, 128> buffer{};
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
	if (!pipe)
		return "very bad non good";

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

void Game::process_input(const Uint8* keyboard_state) {
	if (game_mode == GameMode::SINGLE_PLAYER || game_mode == GameMode::PRACTICE) {
		if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP])
			player_1.move(MoveDirection::UP);
		if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN])
			player_1.move(MoveDirection::DOWN);
	}
	else if (game_mode == GameMode::LOCAL_MULTIPLAYER) {
		if (keyboard_state[SDL_SCANCODE_W])
			player_1.move(MoveDirection::UP);
		if (keyboard_state[SDL_SCANCODE_S])
			player_1.move(MoveDirection::DOWN);
		if (keyboard_state[SDL_SCANCODE_UP])
			player_2.move(MoveDirection::UP);
		if (keyboard_state[SDL_SCANCODE_DOWN])
			player_2.move(MoveDirection::DOWN);
	}
	else if (game_mode == GameMode::ONLINE_MULTIPLAYER) {
		if (connection_manager.type == "client") {
			if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {
				online_has_moved = true;
				player_2.move(MoveDirection::UP);
			}
			if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN]) {
				online_has_moved = true;
				player_2.move(MoveDirection::DOWN);
			}
		}
		else if (connection_manager.type == "server") {
			if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {
				online_has_moved = true;
				player_1.move(MoveDirection::UP);
			}
			if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN]) {
				online_has_moved = true;
				player_1.move(MoveDirection::DOWN);
			}
		}
	}

	if (keyboard_state[SDL_SCANCODE_ESCAPE]) { // Return to main menu by pressing ESC.
		reset_game();

		if (game_mode == GameMode::ONLINE_MULTIPLAYER)
			connection_manager.reset();

		push_event("main_menu", "start");
	}
}

// Pushes an SDL_USEREVENT with two datas as string pointers.
void Game::push_event(std::string data1, std::string data2) {
	// Dynamically allocating these as they'll need to outlive the scope they're in. They'll be handled properly when the event is processed.
	void* data1_ptr = static_cast<void*>(new std::string(data1));
	void* data2_ptr = static_cast<void*>(new std::string(data2));

	SDL_Event event;

	SDL_zero(event);
	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = data1_ptr;
	event.user.data2 = data2_ptr;
	SDL_PushEvent(&event);
}

void Game::center_scores() {
	// Center the scores on the left and right sides of the screen.
	player_1_score_text.rect.x = ((screen_width / 2) / 2) - (player_1_score_text.rect.w / 2);
	player_2_score_text.rect.x = (screen_width / 2) + (screen_width / 4) - (player_2_score_text.rect.w / 2); // 3/4th of screen width.

	player_1_score_text.rect.y = player_2_score_text.rect.y = (screen_height / 2) - (screen_height / 4); // 3/4th of screen height.
}

void Game::update_scores(SDL_Renderer* renderer) {
	player_1_score_text.swap_text(std::to_string(player_1_score));
	player_2_score_text.swap_text(std::to_string(player_2_score));
}

void Game::reset_game() {
	is_fast = false;
	has_played_slowplusfast = false;
	has_played_countdown = false;
	has_ended = false;
	has_won = false;

	player_1.sprite.rect.y = (screen_height / 2) - (player_1.sprite.rect.h / 2);
	player_2.sprite.rect.y = (screen_height / 2) - (player_2.sprite.rect.h / 2);

	player_1_score = player_2_score = 0;
	update_scores(renderer_ptr.get());

	ball.velocity_x = ball.velocity_y = 5;
	ball_hit_count = 0;

	if (game_mode == GameMode::ONLINE_MULTIPLAYER)
		connection_manager.reset();
}

void Game::reset_paddle_positions() {
	player_1.sprite.rect.y = (screen_height / 2) - (player_1.sprite.rect.h / 2);
	player_2.sprite.rect.y = (screen_height / 2) - (player_2.sprite.rect.h / 2);
}

void Game::reset_ball_position() {
	ball.sprite.rect.x = (screen_width / 2) - (ball.sprite.rect.w / 2);
	ball.sprite.rect.y = (screen_height / 2) - (ball.sprite.rect.h / 2);
}

int Game::generate_random_number(int range_begin, int range_end, int multiples) {
	std::random_device randomdevice;
	std::mt19937 gen(rand());
	std::uniform_int_distribution<> dis(range_begin, range_end);
	return dis(gen) * multiples;
}

void Game::start_new_round(std::string winner) {
	// Check if game ended.
	if (player_1_score == end_score) {
		has_ended = true;

		if (game_mode != GameMode::ONLINE_MULTIPLAYER || (game_mode == GameMode::ONLINE_MULTIPLAYER && connection_manager.type == "server")) {
			has_won = true;

			if (Mix_PlayingMusic()) {
				Mix_HaltMusic();
				Mix_PlayMusic(win_theme.get(), -1);
			}
		}
		else {
			has_won = false;

			if (Mix_PlayingMusic()) {
				Mix_HaltMusic();
				Mix_PlayMusic(lose_theme.get(), -1);
			}
		}

		return;
	}
	else if (player_2_score == end_score) {
		has_ended = true;

		if (game_mode == GameMode::ONLINE_MULTIPLAYER && connection_manager.type == "client") {
			has_won = true;

			if (Mix_PlayingMusic()) {
				Mix_HaltMusic();
				Mix_PlayMusic(win_theme.get(), -1);
			}
		}
		else {
			has_won = false;

			if (Mix_PlayingMusic()) {
				Mix_HaltMusic();
				Mix_PlayMusic(lose_theme.get(), -1);
			}
		}
		return;
	}

	is_fast = false;

	// easter egg!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! funny!!!!!!!!!!!!!!!!!
	if (player_1_score == 3 && player_2_score == 1)
		play_if_sound_on(uwu_sfx.get());

	ball.velocity_x = ball.velocity_y = 5;

	std::vector<int> velocities = { -1, 1 };
	ball.velocity_y *= velocities[generate_random_number(0, 1)]; // Make the ball go towards above or below in a random fashion, but towards the winner.

	if (winner == "player1")
		ball.velocity_x *= -1; // Make the ball go towards the winner at the start of the round.

	ball_hit_count = 0;

	reset_paddle_positions();
	reset_ball_position();
	update_scores(renderer_ptr.get());
	center_scores(); // Re-center scores when we swap their texts in case they got larger.

	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
		Mix_PlayMusic(slowplusfast_theme.get(), 0);
	}
}

void Game::increase_ball_speed() {
	if (ball.velocity_x < 0)
		ball.velocity_x -= 1;
	else
		ball.velocity_x += 1;

	if (ball.velocity_y < 0)
		ball.velocity_y -= 1;
	else
		ball.velocity_y += 1;
}

// todo - fix the bug that happens when the ball is hit from the side
void Game::bounce_ball() {
	ball.velocity_x *= -1;

	ball_hit_count++;
	if (ball_hit_count != 0 && ball_hit_count % 3 == 0)
		increase_ball_speed();
}

void Game::ai() {
	if (ball.sprite.rect.y + ball.sprite.rect.h / 2 > player_2.sprite.rect.y + ball.sprite.rect.h / 2) { // Move down if the middle of the ball is below middle of the paddle.
		player_2.move(MoveDirection::DOWN);
	}
	else if (ball.sprite.rect.y + ball.sprite.rect.h / 2 < player_2.sprite.rect.y + player_2.sprite.rect.h / 2) { // Move up if the middle of the ball is above middle of the paddle.
		player_2.move(MoveDirection::UP);
	}
}

void Game::process_received_data(std::string received_data) {
	if (received_data == "CONNRESET") {
		std::cerr << "Lost connection." << "\n";
		connection_manager.is_connected = false;
		push_event("main_menu_err", get_nethelpmsgstr(WSAECONNRESET));
	}
	else {
		std::vector<std::string> seperated_args = split_string(received_data, 32); // char(32) would be " " (whitespace).

		for (auto&& arg : seperated_args) {
			std::vector<std::string> command_and_value = split_string(arg, 58); // char(58) would be ":".

			if (command_and_value.size() == 2) {
				std::string command = command_and_value[0];
				int value = std::stoi(command_and_value[1]);

				if (command == "bx")
					ball.sprite.rect.x = value;
				else if (command == "by")
					ball.sprite.rect.y = value;
				else if (command == "p1")
					player_1.sprite.rect.y = value;
				else if (command == "p2")
					player_2.sprite.rect.y = value;
				else if (command == "p1s") {
					if (player_1_score != value) {
						player_1_score = value;
						update_scores(renderer_ptr.get());
						center_scores();
					}
				}
				else if (command == "p2s") {
					if (player_2_score != value) {
						player_2_score = value;
						update_scores(renderer_ptr.get());
						center_scores();
					}
				}
				else if (command == "bvx")
					ball.velocity_x = value;
				else if (command == "bvy")
					ball.velocity_y = value;
				else if (command == "endsc")
					if (end_score != value)
						end_score = value;
			}
		}
	}
}

std::vector<std::string> Game::split_string(const std::string& text, char seperator) {
	std::vector<std::string> tokens;

	std::size_t start = 0;
	std::size_t end = 0;

	while ((end = text.find(seperator, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

void Game::tick() {
	const Uint8* keyboard_state = SDL_GetKeyboardState(NULL); // Can't use smart pointers here - "The pointer returned is a pointer to an internal SDL array. It will be valid for the whole lifetime of the application and should not be freed by the caller." (from the SDL documentation for SDL_GetKeyboardState)
	process_input(keyboard_state); // This is where we need to handle more precise keyboard input, that's why we're using SDL_GetKeyboardState() when we're in game for keyboard input handling.

	if ((game_mode == GameMode::ONLINE_MULTIPLAYER && !connection_manager.is_connected) || has_ended)
		return;

	// Does the countdown before the game starts.
	if (game_start_time + countdown_time > SDL_GetTicks()) {
		if (!has_played_countdown) {
			play_if_sound_on(countdown_sfx.get());
			has_played_countdown = true;
		}
		return;
	}
	
	// Play the slowplusfast music once.
	if (!has_played_slowplusfast && sound_on) {
		Mix_PlayMusic(slowplusfast_theme.get(), 0);
		has_played_slowplusfast = true;
	}

	// Switch to looping the fast music once the slow + fast music is over.
	if (!Mix_PlayingMusic() && sound_on) {
		Mix_PlayMusic(fast_theme.get(), -1);
		is_fast = true;
	}

	// See if there's any data from the server/client and apply it to the current state if there is.
	if (game_mode == GameMode::ONLINE_MULTIPLAYER && connection_manager.is_connected)
		process_received_data(connection_manager.receive_data());

	ball.move();

	if (game_mode == GameMode::SINGLE_PLAYER)
		ai();

	if (game_mode == GameMode::PRACTICE)
		player_2.sprite.rect.y = ball.sprite.rect.y;

	// Bounce the ball off of top and bottom sides of the screen.
	if (ball.sprite.rect.y <= 0) { 
		ball.velocity_y *= -1;
		play_if_sound_on(bounce_ymin_sfx.get());
	}

	if (ball.sprite.rect.y + ball.sprite.rect.h >= screen_height) {
		ball.velocity_y *= -1;
		play_if_sound_on(bounce_ymax_sfx.get());
	}

	// Bounce the ball off of player paddles.
	if (SDL_HasIntersection(&ball.sprite.rect, &player_1.sprite.rect)) {
		bounce_ball();
		if (!is_fast)
			play_if_sound_on(ding_sfx.get());
	}

	if (SDL_HasIntersection(&ball.sprite.rect, &player_2.sprite.rect)) {
		bounce_ball();
		if (!is_fast)
			play_if_sound_on(dong_sfx.get());
	}

	// Check if player 1 scored.
	if (ball.sprite.rect.x + ball.sprite.rect.w >= screen_width) {
		player_1_score++;
		play_if_sound_on(score_sfx.get());
		start_new_round("player1");
	}

	// Check if player 2 scored.
	if (ball.sprite.rect.x <= 0) {
		player_2_score++;
		play_if_sound_on(score_sfx.get());
		start_new_round("player2");
	}

	// If server, send data about the game state to the client every server_send_interval milliseconds for synchronization.
	if (server_send_interval + server_send_timer < SDL_GetTicks() && game_mode == GameMode::ONLINE_MULTIPLAYER && connection_manager.type == "server" && connection_manager.is_connected) {
		std::string serialized_data = "bx:" + std::to_string(ball.sprite.rect.x) +
									 " by:" + std::to_string(ball.sprite.rect.y) +
									 " p1:" + std::to_string(player_1.sprite.rect.y) + 
			                         " p1s:" + std::to_string(player_1_score) +
									 " p2s:" + std::to_string(player_2_score) + 
									 " bvx:" + std::to_string(ball.velocity_x) +
									 " bvy:" + std::to_string(ball.velocity_y) +
									 " endsc:" + std::to_string(end_score);
	// todo - maybe use protobuf instead?

		connection_manager.send_data(serialized_data);
		server_send_timer = SDL_GetTicks();
	}

	// If we moved in online play, let the other side know about that.
	if (game_mode == GameMode::ONLINE_MULTIPLAYER && connection_manager.is_connected && online_has_moved) {
		if (connection_manager.type == "client")
			connection_manager.send_data("p2:" + std::to_string(player_2.sprite.rect.y));
		else if (connection_manager.type == "server")
			connection_manager.send_data("p1:" + std::to_string(player_1.sprite.rect.y));

		online_has_moved = false;
	}
}
