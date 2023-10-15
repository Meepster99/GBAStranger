#pragma once 

#include "SharedTypes.h"

#include "Tile.h"
#include "TileManager.h"
#include "RoomManager.h"
#include "EntityManager.h"
#include "EffectsManager.h"


// collision and details should REALLY be on the same layer. this is just legacy bs

class Collision : public Layer {
public:

	// collision uses default everything
	Collision() :
	Layer(bn::regular_bg_tiles_items::dw_tile_bg_1, 2) 
	{}
	
};

class Details : public Layer {
public:

	// details uses default everything
	Details() :
	Layer(bn::regular_bg_tiles_items::dw_tile_edges, 2, 5)  // why 5? i tried a random number and it worked
	{}
	
};

// ----

struct GameSave {
	u8 locustCount = 0;
	bool isVoided = false;
	int roomIndex = 1;
	
	uint64_t hash = 0;
};

class Game {
public:
	
	Collision collision;
	Details details;
	
	u8 collisionMap[14][9];
	u8 detailsMap[14][9];
	
	GameSave saveData;
	
	RoomManager roomManager;
	
	EntityManager entityManager;
	
	// why is this one plural while the rest aint? i should rlly change that
	EffectsManager effectsManager;
	
	TileManager tileManager;
	
	bn::timer miscTimer;
	
	GameState state = GameState::Loading;
	
	int miscDebug = 0; // just here for when i want to disp something on the screen
	int miscDebug2 = 0;

	Game() : collision(),
	details(),
	entityManager(this),
	effectsManager(this),
	tileManager(this)
	{
		
		// goofy
		tileManager.entityManager = &entityManager;
		tileManager.effectsManager = &effectsManager;
		
		Entity::entityManager = &entityManager;
		Entity::effectsManager = &effectsManager;
		Entity::tileManager = &tileManager;
		
		entityManager.effectsManager = &effectsManager;
		entityManager.tileManager = &tileManager;
		effectsManager.tileManager = &tileManager;
		effectsManager.entityManager = &entityManager;
		
		FloorTile::effectsManager = &effectsManager;
		FloorTile::tileManager = &tileManager;
		FloorTile::entityManager = &entityManager;
		FloorTile::rawMap = &(tileManager.floorLayer.rawMap);
		
		Entity::game = this;
		
		BigSprite::game = this;
	}
	
	void run();
	
	void save();
	void load();
	uint64_t getSaveHash();
	
	void loadLevel(bool debug = false);
	void resetRoom(bool debug = false);
	
	void fullDraw();
	void fullTileDraw();
	
	void doVBlank();
	
	int paletteIndex = 0;
	void changePalette(int offset);

	void doButanoUpdate();
	
};

