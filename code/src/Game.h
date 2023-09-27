#pragma once 

#include "SharedTypes.h"

#include "Tile.h"
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

class Floor : public Layer {
public: 

	// Floor overrides the draw method. 

	Floor(bn::regular_bg_tiles_item tileset, int zIndex) :
	Layer(tileset, zIndex) 
	{}
	
	u8 tempFloorMap[14][9];
	
	void draw(u8 (&collisionMap)[14][9]) override { (void)collisionMap; BN_ERROR("dont call floordraw with only one param"); }

	void draw(u8 (&collisionMap)[14][9], FloorTile* (&floorMap)[14][9]) {
		
		// THIS COULD, AND SHOULD BE OPTIMIZED INTO ONE LOOP OVER THE THING.
		// also the whole background doesnt need a redraw, only the stuff that changed
		
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				
				if(floorMap[x][y] == NULL) {
					tempFloorMap[x][y] = 0;
					continue;
				}
				
				if(!floorMap[x][y]->isAlive) {
					delete floorMap[x][y];
					floorMap[x][y] = NULL;

					tempFloorMap[x][y] = 0;
					continue;
				}
				
				tempFloorMap[x][y] = floorMap[x][y]->getTileValue();
			}
		}
		
		
		for(int x=0; x<14; x++) {
			for(int y=0; y<8; y++) {
				// 8 instead of 9 bc we are only putting things one layer below.
				
				// this if statement is hella scuffed. need to like,, gods idek 
				// switch,,, is a little less scuffed?
				// actually no 
				
				if(tempFloorMap[x][y] == 2) {
					continue;
				}
				
				if(floorMap[x][y] != NULL && floorMap[x][y]->tileType() == TileType::Glass) {
					continue;
				}
				
				u8 currentTile = tempFloorMap[x][y];
				u8 downTile = tempFloorMap[x][y+1];
				
				if(currentTile >= 1 && downTile == 0 && collisionMap[x][y+1] < 3) {
					tempFloorMap[x][y+1] = 2;
				}
				
			}
		}
		Layer::draw(tempFloorMap);	
	}
};

// ----

class Game {
public:
	
	Collision collision;
	Details details;
	Floor floor;	
	
	u8 collisionMap[14][9];
	u8 detailsMap[14][9];
	FloorTile* floorMap[14][9];
	
	RoomManager roomManager;
	
	EntityManager entityManager;
	
	EffectsManager effectsManager;
	
	DebugText debugText;
	
	bn::timer miscTimer;
	
	int miscDebug = 0; // just here for when i want to disp something on the screen
	
	Game() : collision(bn::regular_bg_tiles_items::dw_tile_bg_1, 3),
	details(bn::regular_bg_tiles_items::dw_tile_edges, 2),
	floor(bn::regular_bg_tiles_items::dw_customfloortiles, 1),
	entityManager(this),
	debugText(this)
	{
		//initing to null, just for sanity.
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				floorMap[x][y] = NULL;	
				collisionMap[x][y] = 0;
				detailsMap[x][y] = 0;
			}
		}
		
	}
	
	void run();
	
	void loadLevel();
	void resetRoom(Entity* reason = NULL, bool debug = false);
	
	void fullDraw();
	
	int paletteIndex = 0;
	void changePalette(int offset);

	

	
	
};

