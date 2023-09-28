#pragma once 

#include "SharedTypes.h"

#include "Tile.h"
#include "TileManager.h"
#include "RoomManager.h"
#include "EntityManager.h"
#include "EffectsManager.h"

#include "DebugText.h"



class Collision : public Layer {
public:

	// collision uses default everything
	Collision(bn::regular_bg_tiles_item tileset, int zIndex) :
	Layer(tileset, zIndex) 
	{}
	
};

class Details : public Layer {
public:

	// details uses default everything
	Details(bn::regular_bg_tiles_item tileset, int zIndex) :
	Layer(tileset, zIndex, 5)  // why 5? i tried a random number and it worked
	{}
	
};

// ----

class Game {
public:
	
	Collision collision;
	Details details;
	
	u8 collisionMap[14][9];
	u8 detailsMap[14][9];
	
	
	RoomManager roomManager;
	
	EntityManager entityManager;
	
	EffectsManager effectsManager;
	
	TileManager tileManager;
	
	DebugText debugText;
	
	bn::timer miscTimer;
	
	GameState state = GameState::Loading;
	
	int miscDebug = 0; // just here for when i want to disp something on the screen
	int miscDebug2 = 0;
	
	
	
	Game() : collision(bn::regular_bg_tiles_items::dw_tile_bg_1, 3),
	details(bn::regular_bg_tiles_items::dw_tile_edges, 2),
	entityManager(this),
	effectsManager(this),
	tileManager(this),
	debugText(this) {
		
		// goofy
		tileManager.entityManager = &entityManager;
		tileManager.effectsManager = &effectsManager;
		Entity::entityManager = &entityManager;
		Entity::effectsManager = &effectsManager;
		Entity::tileManager = &tileManager;
		entityManager.effectsManager = &effectsManager;
		entityManager.tileManager = &tileManager;
		
	}
	
	void run();
	
	void loadLevel();
	void resetRoom(bool debug = false);
	
	void fullDraw();
	
	void doVBlank();
	
	int paletteIndex = 0;
	void changePalette(int offset);

};

