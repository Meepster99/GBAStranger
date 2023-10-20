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
	SaneSet<Obstacle*, MAXENTITYSPRITES> kickedList;

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
	
	SaneSet<Entity*, 4>& getMap(const Pos &p) { 
		//BN_ASSERT(p.sanity(), "point sanity failed in getmap, x=", p.x, " y=", p.y);
		return entityMap[p.x][p.y]; 
	}
	
	const SaneSet<Entity*, 4>& getMap(const Pos &p) const { 
		//BN_ASSERT(p.sanity(), "point sanity failed in getmap, x=", p.x, " y=", p.y);
		return entityMap[p.x][p.y]; 
	}
	
	bool hasEntity(const Pos& p) const;
	bool hasPlayer(const Pos& p) const;
	bool hasNonPlayerEntity(const Pos& p) const;
	bool hasEnemy(const Pos& p) const;
	bool hasObstacle(const Pos& p) const;
	bool hasCollision(const Pos& p) const;
	bn::optional<TileType> hasFloor(const Pos& p) const;
	
	bn::optional<Direction> canSeePlayer(const Pos& p) const;
	bn::optional<Direction> canPathToPlayer(const Pos& p) const;
	bn::optional<Direction> canPathToPlayer(Diamond* e, Pos playerStart);
	
	void rodUse();
	
	void sanity() const;
			
	void addKill(bn::optional<Entity*> e) {
		BN_ASSERT(killedPlayer.size() + 1 != killedPlayer.maxSize(), "ran out of entitys for the killplayer vector?");
		if(e.has_value()) {
			killedPlayer.insert(e.value());
		}
	}
	
	void addEntity(Entity* e);
	
	bool hasKills() const { return killedPlayer.size() != 0; }
	// should i just make killedplayer public? idek anymore
	bool playerWon() const { return killedPlayer.contains(NULL); }
	bool enemyKill() const { return !killedPlayer.contains(player); }
	
	void hideForDialogueBox(bool vis);
	bool exitRoom();
	bool enterRoom();
	void doVBlank();
		
private:
	// make a class OBJECT, have entity and floortile extend it, have this be an array of that.
	// or, illl,,,,, ill just have the floor be a special case tbh 
	// 16 is excessive
	SaneSet<Entity*, 4> killedPlayer; 
	
};


