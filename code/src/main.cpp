
#include "SharedTypes.h"

#include "Game.h"


int main() {
	
	bn::core::init(); 
	
	bn::bg_tiles::set_allow_offset(true);
	
	Game* game = NULL;
	
	// this needs to be a pointer, or else i get scary stack errors.
	while(true) {
		game = new Game();
		game->run();
		delete game;
		game = NULL;
		bn::core::update();
		
		/*
		BN_LOG("bg_maps status");
		bn::bg_maps::log_status();
		BN_LOG("bg_tiles status");
		bn::bg_tiles::log_status();
		BN_LOG("sprite_tiles status");
		bn::sprite_tiles::log_status();
		BN_LOG("bg_palettes status");
		bn::bg_palettes::log_status();
		BN_LOG("sprite_palettes status");
		bn::sprite_palettes::log_status();
		BN_LOG("memory status");
		bn::memory::log_alloc_ewram_status();
		*/
	}
	
}
