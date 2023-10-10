

#include "TileManager.h"
#include "EffectsManager.h"
#include "EntityManager.h"

#include "Game.h"

// TODO: should tiles have entitymanagaer access?
// copy tiles could call shadow spawns from there
// death tiles could kill from there 
// also, i would be passing the entity in as an optional param, maybe?
// assuming that doesnt cause slowdown, that would be a good idea

void TileManager::loadTiles(TileType* floorPointer) {
	
	floorSteps.clear();
	
	if(entityManager->player != NULL) {
		if(entityManager->player->rod != NULL) {
			delete entityManager->player->rod;	
			entityManager->player->rod = NULL;
		}
	}
	
	exitTile = NULL;
	rodTile = NULL;
	locustTile = NULL;
	locustCounterTile = NULL;
	
	int switchTracker = 0;

	for(int x=0; x<14; x++) { 
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] != NULL) {
				delete floorMap[x][y];
			}
			
			floorMap[x][y] = NULL;
			
			Pos tempPos(x, y);

			switch(floorPointer[x + 14 * y]) {
				case TileType::Pit:
					break;
				case TileType::Floor:
					floorMap[x][y] = new FloorTile(tempPos);
					break;
				case TileType::Glass:
					floorMap[x][y] = new Glass(tempPos);
					break;
				case TileType::Bomb:
					floorMap[x][y] = new Bomb(tempPos);
					break;
				case TileType::Death:
					floorMap[x][y] = new Death(tempPos);
					break;
				case TileType::Copy:
					floorMap[x][y] = new Copy(tempPos);
					break;
				case TileType::Exit:
					floorMap[x][y] = new Exit(tempPos);
					//BN_ASSERT(exitTile == NULL, "tried loading in two exits on one level?");
					// i could just,,, loop over the floor every time a switch is pressed, but i dont rlly want to do that
					exitTile = static_cast<Exit*>(floorMap[x][y]);
					break;
				case TileType::Switch:
					floorMap[x][y] = new Switch(tempPos);
					switchTracker++;
					break;
				default:
					BN_ERROR("unknown tile tried to get loaded in, wtf");
					break;
			}
		}
	}
	
	//BN_ASSERT(exitTile != NULL, "no exittile was loaded in this level?");

	BN_ASSERT(Switch::pressedCount == 0, "after loading in new tiles, the number of pressed tiles wasnt 0?");
	
	BN_ASSERT(Switch::totalCount == switchTracker, "after loading in new tiles, the number of switches and number inside the class wasnt equal???");
	
	// draw out the UI.
	
	// SHOULD THE UI BE EFFECT BASED?
	// i could add funcs for pickup and putdown, and have the tiles possess effectswhich draw them out?
	// ugh, tbh it looks fine except for the D in void. maybe thats a manual fix.
	
	floorMap[0][8] = new WordTile(Pos(0, 8));
	
	floorMap[1][8] = new WordTile(Pos(1, 8), 'V', 'O');
	floorMap[2][8] = new WordTile(Pos(2, 8), 'I', 'D');
	
	voidTile1 = static_cast<WordTile*>(floorMap[1][8]);
	voidTile2 = static_cast<WordTile*>(floorMap[2][8]);
	
	floorMap[3][8] = new WordTile(Pos(3, 8));
	//floorMap[4][8] = new WordTile(Pos(4, 8), 'L', 'C');
	floorMap[4][8] = new LocustTile(Pos(4, 8));
	locustTile = static_cast<LocustTile*>(floorMap[4][8]);
	
	//floorMap[5][8] = new WordTile(Pos(5, 8), '4', '2');
	floorMap[5][8] = new WordTile(Pos(5, 8), ' ', ' ');
	locustCounterTile = static_cast<WordTile*>(floorMap[5][8]);
	
	//floorMap[6][8] = new WordTile(Pos(6, 8), 'R', 'D');
	floorMap[6][8] = new RodTile(Pos(6, 8));
	rodTile = static_cast<RodTile*>(floorMap[6][8]);
	
	floorMap[7][8] = new WordTile(Pos(7, 8));
	floorMap[8][8] = new WordTile(Pos(8, 8));
	floorMap[9][8] = new WordTile(Pos(9, 8));
	floorMap[10][8] = new WordTile(Pos(10, 8));
	floorMap[11][8] = new WordTile(Pos(11, 8));
	
	// this should be changed. roommanager should just have a array with a 3 length char array for what floor number should be displayed(or ???)
	int roomIndex = game->roomManager.roomIndex;
	
	BN_ASSERT(roomIndex <= 999, "why in tarnation is the roommanager's roomindex greater than 999???");
	
	floorMap[12][8] = new WordTile(Pos(12, 8), 'B', '0' + roomIndex / 100);
	floorMap[13][8] = new WordTile(Pos(13, 8), '0' + (roomIndex / 10) % 10, '0' + (roomIndex % 10));
	
}

// ----- 

void TileManager::doFloorSteps() { profileFunction();
	
	// rlly should of made a tilemanager
	
	// STEP UPDATES OCCUR AFTER DOING EXIT CHECKING, 
	// SINCE IF EVERY BUTTON IS PRESSED, YOU HAVE ONE TICK TO EXIT
	// but,, what about shadows,, this is weird
	// THIS DELAYED PRESS THING ONLY WORKS IF IT WAS A NONPLAYER ENTITY PRESSING IT!!
	
	// CURRENTLY SINCE I WANTED TO STORE THE ENTITYTYPE, POSES ARE
	// NO LONGER UNIQUE
	
	bn::optional<Entity*> res;
	
	// horrid memory usage
	static SaneSet<Pos, MAXENTITYSPRITES> stepOns;
	static SaneSet<Pos, MAXENTITYSPRITES> stepOffs;
	stepOns.clear();
	stepOffs.clear();
	
	for(auto it = floorSteps.cbegin(); it != floorSteps.cend(); ++it) {
		
		BN_ASSERT((*it).second.first != (*it).second.second, "in doFloorSteps, calculating steps. why is a move here with the same start and end??");
		
		stepOffs.insert((*it).second.first);
		stepOns.insert((*it).second.second);
	}
	
	// stepoffs occur before stepons bc of shadows, and switches
	for(auto it = stepOffs.cbegin(); it != stepOffs.cend(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) {
			stepOff(tempPos);
		}
	}
	
	for(auto it = stepOns.cbegin(); it != stepOns.cend(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) {
			stepOn(tempPos);
		}
	}
	
	
	// ok now we are in that class. im still going to leave parts ofthis hardcoded tho
	for(auto it = floorSteps.cbegin(); it != floorSteps.cend(); ++it) {
		if((*it).first == EntityType::Player) {
			
			Pos start = (*it).second.first;

			if(hasFloor(start) == TileType::Copy) {
				entityManager->shadowQueue.push_back(start);
			}
		}
	}
	

	stepOns.clear();
	stepOffs.clear();
	floorSteps.clear();
	

	// this code is not efficient
	// TODO:refactor it so that tiles can kill entities!
	// for now, im just going to have death tiles not work, as im on a trail lol
	/*
	// this code was causing HORRID slowdown.
	// but why?
	
	=// it would be ideal for me to pass the entity to the tile step func tbh, to do kills
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			if(!hasFloor(Pos(x, y))) {
				continue;
			}
	
			switch(hasFloor(Pos(x, y)).value()) {
				case TileType::Death:
					for(auto it = entityManager->getMap(Pos(x, y)).begin(); it != entityManager->getMap(Pos(x, y)).end(); ) {
						if((*it)->entityType() == EntityType::Player) {
							entityManager->addKill(*it); 
							++it;
						} else {
							it = entityManager->killEntity(*it);
						}
					}
					break;
				default:
					break;
			}
		}
	}
	*/
	
	if(hasFloor(entityManager->player->p) == TileType::Exit && Switch::pressedCount == Switch::totalCount) {
		entityManager->addKill(NULL);
	}
	
	
	// this does not need to be called every time
	// if it causes slowdown, fix it
	floorLayer.reloadCells();
}

void TileManager::updateTile(const Pos& p) {
	
	const u8 x = p.x;
	const u8 y = p.y;

	if(floorMap[x][y] != NULL && !floorMap[x][y]->isAlive) {
		delete floorMap[x][y];
		floorMap[x][y] = NULL;
	}
	
	if(floorMap[x][y] == NULL) {
		// the collision check isnt needed, but im keeping it here just in case
		// i also could(and maybe should?) use the hascollison func. 
		// idk abt speed but it would be good to standardize it, as im currently doing with the hasfloor func
		
		if(y > 0 && !hasCollision(Pos(x, y-1)) && hasFloor(x, y-1) && floorMap[x][y-1]->drawDropOff()) {
			FloorTile::drawDropOff(x, y);
		} else {
			FloorTile::drawPit(x, y);
		}
		
		// some issues could be caused here if this tile isnt freed yet. 
		// rewrite all map calls to hasfloor
		if(y < 8 && !hasFloor(x, y+1) && !hasCollision(Pos(x, y+1))) {
			FloorTile::drawPit(x, y+1);
		}
		
	} else {
		floorMap[x][y]->draw();
		
		if(floorMap[x][y]->drawDropOff() && y < 7 && !hasFloor(x, y+1) && !hasCollision(Pos(x, y+1))) {
			FloorTile::drawDropOff(x, y+1);
		}
	}
	
}

void TileManager::updateExit() {
	if(exitTile == NULL) {
		return;
	}
	if(exitTile == entityManager->player->rod) {
		return;
	}
	updateTile(exitTile->tilePos);
}

void TileManager::updateRod() {
	// i dont like the way these funcs are being coded tbh
	if(rodTile == NULL) {
		return;
	}
	if(rodTile == entityManager->player->rod) {
		return;
	}
	
	updateTile(rodTile->tilePos);
}

void TileManager::updateLocust() {

	if(locustTile != NULL && locustTile != entityManager->player->rod) {
		updateTile(locustTile->tilePos);
	}
	
	if(locustCounterTile != NULL && locustCounterTile != entityManager->player->rod) {
		
		if(entityManager->player->locustCount != 0) {
			locustCounterTile->first = '0' + ((entityManager->player->locustCount / 10) % 10);
			locustCounterTile->second = '0' + (entityManager->player->locustCount % 10);
		}

		updateTile(locustCounterTile->tilePos);
	}
	
}

void TileManager::updateVoidTiles() {
	
	bool isVoided = entityManager->player->isVoided;
	
	BN_LOG("player void status ", isVoided);
	
	if(voidTile1 != NULL && voidTile1 != entityManager->player->rod) {
		
		voidTile1->first = isVoided ? 'V' : 'H';
		voidTile1->second = isVoided ? 'O' : 'P';
		
		updateTile(voidTile1->tilePos);
	}
	
	if(voidTile2 != NULL && voidTile2 != entityManager->player->rod) {
		
		voidTile2->first = isVoided ? 'I' : '0';
		voidTile2->second = isVoided ? 'D' : '7';
		
		updateTile(voidTile2->tilePos);
	}
	
	
}

bool TileManager::hasCollision(const Pos& p) {
	return entityManager->hasCollision(p);;
}

void TileManager::fullDraw() { 
	
	floorLayer.draw(game->collisionMap, floorMap);
	
	// i do not like this!
	updateExit();
	updateRod();
	updateLocust();
	updateVoidTiles();
}

bool TileManager::exitRoom() {
	return true;
}

bool TileManager::enterRoom() {
	return true;
}

void TileManager::doVBlank() {
	
	// things like glass breaking(and maybe others) should occur in here!
	
	
	
	return;
}

// -----

bn::optional<TileType> TileManager::hasFloor(const u8& x, const u8& y) { profileFunction();
	FloorTile* temp = floorMap[x][y];
	
	if(temp == NULL || !temp->isAlive) {
		return bn::optional<TileType>();
	}
	
	return bn::optional<TileType>(temp->tileType());
}

void TileManager::stepOff(Pos p) { profileFunction();
	// should a check be here for tile isalive as well?
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepoff on a tile, it was null?");
	
	floorMap[p.x][p.y]->stepOff();
}

void TileManager::stepOn(Pos p) { profileFunction();
	// should a check be here for tile isalive as well?
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepon on a tile, it was null?");
	
	floorMap[p.x][p.y]->stepOn();

}


