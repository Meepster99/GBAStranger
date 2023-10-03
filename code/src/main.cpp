
#include "SharedTypes.h"

#include "Game.h"

int main() {
	
	bn::core::init(); 
	
	// this needs to be a pointer, or else i get scary stack errors.
	Game* game = new Game();
	game->run();
}
