
#include "Game.h"

#include "Palette.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

void Game::resetRoom(Entity* reason, bool debug) {
	
	if(!debug) {
		
		//fullDraw();
		floor.draw(collisionMap, floorMap);
		
		if(reason == NULL) {
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
	
	TileType* floorPointer = (TileType*)idek.floor;
	
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
			
			
			if(floorMap[x][y] != NULL) {
				delete floorMap[x][y];
			}
			
			floorMap[x][y] = NULL;

			switch(floorPointer[x + 14 * y]) {
				case TileType::Pit:
					break;
				case TileType::Floor:
					floorMap[x][y] = new FloorTile();
					break;
				case TileType::Glass:
					floorMap[x][y] = new Glass();
					break;
				case TileType::Bomb:
					floorMap[x][y] = new Bomb();
					break;
				case TileType::Death:
					floorMap[x][y] = new Death();
					break;
				case TileType::Copy:
					floorMap[x][y] = new Copy();
					break;
				case TileType::Exit:
					floorMap[x][y] = new Exit();
					break;
				case TileType::Switch:
					floorMap[x][y] = new Switch();
					break;
				default:
					BN_ERROR("unknown tile tried to get loaded in, wtf");
					break;
			}
		}
	}

	EntityHolder* entitiesPointer = (EntityHolder*)idek.entities;
	int entitiesCount = idek.entityCount;

	entityManager.loadEntities(entitiesPointer, entitiesCount);
	
}

void Game::fullDraw() {
	collision.draw(collisionMap);
	details.draw(detailsMap);
	floor.draw(collisionMap, floorMap);
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
	floor.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	
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
			
			BN_LOG(miscTimer.elapsed_ticks());
			
			miscTimer.restart();
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
			
			floor.draw(collisionMap, floorMap);
			
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

