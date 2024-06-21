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
	
	bn::deque<Shadow*, MAXENTITYSPRITES> shadowList;
	
	bn::deque<Pos, 128> shadowQueue;
	
	SaneSet<Entity*, MAXENTITYSPRITES> deadList;
	SaneSet<Obstacle*, MAXENTITYSPRITES> kickedList;
	
	SaneSet<Pos, MAXENTITYSPRITES> posTracker;
	
	Pos playerStart = Pos(0, 0); // track starting player position for handling death animations

	Entity* playerPush = NULL;
	
	Player* player = NULL; // player will be a member of entityList, and included everywhere else
	
	Game* game = NULL;
	EffectsManager* effectsManager = NULL;
	TileManager* tileManager = NULL;

	EntityManager(Game* game_) : game(game_) {}
	
	~EntityManager();
	
	USEEWRAM void loadEntities(EntityHolder* entitiesPointer, int entitiesCount);
	inline Interactable* getEmptyChest(Pos p);
	inline AddStatue* getAddStatue(Pos p);
	inline Boulder* getBoulder(Pos p);
	
	bool menuOpened = false;
	
	void updatePalette(Palette* pal);
	
	USEEWRAM bool moveEntity(Entity* e, bool dontSet = false);
	USEEWRAM void moveEntities(bn::vector<Entity*, MAXENTITYSPRITES>::iterator start, bn::vector<Entity*, MAXENTITYSPRITES>::iterator end);
	USEEWRAM void moveEnemies();
	USEEWRAM void moveObstacles();
	
	USEEWRAM void doMoves();

	USEEWRAM void manageShadows(bn::optional<Direction> playerDir);	

	USEEWRAM void updateMap();

	void updateScreen();
	
	
	bool shouldTickPlayer = true;
	void doTicks();
	
	void fullUpdate();

	void doDeaths();
	
	bn::vector<Entity*, 4>::iterator killEntity(Entity* e);
	void removeEntity(Entity* e);
	
	SaneSet<Entity*, 4>& getMap(const Pos &p) { 
		//BN_ASSERT(p.sanity(), "point sanity failed in getmap, x=", p.x, " y=", p.y);
		return entityMap[p.x][p.y]; 
	}
	
	const SaneSet<Entity*, 4>& getMap(const Pos &p) const { 
		//BN_ASSERT(p.sanity(), "point sanity failed in getmap, x=", p.x, " y=", p.y);
		return entityMap[p.x][p.y]; 
	}
	
	USEEWRAM bool hasEntity(const Pos& p) const;
	USEEWRAM bool hasPlayer(const Pos& p) const;
	USEEWRAM bool hasNonPlayerEntity(const Pos& p) const;
	USEEWRAM bool hasEnemy(const Pos& p) const;
	USEEWRAM bool hasObstacle(const Pos& p) const;
	USEEWRAM bool hasCollision(const Pos& p) const;
	USEEWRAM bn::optional<TileType> hasFloor(const Pos& p) const;
	USEEWRAM bool hasNonInteractableObstacle(const Pos& p) const; //nice function name (curse)
	
	USEEWRAM bn::optional<Direction> canSeePlayer(const Pos& p) const;
	USEEWRAM bn::optional<Direction> canPathToPlayer(const Pos& p, const Pos& playerPos) const;
	USEEWRAM bn::optional<Direction> canPathToPlayer(Diamond* e, Pos playerStartPos);
	
	bool levKill = false;
	
	void rodUse();
	
	void sanity() const;
			
	void addKill(bn::optional<Entity*> e) {
		
		// rm_bee_014, the one full of chesters 
		// picking up the floor tile causes this assert to get thrown. 
		// gods,, such a goofy case. 
		// im just going to make it such that the func returns and does nothing 
		
		//BN_ASSERT(killedPlayer.size() + 1 != killedPlayer.maxSize(), "ran out of entitys for the killplayer vector?");
		
		if(killedPlayer.size() + 1 == killedPlayer.maxSize()) {
			return;
		}
		
		if(e.has_value()) {
			killedPlayer.insert(e.value());
		}
	}
	
	void addEntity(Entity* e);
	
	bool hasKills() const { return killedPlayer.size() != 0; }
	
	bool playerWon() const { return killedPlayer.contains(NULL); }
	bool enemyKill() const { return !killedPlayer.contains(player); }
	bool fallKill() const { return killedPlayer.contains(player); }
	bool obstacleKill() const { 
		bool customKill = false;
		for(auto it = killedPlayer.cbegin(); it != killedPlayer.cend(); ++it) {
			if(*it == NULL) {
				continue;
			}
			if((*it)->isObstacle()) {
				customKill = true;
				break;
			}
		}
		return customKill;
	}
	
	bool monKill() const { 
		bool customKill = false;
		for(auto it = killedPlayer.cbegin(); it != killedPlayer.cend(); ++it) {
			if(*it == NULL) {
				continue;
			}
			if((*it)->entityType() == EntityType::MonStatue) {
				customKill = true;
				break;
			}
		}
		return customKill;
	}

	bool needKillAllAddStatues = false;
	void killAllAddStatues();
	
	bool killAtPos(const Pos& p) const { 
		for(auto it = killedPlayer.cbegin(); it != killedPlayer.cend(); ++it) {
			if(*it == NULL) {
				continue;
			}
			if((*it)->entityType() != EntityType::Player && (*it)->p == p) {
				return true;
			}
		}
		return false;
	}
	
	void createKillEffects() const;
	
	void hideForDialogueBox(bool vis, bool isCutscene);
	
	//#define ENTITYMANAGERATTRIBUTES __attribute__((target("arm"), section(".iwram")))
	//#define ENTITYMANAGERATTRIBUTES __attribute__((section(".iwram")))
	//#define ENTITYMANAGERATTRIBUTES __attribute__((section(".ewram")))
	#define ENTITYMANAGERATTRIBUTES
	
	USEEWRAM bool exitRoom();
	USEEWRAM bool enterRoom();
	USEEWRAM void doVBlank();
		
private:
	SaneSet<Entity*, 4> killedPlayer; 
};


