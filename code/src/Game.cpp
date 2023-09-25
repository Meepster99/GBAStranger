
#include "Game.h"


void Game::resetRoom() {
	loadLevel();
	fullDraw();
}

void Game::loadLevel() {

	Room idek = roomManager.loadRoom();
	
	memcpy(collisionMap, idek.collision, 14 * 9);
	memcpy(detailsMap, idek.details, 14 * 9);
		
	TileType* floorPointer = (TileType*)idek.floor;
	
	for(int i=0; i<14*9; i++) {
		if(floorMap[i] != NULL) {
			delete floorMap[i];
		}
		
		floorMap[i] = NULL;

		switch(floorPointer[i]) {
			case TileType::Pit:
				break;
			case TileType::Floor:
				floorMap[i] = new FloorTile();
				break;
			case TileType::Glass:
				floorMap[i] = new Glass();
				break;
			case TileType::Bomb:
				floorMap[i] = new Bomb();
				break;
			case TileType::Death:
				floorMap[i] = new Death();
				break;
			case TileType::Copy:
				floorMap[i] = new Copy();
				break;
			case TileType::Exit:
				floorMap[i] = new Exit();
				break;
			case TileType::Switch:
				floorMap[i] = new Switch();
				break;
			default:
				BN_ERROR("unknown tile tried to get loaded in, wtf");
				break;
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
}

void Game::run() {
	
	
	Pos p1(12, 8);
	Pos p2(2, 8);
	Pos p3(1, 8);
	
	p3.move(Direction::Right);
	
	
	resetRoom();
	//entityManager.entityList.insert(new Player(Pos(0, 0)));
	
	//EntitySet<8> test;
	//test.insert(new Player(Pos(0, 0)));
	//test.insert(new Player(Pos(0, 1)));
	//BN_ERROR(test.size());
	
	while(true) {
		
		BN_ASSERT(p1 != p2, "p1 != p2");
		BN_ASSERT(p2 == p3, "p2 == p3", p2.x, " ",p2.y, " ",p3.x, " ", p3.y);
		
		
		(void)p1;
		(void)p2;
		(void)p3;
		
		
		debugText.updateText();
		bn::core::update();
	}
}