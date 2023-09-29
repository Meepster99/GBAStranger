#pragma once 

#include "SharedTypes.h"
#include "Tile.h"
#include "Entity.h"
#include "EffectsManager.h"
#include "TileManager.h"

class Game;
class EffectsManager;
class TileManager;

class EntityManager {
public:

	SaneSet<Entity*, 4> entityMap[14][9];
	
	SaneSet<Entity*, 4> futureEntityMap[14][9];
	
	SaneSet<Entity*, MAXENTITYSPRITES> entityList;
	SaneSet<Entity*, MAXENTITYSPRITES> enemyList;
	SaneSet<Entity*, MAXENTITYSPRITES> obstacleList;
	
	bn::vector<Shadow*, MAXENTITYSPRITES> shadowList;
	// i rlly should impliment dynamic resizing for these vecs.
	bn::vector<Pos, 64> shadowQueue;
	
	SaneSet<Entity*, MAXENTITYSPRITES> deadList;

	// 	bn::unordered_set<Entity*, MAXSPRITES, bn::hash<Entity*>, bn::equal_to<Entity*>>
	
	// i could include a unordered map here for each entity type, but 
	// tbh, i dont want to.
	
	Player* player = NULL; // player will be a member of entityList, and included everywhere else
	
	Game* game = NULL;
	EffectsManager* effectsManager = NULL;
	TileManager* tileManager = NULL;

	EntityManager(Game* game_) : game(game_) {}
	
	void loadEntities(EntityHolder* entitiesPointer, int entitiesCount);
	
	void updatePalette(Palette* pal);
	
	bool moveEntity(Entity* e, bool dontSet = false);
	void moveEntities(bn::vector<Entity*, MAXENTITYSPRITES>::iterator start, bn::vector<Entity*, MAXENTITYSPRITES>::iterator end);
	void moveEnemies();
	void moveObstacles();
	
	
	void doMoves();

	void manageShadows(bn::optional<Direction> playerDir);	

	void updateMap();

	void updateScreen();
	
	void doTicks();
	
	void fullUpdate();

	void doDeaths();
	
	bn::vector<Entity*, 4>::iterator killEntity(Entity* e);
	
	SaneSet<Entity*, 4>& getMap(Pos p) { 
		BN_ASSERT(p.sanity(), "point sanity failed in getmap, x=", p.x, " y=", p.y);
		return entityMap[p.x][p.y]; 
	}
	
	bool hasEntity(Pos p);
	bool hasNonPlayerEntity(Pos p);
	bool hasEnemy(Pos p);
	bool hasObstacle(Pos p);
	bool hasCollision(Pos p);
	bn::optional<TileType> hasFloor(Pos p);
	
	bn::optional<Direction> canSeePlayer(Pos p);
	bn::optional<Direction> canPathToPlayer(Pos p);
	bn::optional<Direction> canPathToPlayer(Diamond* e, Pos playerStart);
	
	
	void sanity();
			
	void addKill(bn::optional<Entity*> e) {
		BN_ASSERT(killedPlayer.size() + 1 != killedPlayer.maxSize(), "ran out of entitys for the killplayer vector?");
		if(e.has_value()) {
			killedPlayer.insert(e.value());
		}
	}
	
	bool hasKills() { return killedPlayer.size() != 0; }
	// should i just make killedplayer public? idek anymore
	bool playerWon() { return killedPlayer.contains(NULL); }
	
	bool exitRoom();
	bool enterRoom();
	void doVBlank();
		
private:
	// make a class OBJECT, have entity and floortile extend it, have this be an array of that.
	// or, illl,,,,, ill just have the floor be a special case tbh 
	// 16 is excessive
	SaneSet<Entity*, 4> killedPlayer; 
	
};


