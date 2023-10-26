
#include "SharedTypes.h"

#include "Game.h"


int main() {
	
	bn::core::init(); 
	
	bn::bg_tiles::set_allow_offset(true);
	
	//bn::bg_tiles::log_status();
	
	// this needs to be a pointer, or else i get scary stack errors.
	Game* game = new Game();
	game->run();
	
}
