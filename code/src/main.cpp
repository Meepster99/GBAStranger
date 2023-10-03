
#include "SharedTypes.h"

#include "Game.h"

/*
class Bruh {
public:
void realllylongfunctionname(int a, int b) {
	
	profileFunction();
		
	bn::timer idk;
	idk.restart();
	
	while(idk.elapsed_ticks() < 200) { }
	
}

void test() {
	profileFunction();
	
	bn::timer idk;
	idk.restart();
	
	
	while(idk.elapsed_ticks() < 2000) { }
	realllylongfunctionname(1,2);
	
	
}
};
*/

int main() {
	
	bn::core::init(); 

	/*Profiler::currentID = __PRETTY_FUNCTION__;	
	BN_PROFILER_START(__PRETTY_FUNCTION__); 
	
	Bruh b;
	
	
	b.test();
	b.test();
	
	BN_PROFILER_STOP();
	bn::profiler::show();
	*/
	
	// this needs to be a pointer, or else i get scary stack errors.
	Game* game = new Game();
	game->run();

}
