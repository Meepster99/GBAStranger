
#include "SharedTypes.h"

#include "Game.h"


class Bruh {
public:

void thegodsdontloveus(int a, int b) {
	profileFunction();
	
	bn::timer idk;
	idk.restart();
	
	while(idk.elapsed_ticks() < 20) { }
	
}

void realllylongfunctionname(int a, int b) {
	
	//BN_LOG("entering realllylongfunctionname");
	profileFunction();
		
	bn::timer idk;
	idk.restart();
	
	while(idk.elapsed_ticks() < 200) { }
	
	thegodsdontloveus(1,2);
	//BN_LOG("returning from realllylongfunctionname");
}

void test() {
	
	//BN_LOG("entering test");
	profileFunction();
	
	bn::timer idk;
	idk.restart();
	
	
	while(idk.elapsed_ticks() < 2000) { }
	realllylongfunctionname(1,2);
	
	//BN_LOG("returning from test");
}
};


int main() {
	
	bn::core::init(); 

	/*
	Bruh b;
	
	using WTFPROFILER::profilerMap;
	
	b.test();
	b.test();
	
	b.thegodsdontloveus(1,2);
	
	
	Profiler::show();
	
	while(true) {
		bn::core::update();
	}
	*/
	
	// this needs to be a pointer, or else i get scary stack errors.
	Game* game = new Game();
	game->run();

}
