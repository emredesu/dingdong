#include "mainmenu.hpp"

MainMenu::MainMenu(int screen_width_param, int screen_height_param, SDL_Renderer* renderer) {
	renderer_ptr = std::shared_ptr<SDL_Renderer>(renderer, SDL_DestroyRenderer);
	screen_width = screen_width_param;
	screen_height = screen_height_param;

	// Prepare the sprites for the main menu.
	background = { "sprites/main_menu_background.png", renderer_ptr.get() };
	city_back = { "sprites/main_menu_city_back.png", renderer_ptr.get() };
	city_front = { "sprites/main_menu_city_front.png", renderer_ptr.get() };
	game_sign = { "sprites/main_menu_game_sign.png", renderer_ptr.get(), 117, 107};
	textbox = { screen_width / 2 - 125, screen_height / 2, renderer_ptr.get() };

	city_front.rect.h = city_back.rect.h = screen_height; // Changing the width and height of city back/front and stars so that it's the same as the window we're rendering it in.
	city_front.rect.w = city_back.rect.w = screen_width;

	start_button = { 148, 330, "START GAME", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), list_game_modes, 20 };
	credits_button = { 389, 330, "CREDITS", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), show_credits };
	quit_button = { 630, 330, "QUIT", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), send_exit };
	single_player_button = { 118, 330, "SINGLE PLAYER", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), start_single_player, 13 };
	practice_button = { 118, 440, "PRACTICE", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), start_practice };
	local_multiplayer_button = { 660, 330, "LOCAL MULTIPLAYER", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), start_local_multiplayer, 12 };
	online_multiplayer_button = { 660, 440, "ONLINE MULTIPLAYER", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), show_multiplayer_options, 12 };
	client_button = { 148, 330, "JOIN", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), get_ipaddr };
	server_button = { 630, 330, "HOST", "sprites/large_button_unhovered.png", "sprites/large_button_hovered.png", renderer_ptr.get(), start_server };
	back_to_main_menu_button = { 940, 740, "<-", "sprites/small_button_unhovered.png", "sprites/small_button_hovered.png", renderer_ptr.get(), init_main_menu, 15 };
	textbox_ok_button = { (textbox.sprite.rect.x + textbox.sprite.rect.w / 2 - 25), (textbox.sprite.rect.y + textbox.sprite.rect.h) + 10, "OK", "sprites/small_button_unhovered.png", "sprites/small_button_hovered.png", renderer_ptr.get(), textbox_ok_button_func, 15 };
	credits_inside_button = { 117, (screen_height / 2), " ", "sprites/credits_button_unhovered.png", "sprites/credits_button_unhovered.png", renderer_ptr.get(), open_ulasyt };

	sound_toggle_button = { 10, screen_height_param - 72, " ", "sprites/sound_on.png", "none", renderer_ptr.get(), toggle_sound, 24, true };
	github_button = { 92, screen_height_param - 72, " ", "sprites/GitHub-Mark-Light-64px.png", "none", renderer_ptr.get(), open_github_link, 24, true };

	persistent_buttons.emplace_back(sound_toggle_button);
	persistent_buttons.emplace_back(github_button);

	give_input_text = { "At which score should the game end?", renderer_ptr.get(), 20 };
	client_timeout_text = { "Could not connect to the server.", renderer_ptr.get(), 24, {255, 0, 0} };
	server_timeout_text = { "No connections received after 60 seconds.", renderer_ptr.get(), 24, {255, 0, 0} };
	disconnection_text = { "Disconnected.", renderer_ptr.get(), 24, {255, 0, 0} };
	error_text = { "No error. You should not be seeing this :c", renderer_ptr.get(), 16, {255, 0, 0}, 5, 25 };

	give_input_text.rect.x = textbox.sprite.rect.x - 300;
	give_input_text.rect.y = textbox.sprite.rect.y - 50;

	// Prepare music files for main menu.
	main_menu_music = std::shared_ptr<Mix_Music>(Mix_LoadMUS("music/main_menu.wav"), SDLGarbageCollector());

	button_click_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/Next.wav"), SDLGarbageCollector());
	type_sfx = std::shared_ptr<Mix_Chunk>(Mix_LoadWAV("sfx/Type.wav"), SDLGarbageCollector());
	
	// todo - github and sound toggle buttons

	init_main_menu();
}

// Pushes an SDL_USEREVENT with two datas as string pointers.
void MainMenu::push_event(std::string data1, std::string data2) {
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

// The menu at the start - contains play, credits, Github and quit buttons.
void MainMenu::init_main_menu() {
	push_event("main_menu", "start");
}

void MainMenu::list_game_modes() {
	push_event("main_menu", "game_modes");
}

void MainMenu::textbox_ok_button_func() {
	push_event("main_menu", "text_input_confirmed");
}

void MainMenu::show_credits() {
	push_event("main_menu", "show_credits");
}

void MainMenu::show_multiplayer_options() {
	push_event("main_menu", "multiplayer_options");
}

void MainMenu::get_end_score() {
	push_event("main_menu", "get_end_score");
}

void MainMenu::get_ipaddr() {
	push_event("main_menu", "get_ipaddr");
}

void MainMenu::send_exit() {
	push_event("exit");
}

void MainMenu::start_single_player() {
	game_mode_to_pass = "single_player";
	push_event("main_menu", "get_end_score");
}

void MainMenu::start_practice() {
	game_mode_to_pass = "practice";
	push_event("main_menu", "get_end_score");
}

void MainMenu::start_local_multiplayer() {
	game_mode_to_pass = "local_multiplayer";
	push_event("main_menu", "get_end_score");
}

void MainMenu::start_client() {
	SDL_StopTextInput();

	push_event("start_game", "online_multiplayer_as_client");
}

void MainMenu::start_server() {
	game_mode_to_pass = "online_multiplayer_as_server";
	push_event("main_menu", "get_end_score");
}

void MainMenu::toggle_sound() {
	push_event("toggle_sound");
}

void MainMenu::open_ulasyt() {
	system("start https://www.youtube.com/channel/UCENSxGq1129zdHRiuSwY7lg");
}

void MainMenu::open_github_link() {
	system("start https://github.com/emredesu/dingdong");
}