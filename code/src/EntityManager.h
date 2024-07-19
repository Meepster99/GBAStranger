#pragma once

#include "SharedTypes.h"
#include "Tile.h"
#include "Entity.h"
#include "EffectsManager.h"
#include "TileManager.h"

//class Game;
//class EffectsManager;
//class TileManager;

//extern SaneSet<Entity*, 4> entityMap[14][9];
//extern SaneSet<Entity*, 4> futureEntityMap[14][9];


namespace EntityManager {

	BN_DATA_EWRAM extern SaneSet<Entity*, 4> entityMap[14][9];
	BN_DATA_EWRAM extern SaneSet<Entity*, 4> futureEntityMap[14][9];
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> entityList;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> enemyList;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> obstacleList;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> addStatueList;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> tanStatueList;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> levStatueList;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> monStatueList;
	BN_DATA_EWRAM extern bn::deque<Shadow*, MAXENTITYSPRITES> shadowList;
	BN_DATA_EWRAM extern bn::deque<Pos, 128> shadowQueue;
	BN_DATA_EWRAM extern SaneSet<Entity*, MAXENTITYSPRITES> deadList;
	BN_DATA_EWRAM extern SaneSet<Obstacle*, MAXENTITYSPRITES> kickedList;
	BN_DATA_EWRAM extern SaneSet<Pos, MAXENTITYSPRITES> posTracker;
	BN_DATA_EWRAM extern Pos playerStart; // track starting player position for handling death animations
	BN_DATA_EWRAM extern Entity* playerPush;
	BN_DATA_EWRAM extern Player* player; // player will be a member of entityList, and included everywhere else
	BN_DATA_EWRAM extern SaneSet<Entity*, 4> killedPlayer;
	BN_DATA_EWRAM extern bool menuOpened;
	BN_DATA_EWRAM extern bool shouldTickPlayer;
	BN_DATA_EWRAM extern bool levKill;
	BN_DATA_EWRAM extern bool needKillAllAddStatues;

	void loadEntities(EntityHolder* entitiesPointer, int entitiesCount);
	inline Interactable* getEmptyChest(Pos p);
	inline AddStatue* getAddStatue(Pos p);
	inline Boulder* getBoulder(Pos p);

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
	void removeEntity(Entity* e);

	//const SaneSet<Entity*, 4>& getMap(const Pos &p);
	SaneSet<Entity*, 4>& getMap(const Pos &p);

	bool hasEntity(const Pos& p);
	bool hasPlayer(const Pos& p);
	bool hasNonPlayerEntity(const Pos& p);
	bool hasEnemy(const Pos& p);
	bool hasObstacle(const Pos& p);
	bool hasCollision(const Pos& p);
	bn::optional<TileType> hasFloor(const Pos& p);
	bool hasNonInteractableObstacle(const Pos& p); //nice function name (curse)

	bn::optional<Direction> canSeePlayer(const Pos& p);
	bn::optional<Direction> canPathToPlayer(const Pos& p, const Pos& playerPos);
	bn::optional<Direction> canPathToPlayer(Diamond* e, Pos playerStartPos);

	void rodUse();

	void sanity();

	void addKill(bn::optional<Entity*> e);

	void addEntity(Entity* e);

	bool hasKills();
	bool playerWon();
	bool enemyKill();
	bool fallKill();
	bool obstacleKill();

	bool monKill();

	bool killAtPos(const Pos& p);

	void killAllAddStatues();
	void createKillEffects();
	void hideForDialogueBox(bool vis, bool isCutscene);

	bool exitRoom();
	bool enterRoom();
	void doVBlank();
};
