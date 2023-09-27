
#include "Game.h"

#include "Palette.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

void Game::resetRoom(Entity* reason, bool debug) {
	
	if(!debug) {
		
		tileManager.fullDraw();
		
		if(reason == NULL) {
			entityManager.player->doUpdate();
			debugText.updateText();
			bn::core::update();
			roomManager.nextRoom();
		} else if(reason->entityType() == EntityType::Player) {
			reason->doUpdate();
		}
		
		for(int i=0; i<10; i++) {
			miscTimer.restart();
			while(miscTimer.elapsed_ticks() < FRAMETICKS * 6) {
				debugText.updateText();
				bn::core::update();
			}
			if(reason != NULL) {
				reason->doTick();
			}
		}		
	}
	
	BN_LOG("resetroom called");
	loadLevel();
	BN_LOG("loadlevel finished");
	fullDraw();
	BN_LOG("fulldraw finished");
}

void Game::loadLevel() {

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
			
			/*if(detailsMap[x][y] == 0) {
				detailsMap[x][y] = 1;
			} */
		}
	}
	
	TileType* floorPointer = (TileType*)idek.floor;
	
	tileManager.entityManager = &entityManager;
	tileManager.effectsManager = &effectsManager;
	
	tileManager.loadTiles(floorPointer);

	EntityHolder* entitiesPointer = (EntityHolder*)idek.entities;
	int entitiesCount = idek.entityCount;
	
	Entity::entityManager = &entityManager;
	Entity::effectsManager = &effectsManager;
	Entity::tileManager = &tileManager;

	entityManager.effectsManager = &effectsManager;
	entityManager.tileManager = &tileManager;
	
	entityManager.loadEntities(entitiesPointer, entitiesCount);
	
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
	
	BackgroundMap::backgroundPalette = paletteList[paletteIndex];
	
}

bool vblanked = false;

void didVBlank() {
	vblanked = true;
}

void Game::run() {

	bn::core::set_vblank_callback(didVBlank);
	
	bn::timer inputTimer;

	int frame = 0;
	
	resetRoom(NULL, true);

	bn::optional<Entity*> test = NULL;
	
	while(true) {
		
		if(vblanked) {
			
			vblanked = false;
		
			frame = (frame + 1) % 60000;

			miscDebug = frame;

		}
		
		
		if(bn::keypad::l_held() || bn::keypad::r_held()) {
			if(bn::keypad::l_held()) {
				roomManager.prevRoom();
			} else {
				roomManager.nextRoom();
			}
			resetRoom(NULL, true);
			
			miscTimer.restart();
			
			while(miscTimer.elapsed_ticks() < 254843 / 12) {
			
			}
		
			debugText.updateText();
			bn::core::update();

			continue;
		}
		
		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > 254843 / 20) {
			
			inputTimer.restart();
			
			if(bn::keypad::start_pressed()) {
				changePalette(1);
				continue;
			} else if(bn::keypad::select_pressed()) {
				changePalette(-1);
				continue;
			}
			
			
			bn::optional<Entity*> res = entityManager.doMoves();
			
			BN_LOG("domoves returned ", res.has_value());
			
			if(res) {
				resetRoom(res.value());
				continue;
			}
			
			// inefficient, i rlly should of had a floormanager class.
			
			tileManager.fullDraw();
			
		}
		
		// is modulo expensive???
		if(frame % 25 == 0) {
			entityManager.doTicks();
		}
		
		if(frame % 8 == 0) {
			entityManager.doDeaths();
		}


		debugText.updateText();
		bn::core::update();
	}
}

