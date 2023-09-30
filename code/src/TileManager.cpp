

#include "TileManager.h"
#include "EffectsManager.h"
#include "EntityManager.h"

#include "Game.h"


void TileManager::loadTiles(TileType* floorPointer) {
	
	floorSteps.clear();
	
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
	

	BN_ASSERT(Switch::pressedCount == 0, "after loading in new tiles, the number of pressed tiles wasnt 0?");
	
	BN_ASSERT(Switch::totalCount == switchTracker, "after loading in new tiles, the number of switches and number inside the class wasnt equal???");
	
}

// ----- 

void TileManager::doFloorSteps() {
	
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
	
	for(auto it = floorSteps.begin(); it != floorSteps.end(); ++it) {
		
		BN_ASSERT((*it).second.first != (*it).second.second, "in doFloorSteps, calculating steps. why is a move here with the same start and end??");
		
		stepOffs.insert((*it).second.first);
		stepOns.insert((*it).second.second);
	}
	
	// stepoffs occur before stepons bc of shadows, and switches
	for(auto it = stepOffs.begin(); it != stepOffs.end(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) {
			stepOff(tempPos);
		}
	}
	
	for(auto it = stepOns.begin(); it != stepOns.end(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) {
			stepOn(tempPos);
		}
	}

	
	// check if a player has stepped on a copy tileType
	// literally spent had to redo so much code just for this omfg 
	// ive said it twice before, should of had a tilemanager class.
	
	// ok now we are in that class. im still going to leave parts ofthis hardcoded tho
	for(auto it = floorSteps.begin(); it != floorSteps.end(); ++it) {
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
	// refactor it so that tiles can kill entities!
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
	
	if(hasFloor(entityManager->player->p) == TileType::Exit && Switch::pressedCount == Switch::totalCount) {
		entityManager->addKill(NULL);
	}
	

}

void TileManager::fullDraw() {
	floorLayer.draw(game->collisionMap, floorMap);
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


bn::optional<TileType> TileManager::hasFloor(Pos p) {
	FloorTile* temp = floorMap[p.x][p.y];
	
	if(temp == NULL || !temp->isAlive) {
		return bn::optional<TileType>();
	}
	
	return bn::optional<TileType>(temp->tileType());
}

void TileManager::stepOff(Pos p) {
	// should a check be here for tile isalive as well?
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepoff on a tile, it was null?");
	
	floorMap[p.x][p.y]->stepOff();
}

void TileManager::stepOn(Pos p) {
	// should a check be here for tile isalive as well?
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepon on a tile, it was null?");
	
	floorMap[p.x][p.y]->stepOn();
}


