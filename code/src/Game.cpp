
#include "Game.h"

#include "Palette.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

Game* globalGame = NULL;

unsigned int frame = 0;

bn::random randomGenerator = bn::random();

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
		state = GameState::Exiting;
		
		// wait for animations to finish 
		while(state == GameState::Exiting) { // wait for gamestate to no longer be exiting
			doButanoUpdate();
		} 
	}

	state = GameState::Loading;
	save();
	
	loadLevel(debug);
	if(!debug) {
		doButanoUpdate();
	}
	fullDraw();
	
	state = GameState::Normal;
	
	if(!debug) {
		state = GameState::Entering;
		while(state == GameState::Entering) { // wait for gamestate to no longer be entering
			doButanoUpdate();
		}
	}
	
	BN_ASSERT(state == GameState::Normal, "after a entering gamestate, the next state should be normal");
	
	BN_LOG("reset room done");
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
	

	EffectHolder* effectsPointer = (EffectHolder*)idek.effects;
	int effectsCount = idek.effectsCount;
	
	effectsManager.loadEffects(effectsPointer, effectsCount);
	
	
	
}

void Game::fullDraw() {
	collision.draw(collisionMap);
	details.draw(detailsMap);

	tileManager.fullDraw();
	
	entityManager.fullUpdate();
}

void Game::fullTileDraw() {
	collision.draw(collisionMap);
	details.draw(detailsMap);
	tileManager.fullDraw();
}

void Game::changePalette(int offset) {
	
	// https://stackoverflow.com/questions/3417183/modulo-of-negative-numbers
	// lol
	
	const int paletteListSize = (int)(sizeof(paletteList) / sizeof(paletteList[0]));
	
	paletteIndex += offset;
	
	paletteIndex = ((paletteIndex % paletteListSize) + paletteListSize) % paletteListSize;
	
	entityManager.updatePalette(paletteList[paletteIndex]);
	effectsManager.updatePalette(paletteList[paletteIndex]);
	
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

	load();
	
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
			//BN_LOG("a move took ", tickCount / FRAMETICKS, " frames");
			BN_LOG("a move took ", tickCount.safe_division(FRAMETICKS), " frames");
		}
		
		doButanoUpdate();
	}
}

uint64_t Game::getSaveHash() {
	uint64_t hash = 0;
	
	#define rotateHash(n) hash = (hash << n) | (hash >> ((sizeof(hash) * 8) - n))
	
	// this is barely even a hash algorithm, but it will work ig
	
	hash ^= saveData.locustCount;
	rotateHash(sizeof(saveData.locustCount) * 8);
	
	hash ^= saveData.isVoided;
	rotateHash(sizeof(saveData.isVoided) * 8);
	
	hash ^= saveData.roomIndex;
	rotateHash(sizeof(saveData.roomIndex) * 8);

	return hash;
}

void Game::save() {
	BN_LOG("saving save");
	
	saveData.locustCount = entityManager.player->locustCount;
	saveData.isVoided = entityManager.player->isVoided;
	saveData.roomIndex = roomManager.roomIndex;
	
	saveData.hash = getSaveHash();
	bn::sram::write(saveData);
	
	BN_LOG("locust: ", saveData.locustCount);
	BN_LOG("void: ", saveData.isVoided);
	BN_LOG("room: ", saveData.roomIndex);
}

void Game::load() {
	BN_LOG("loading save");
	bn::sram::read(saveData);
	
	if(saveData.hash != getSaveHash()) {
		BN_LOG("either a save wasnt found, or it was corrupted. creating new save");
		saveData = GameSave();
	}
	
	BN_LOG("locust: ", saveData.locustCount);
	BN_LOG("void: ", saveData.isVoided);
	BN_LOG("room: ", saveData.roomIndex);
	
	roomManager.roomIndex = saveData.roomIndex;
}

