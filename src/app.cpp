#include "app.hpp"

bool App::initialize_sdl_subsystems() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cerr << "Failed to initialize SDL video and/or audio: " << SDL_GetError() << "\n";
		return false;
	}

	window = std::unique_ptr<SDL_Window, SDLGarbageCollector>(SDL_CreateWindow(window_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN));
	if (window == NULL) {
		std::cerr << "Window could not be created: " << SDL_GetError() << "\n";
		return false;
	}

	renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED), SDLGarbageCollector());
	if (renderer == NULL) {
		std::cerr << "Renderer could not be created: " << SDL_GetError() << "\n";
		return false;
	}
	else
		SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		std::cerr << "SDL_image could not be initialized: " << IMG_GetError() << "\n";
		return false;
	}

	if (TTF_Init() == -1) {
		std::cerr << "SDL_ttf could not be initialized: " << TTF_GetError() << "\n";
		return false;
	}
	
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cerr << "SDL_mixer could not be initialized: " << Mix_GetError() << "\n";
		return false;
	}

	SDL_SetMainReady(); // Let the rest of the SDL library know that its initialization was done properly.

	main_menu = { window_width, window_height, renderer.get() };
	game = { window_width, window_height, GameMode::SINGLE_PLAYER, renderer.get() }; // Initialize the game object that will be used when the player starts a game.

	SDL_StopTextInput();

	return true;
}

void App::quit_all_subsystems() {
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	WSACleanup();
}

void App::play_if_sound_on(Mix_Chunk* chunk, int loops) {
	if (sound_on)
		Mix_PlayChannel(-1, chunk, loops);
}

void App::process_input(SDL_Keycode pressed_key) { // We don't need to handle precise keyboard input when we're not in-game, so we're just using SDL events for keyboard input handling here.
	switch (pressed_key) {
		case SDLK_ESCAPE:
			quit_all_subsystems();
			app_active = false;
			break;
		case SDLK_BACKSPACE: // Handle backspace during typing.
			if (SDL_IsTextInputActive()) {
				if (main_menu.textbox.string.length() > 0) {
					main_menu.textbox.string.pop_back();

					if (main_menu.textbox.string.length() == 0)
						main_menu.textbox.string = " "; // If the result causes our string to become empty, turn it into a whitespace.

					main_menu.textbox.update_text();
					play_if_sound_on(main_menu.type_sfx.get());
				}
			}
			break;
		case SDLK_c: // Handle copy (CTRL + c) during typing.
			if (SDL_IsTextInputActive()) {
				if (SDL_GetModState() & KMOD_CTRL)
					SDL_SetClipboardText(main_menu.textbox.string.c_str());
			}
			break;
		case SDLK_v: // Handle paste (CTRL + v) during typing.
			if (SDL_IsTextInputActive()) {
				if (SDL_GetModState() & KMOD_CTRL) {
					main_menu.textbox.string = SDL_GetClipboardText();
					main_menu.textbox.update_text();
					play_if_sound_on(main_menu.type_sfx.get());
				}
			}
			break;
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (SDL_IsTextInputActive())
				main_menu.push_event("main_menu", "text_input_confirmed");
			break;
	}
}

void App::handle_events() {
	SDL_Event event;
	
	if (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_QUIT:
				quit_all_subsystems();
				app_active = false;
				break;
			case SDL_KEYDOWN:
				if (app_state != AppState::IN_GAME)
					process_input(event.key.keysym.sym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (app_state == AppState::MAIN_MENU && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT ) {
					for (auto&& button : main_menu.buttons) {
						if (button.is_hovered()) {
							play_if_sound_on(main_menu.button_click_sfx.get());
							button.run_on_click();
						}
					}
					for (auto&& button : main_menu.persistent_buttons) {
						if (button.is_hovered()) {
							play_if_sound_on(main_menu.button_click_sfx.get());
							button.run_on_click();
						}
					}
				}
				break;
			case SDL_USEREVENT:
			{
				std::unique_ptr<std::string> data1_ptr(static_cast<std::string*>(event.user.data1));
				std::unique_ptr<std::string> data2_ptr(static_cast<std::string*>(event.user.data2));

				std::string data1 = *data1_ptr;
				std::string data2 = *data2_ptr;

				if (data1 == "start_game") {
					main_menu.had_error = false;
					app_state = AppState::IN_GAME;

					if (data2 == "single_player")
						game.init_game(GameMode::SINGLE_PLAYER, window_width, window_height, sound_on, end_score_to_pass);
					else if (data2 == "practice")
						game.init_game(GameMode::PRACTICE, window_width, window_height, sound_on, end_score_to_pass);
					else if (data2 == "local_multiplayer")
						game.init_game(GameMode::LOCAL_MULTIPLAYER, window_width, window_height, sound_on, end_score_to_pass);
					else if (data2 == "online_multiplayer_as_client") {
						game.game_mode = GameMode::ONLINE_MULTIPLAYER;
						game.connection_manager.type = "client";
						update(); // Need to call these here for the screen to update and show the "connecting" message as the connection calls are blocking and prevent the screen from updating on its own.
						render();

						// Convert string to wstring.
						int len = 0;
						int slen = (int)main_menu.textbox.string.length() + 1;

						len = MultiByteToWideChar(CP_ACP, 0, main_menu.textbox.string.c_str(), slen, 0, 0);

						std::unique_ptr<wchar_t> buffer = std::unique_ptr<wchar_t>(new wchar_t[slen]);

						MultiByteToWideChar(CP_ACP, 0, main_menu.textbox.string.c_str(), slen, buffer.get(), len);

						game.connection_manager.server_ipv4 = buffer.get();

						game.init_game(GameMode::ONLINE_MULTIPLAYER, window_width, window_height, sound_on, end_score_to_pass);
					}
					else if (data2 == "online_multiplayer_as_server") {
						game.game_mode = GameMode::ONLINE_MULTIPLAYER;
						game.connection_manager.type = "server";
						update(); // // Need to call these here for the screen to update and show the "awaiting connection" message as the connection calls are blocking and prevent the screen from updating on its own.
						render();

						game.init_game(GameMode::ONLINE_MULTIPLAYER, window_width, window_height, sound_on, end_score_to_pass);
					}
					// Need to call these once here for the screen to update before the countdown.
					update();
					render();
				}
				else if (data1 == "main_menu") {
					if (data2 == "start") {
						SDL_StopTextInput();
						main_menu.buttons.clear();

						main_menu.buttons.emplace_back(main_menu.start_button);
						main_menu.buttons.emplace_back(main_menu.credits_button);
						main_menu.buttons.emplace_back(main_menu.quit_button);

						if (Mix_PlayingMusic())
							Mix_HaltMusic();
						if (sound_on)
							Mix_PlayMusic(main_menu.main_menu_music.get(), -1);

						app_state = AppState::MAIN_MENU;
					}
					else if (data2 == "game_modes") {
						main_menu.buttons.clear();

						main_menu.buttons.emplace_back(main_menu.single_player_button);
						main_menu.buttons.emplace_back(main_menu.practice_button);
						main_menu.buttons.emplace_back(main_menu.local_multiplayer_button);
						main_menu.buttons.emplace_back(main_menu.online_multiplayer_button);
						main_menu.buttons.emplace_back(main_menu.back_to_main_menu_button);
					}
					else if (data2 == "show_credits") {
						main_menu.buttons.clear();

						main_menu.buttons.emplace_back(main_menu.credits_inside_button);
						main_menu.buttons.emplace_back(main_menu.back_to_main_menu_button);
					}
					else if (data2 == "multiplayer_options") {
						main_menu.buttons.clear();

						main_menu.buttons.emplace_back(main_menu.back_to_main_menu_button);
						main_menu.buttons.emplace_back(main_menu.server_button);
						main_menu.buttons.emplace_back(main_menu.client_button);
					}
					else if (data2 == "get_end_score") {
						main_menu.buttons.clear();

						main_menu.give_input_text.swap_text("At which score should the game end?");
						main_menu.textbox.getting_input_of_what = "end_score";

						main_menu.buttons.emplace_back(main_menu.textbox_ok_button);
						main_menu.buttons.emplace_back(main_menu.back_to_main_menu_button);

						SDL_StartTextInput();
					}
					else if (data2 == "get_ipaddr") {
						main_menu.buttons.clear();

						main_menu.give_input_text.swap_text("Please enter the IP address to connect to.");
						main_menu.textbox.getting_input_of_what = "ip";

						main_menu.buttons.emplace_back(main_menu.textbox_ok_button);
						main_menu.buttons.emplace_back(main_menu.back_to_main_menu_button);

						SDL_StartTextInput();
					}
					else if (data2 == "text_input_confirmed") {
						if (main_menu.textbox.getting_input_of_what == "ip")
							main_menu.push_event("start_game", "online_multiplayer_as_client");
						else if (main_menu.textbox.getting_input_of_what == "end_score") {
							try {
								end_score_to_pass = std::stoi(main_menu.textbox.string);
							}
							catch (const std::invalid_argument) {
								main_menu.textbox.string = "not an int";
								main_menu.textbox.update_text();
								return;
							}
							catch (const std::out_of_range) {
								main_menu.textbox.string = "too big";
								main_menu.textbox.update_text();
								return;
							}

							SDL_StopTextInput();
							main_menu.push_event("start_game", main_menu.game_mode_to_pass);
						}
					}
				}
				else if (data1 == "toggle_sound") {
					sound_on = !sound_on;

					if (sound_on) {
						main_menu.sound_toggle_button.unhovered_sprite.swap_texture("sprites/sound_on.png", renderer.get());

						if (!Mix_PlayingMusic())
							Mix_PlayMusic(main_menu.main_menu_music.get(), -1);
					}

					if (!sound_on) {
						main_menu.sound_toggle_button.unhovered_sprite.swap_texture("sprites/sound_off.png", renderer.get());

						if (Mix_PlayingMusic())
							Mix_HaltMusic();
					}
				}
				else if (data1 == "main_menu_err") {
					main_menu.had_error = true;
					main_menu.error_text.swap_text(data2);
					main_menu.push_event("main_menu", "start");
				}
				else if (data1 == "exit") {
					quit_all_subsystems();
					app_active = false;
				}
				break;
			}
			case SDL_TEXTINPUT:
				if (!(main_menu.textbox.string.length() >= 15)) {
					if (main_menu.textbox.string == " ")
						main_menu.textbox.string = event.text.text;
					else
						main_menu.textbox.string = main_menu.textbox.string + event.text.text;

					main_menu.textbox.update_text();
					play_if_sound_on(main_menu.type_sfx.get());
				}
				break;
		}
	}

	if (app_state == AppState::IN_GAME) {
		game.tick();
	}
}

void App::update() {
	SDL_RenderClear(renderer.get());

	switch (app_state) {
		case AppState::MAIN_MENU:
			SDL_RenderCopy(renderer.get(), main_menu.background.texture.get(), NULL, &main_menu.background.rect);
			SDL_RenderCopy(renderer.get(), main_menu.game_sign.texture.get(), NULL, &main_menu.game_sign.rect);
			SDL_RenderCopy(renderer.get(), main_menu.city_back.texture.get(), &main_menu.city_back_current_rect, &main_menu.city_back.rect);
			SDL_RenderCopy(renderer.get(), main_menu.city_front.texture.get(), &main_menu.city_front_current_rect, &main_menu.city_front.rect);

			if (main_menu.had_error)
				SDL_RenderCopy(renderer.get(), main_menu.error_text.texture.get(), NULL, &main_menu.error_text.rect);
			
			// Makes the city move in the main menu.
			main_menu.current_frame_on_main_menu++;

			main_menu.city_front_current_rect.x++;
			if (main_menu.city_front_current_rect.x >= 3000)
				main_menu.city_front_current_rect.x = 0;
			
			if (main_menu.current_frame_on_main_menu % main_menu.frames_to_city_back_swap == 0) {
				main_menu.city_back_current_rect.x++;

				if (main_menu.city_back_current_rect.x >= 3000)
					main_menu.city_back_current_rect.x = 0;
			}

			// Render buttons in the main menu.
			for (auto&& button : main_menu.buttons) {
				if (button.is_hovered())
					SDL_RenderCopy(renderer.get(), button.hovered_sprite.texture.get(), NULL, &button.button_rect);
				else
					SDL_RenderCopy(renderer.get(), button.unhovered_sprite.texture.get(), NULL, &button.button_rect);

				SDL_RenderCopy(renderer.get(), button.button_text.texture.get(), NULL, &button.button_text.rect);
			}

			SDL_RenderCopy(renderer.get(), main_menu.github_button.unhovered_sprite.texture.get(), NULL, &main_menu.github_button.unhovered_sprite.rect);
			SDL_RenderCopy(renderer.get(), main_menu.sound_toggle_button.unhovered_sprite.texture.get(), NULL, &main_menu.sound_toggle_button.unhovered_sprite.rect);

			if (SDL_IsTextInputActive()) {
				SDL_RenderCopy(renderer.get(), main_menu.give_input_text.texture.get(), NULL, &main_menu.give_input_text.rect);
				SDL_RenderCopy(renderer.get(), main_menu.textbox.sprite.texture.get(), NULL, &main_menu.textbox.sprite.rect);
				SDL_RenderCopy(renderer.get(), main_menu.textbox.text.texture.get(), NULL, &main_menu.textbox.text.rect);
			}

			break;

		case AppState::IN_GAME:
			if (!game.has_ended) {
				SDL_RenderCopy(renderer.get(), game.game_background.texture.get(), NULL, &game.game_background.rect); // Render background.
				SDL_RenderCopy(renderer.get(), game.player_1.sprite.texture.get(), NULL, &game.player_1.sprite.rect); // Render left paddle.
				SDL_RenderCopy(renderer.get(), game.player_2.sprite.texture.get(), NULL, &game.player_2.sprite.rect); // Render right paddle.
				SDL_RenderCopy(renderer.get(), game.middle_line.texture.get(), NULL, &game.middle_line.rect); // Render the vertical line in the middle of the screen.
				SDL_RenderCopy(renderer.get(), game.player_1_score_text.texture.get(), NULL, &game.player_1_score_text.rect); // Render player 1's score.
				SDL_RenderCopy(renderer.get(), game.player_2_score_text.texture.get(), NULL, &game.player_2_score_text.rect); // Render player 2's score.
				SDL_RenderCopy(renderer.get(), game.ball.sprite.texture.get(), NULL, &game.ball.sprite.rect); // Render the ball.

				// Render the "awaiting connection" or "connecting" messages if the game is online multiplayer but we haven't connected to someone yet.
				if (game.game_mode == GameMode::ONLINE_MULTIPLAYER && !game.connection_manager.is_connected) {
					if (game.connection_manager.type == "server")
						SDL_RenderCopy(renderer.get(), game.server_awaiting_connection_text.texture.get(), NULL, &game.server_awaiting_connection_text.rect);
					else if (game.connection_manager.type == "client")
						SDL_RenderCopy(renderer.get(), game.client_connecting_text.texture.get(), NULL, &game.client_connecting_text.rect);
				}
				break;
			}
			else {
				switch (game.has_won) {
					case true:
						SDL_RenderCopy(renderer.get(), game.you_won_screen.texture.get(), NULL, &game.you_won_screen.rect);
						break;
					case false:
						SDL_RenderCopy(renderer.get(), game.you_lost_screen.texture.get(), NULL, &game.you_lost_screen.rect);
						break;
				}
			}
	}
}

void App::render() {
	SDL_RenderPresent(renderer.get());
}

void App::main_loop() {
	// Implementation of FPS lock using chrono.
	using dsec = std::chrono::duration<double>;
	auto casted_fps_limit = std::chrono::round<std::chrono::system_clock::duration>(dsec{ 1. / FPS_LIMIT }); // How many milliseconds will we wait for the next frame?
	auto frame_begin_time = std::chrono::system_clock::now(); // Frame start time is now.
	auto frame_end_time = frame_begin_time + casted_fps_limit; // Next frame will be in (time it took to process this frame) + 1000/FPS_LIMIT milliseconds.

	/* Uncomment this and the part inside the loop below to get reported of the framerate every second.
	unsigned int frame_count_per_second = 0;
	auto previous_time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(frame_begin_time);
	*/

	while (app_active) {
		if (std::chrono::system_clock::now() < frame_end_time) // If it's not the time for the next frame, wait until we're there.
			std::this_thread::sleep_until(frame_end_time);

		frame_begin_time = std::chrono::system_clock::now();
		frame_end_time = frame_begin_time + casted_fps_limit;

		handle_events();
		update();
		render();
		
		/*
		auto time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
		++frame_count_per_second;
		if (time_in_seconds > previous_time_in_seconds) {
			std::cerr << frame_count_per_second << " frames per second.\n"; // Using cerr for instant reports as it's not buffered like cout.
			frame_count_per_second = 0;
			previous_time_in_seconds = time_in_seconds;
		}
		*/
	}
}

App::App() {
	if (!(initialize_sdl_subsystems()))
		quit_all_subsystems();
}

