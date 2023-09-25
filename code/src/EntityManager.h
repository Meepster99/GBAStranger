#pragma once 

#include "SharedTypes.h"
#include "Tile.h"
#include "Entity.h"

class Game;

#define MAXSPRITES 128


class EntityManager {
public:

	EntitySet<4> entityMap[14][9];
	
	EntitySet<MAXSPRITES> entityList;
	EntitySet<MAXSPRITES> enemyList;
	EntitySet<MAXSPRITES> obstacleList;
	
	// i could include a unordered map here for each entity type, but 
	// tbh, i dont want to.
	
	Player* player = NULL; // player will be a member of entityList, and included everywhere else
	
	Game* game;

	EntityManager(Game* game_) : game(game_) {}
	
	void loadEntities(EntityHolder* entitiesPointer, int entitiesCount);
	
	void doMove(Direction playerMove);
	
	void updateMap();

	EntitySet<4>& getMap(Pos p) { return entityMap[p.x][p.y]; }

};


