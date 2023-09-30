#pragma once 

#include "SharedTypes.h"
#include "Tile.h"

class Game;
class EffectsManager;
class EntityManager;

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

class TileManager {
public:

	FloorTile* floorMap[14][9];

	Game* game = NULL;
	Floor floorLayer;
	
	EntityManager* entityManager;
	EffectsManager* effectsManager;
	
	// goofy, but will work
	bn::vector<bn::pair<EntityType, bn::pair<Pos, Pos>>, MAXENTITYSPRITES> floorSteps;
	
	TileManager(Game* game_) : game(game_), 
	floorLayer(bn::regular_bg_tiles_items::dw_customfloortiles, 3)
	{
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				floorMap[x][y] = NULL;
			}
		}
		
	}

	void loadTiles(TileType* floorPointer);
		
	void doFloorSteps();
	
	void fullDraw();
	
	bn::optional<TileType> hasFloor(Pos p); // i really enjoy this use of optional here.
	
	void stepOff(Pos p);
	void stepOn(Pos p);
	
	bool exitRoom(); // just in here as a temporary measure
	bool enterRoom(); // just in here as a temporary measure
	void doVBlank();
};

