
#include "Game.h"

#include "Palette.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

Game* globalGame = NULL;

unsigned int frame = 0;

void Game::doButanoUpdate() {
	bn::core::update();
	
	int temp = bn::core::last_missed_frames();
	if(temp != 0) {
		BN_LOG("dropped frames: ", temp);
	}
	
}	

void Game::resetRoom(bool debug) {
	
	BN_LOG("entered reset room with debug=",debug);
	
	if(!debug) {
		
		//tileManager.fullDraw();
		state = GameState::Exiting;
		
		// wait for animations to finish 
		// IS THERE A NON BUSYLOOP VER OF THIS?

		while(state == GameState::Exiting) { // wait for gamestate to no longer be exiting
			//BN_LOG("looping on ", state);
			
			//debugText.updateText();
			//bn::core::update(); 
			doButanoUpdate();
		} 
	}

	state = GameState::Loading;
	
	BN_LOG("resetroom called");
	loadLevel(debug);
	BN_LOG("loadlevel finished");
	if(!debug) {
		doButanoUpdate();
	}
	fullDraw();
	BN_LOG("fulldraw finished");
	
	state = GameState::Normal;
	
	if(!debug) {
		state = GameState::Entering;
		while(state == GameState::Entering) { // wait for gamestate to no longer be entering
			//BN_LOG("looping on ", state);
			//debugText.updateText();
			//bn::core::update(); 
			doButanoUpdate();
		}
	}
	
	BN_ASSERT(state == GameState::Normal, "after a entering gamestate, the next state should be normal");
	
	//BN_PROFILER_RESET();
}

void Game::loadLevel(bool debug) {

	Room idek = roomManager.loadRoom();

	for(int x=0; x<14; x++) { 
		for(int y=0; y<9; y++) {
			
			collisionMap[x][y] = ((u8*)idek.collision)[x + 14 * y];
			detailsMap[x][y] = ((u8*)idek.details)[x + 14 * y];
			
			// when changing the color palettes, some weird shit happened where, areas 
			// would be transparent when they shouldnt be? this hopefully fixes that
		
			if(collisionMap[x][y] == 0) {
				collisionMap[x][y] = 1;
			}
		}
	}
	
	TileType* floorPointer = (TileType*)idek.floor;
	
	
	
	tileManager.loadTiles(floorPointer);
	
	if(!debug) {
		doButanoUpdate();
	}

	EntityHolder* entitiesPointer = (EntityHolder*)idek.entities;
	int entitiesCount = idek.entityCount;
	
	entityManager.loadEntities(entitiesPointer, entitiesCount);
	
	effectsManager.reset();
	
}

void Game::fullDraw() {
	collision.draw(collisionMap);
	details.draw(detailsMap);

	tileManager.fullDraw();
	
	entityManager.fullUpdate();
}

void Game::changePalette(int offset) {
	
	// https://stackoverflow.com/questions/3417183/modulo-of-negative-numbers
	// lol
	
	const int paletteListSize = (int)(sizeof(paletteList) / sizeof(paletteList[0]));
	
	paletteIndex += offset;
	
	paletteIndex = ((paletteIndex % paletteListSize) + paletteListSize) % paletteListSize;
	
	entityManager.updatePalette(paletteList[paletteIndex]);
	
	collision.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	details.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	tileManager.floorLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	effectsManager.effectsLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	
	BackgroundMap::backgroundPalette = paletteList[paletteIndex];
	
}

void didVBlank() {

	//frame = (frame + 1) % 600000;
	frame++;
	if(frame > 600000) {
		frame = 0;
	}
	
	globalGame->doVBlank();
}

void Game::doVBlank() {
	
	// can vblank occur during the gameloop, or does this only get called after we call a butano update?
	// if this can get called in the gameloop, then we will have problems once we integrate dialogue, bc during dialogue this needs to be disabled!
	
	static bool a, b, c = false;
	
	switch(state) {
		default:
		case GameState::Normal:
			entityManager.doVBlank();
			effectsManager.doVBlank();
			tileManager.doVBlank();
			break;
		case GameState::Exiting:
			if(!a) { a = entityManager.exitRoom(); }
			if(!b) { b = effectsManager.exitRoom(); }
			if(!c) { c = tileManager.exitRoom(); }
			if(a && b && c) {
				state = GameState::Entering;
				a = b = c = false; // vine boom sound effect
			}
			break;
		case GameState::Entering:
			if(!a) { a = entityManager.enterRoom(); }
			if(!b) { b = effectsManager.enterRoom(); }
			if(!c) { c = tileManager.enterRoom(); }
			if(a && b && c) {
				state = GameState::Normal;
				a = b = c = false; // vine boom sound effect
			}
			break;
		case GameState::Loading:
		case GameState::Paused:
			break;
	}
	
	
}

void Game::run() {
	
	BN_LOG("look at u bein all fancy lookin in the logs");
	
	globalGame = this;

	bn::core::set_vblank_callback(didVBlank);
	
	bn::timer inputTimer;
	
	state = GameState::Loading;
	
	loadLevel();
	fullDraw();
	
	state = GameState::Normal;
	
	
	//bn::sound_items::msc_013.play(1);
	//bn::music_items::cyberrid.play(0.5);
	
	while(true) {
		
		miscDebug = LevStatue::rodUses;
		miscDebug2 = LevStatue::totalLev;
		
		if(bn::keypad::l_held() || bn::keypad::r_held()) {
			
			int debugIncrement = bn::keypad::select_held() ? 5 : 1;
			
			if(bn::keypad::l_held()) {
				for(int i=0; i<debugIncrement; i++) {
					roomManager.prevRoom();
				}
			} else {
				for(int i=0; i<debugIncrement; i++) {
					roomManager.nextRoom();
				}
			}
			resetRoom(true);
			
			miscTimer.restart();
			
			while(miscTimer.elapsed_ticks() < FRAMETICKS * 5) { }
	
			doButanoUpdate();

			continue;
		}
		
		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > FRAMETICKS * 3) {
			
			inputTimer.restart();
			
			if(bn::keypad::start_pressed()) {
				changePalette(1);
				continue;
			}
	
			if(bn::keypad::select_pressed()) {
				continue;
			}
			
			if(bn::keypad::b_pressed()) {
				if(bn::keypad::select_held()) {
					Profiler::reset();
				} else {
					Profiler::show();
				}
				continue;
			}
			
			entityManager.doMoves();
		
			if(entityManager.hasKills()) {
				resetRoom();
				continue;
			}
			
			bn::fixed tickCount = inputTimer.elapsed_ticks();
			(void)tickCount; // supress warning if logging is disabled
			BN_LOG("a move took ", tickCount / FRAMETICKS, " frames");
		
			
		}

		doButanoUpdate();
	}
}

