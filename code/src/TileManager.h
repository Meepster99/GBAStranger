#pragma once 

#include "SharedTypes.h"
#include "Tile.h"

class Game;
class EffectsManager;
class EntityManager;

class Floor : public Layer {
public: 

	// Floor overrides the draw method. 

	Floor() :
	Layer(bn::regular_bg_tiles_items::dw_customfloortiles, 2) 
	{}
	
	void draw(u8 (&collisionMap)[14][9]) override { (void)collisionMap; BN_ERROR("dont call floordraw with only one param"); }

	void draw(u8 (&collisionMap)[14][9], FloorTile* (&floorMap)[14][9]) {
		
		// THIS COULD, AND SHOULD BE OPTIMIZED INTO ONE LOOP OVER THE THING.
		// also the whole background doesnt need a redraw, only the stuff that changed
		
		for(int x=0; x<14; x++) {
			// we now do this in reverse to preserve the dropoffs
			//for(int y=0; y<9; y++) {
			for(int y=8; y>=0; y--) {
				
				if(floorMap[x][y] != NULL && !floorMap[x][y]->isAlive) {
					delete floorMap[x][y];
					floorMap[x][y] = NULL;
				}
				
				// i pray this doesnt fuck my performance.
				if(floorMap[x][y] == NULL) {
					FloorTile::drawPit(x, y);
				} else {
					floorMap[x][y]->draw();
					
					// y < 7 here bc row 8 is for ui, and we dont want dropoffs going there,, do we?

					if(floorMap[x][y]->drawDropOff() && y < 7 && floorMap[x][y+1] == NULL && collisionMap[x][y+1] < 3) {
						rawMap.setTile(x * 2 + 1, (y + 1) * 2 + 1, 4 * 2); 
						rawMap.setTile(x * 2 + 2, (y + 1) * 2 + 1, 4 * 2 + 1); 
						rawMap.setTile(x * 2 + 1, (y + 1) * 2 + 2, 4 * 2 + 2); 
						rawMap.setTile(x * 2 + 2, (y + 1) * 2 + 2, 4 * 2 + 3); 
					}
				}
				
			}
		}
		
		rawMap.reloadCells();
	}

};

class TileManager {
public:

	FloorTile* floorMap[14][9];

	Exit* exitTile = NULL;
	RodTile* rodTile = NULL;
	LocustTile* locustTile = NULL;
	WordTile* locustCounterTile = NULL;
	
	WordTile* voidTile1 = NULL;
	WordTile* voidTile2 = NULL;

	Game* game = NULL;
	Floor floorLayer;
	
	EntityManager* entityManager;
	EffectsManager* effectsManager;
	
	// goofy, but will work
	// should i instead pass the entity* into here so that death tiles can properly kill?
	bn::vector<bn::pair<EntityType, bn::pair<Pos, Pos>>, MAXENTITYSPRITES> floorSteps;
	
	TileManager(Game* game_) : game(game_), 
	floorLayer()
	{
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				floorMap[x][y] = NULL;
			}
		}
		
	}

	void loadTiles(TileType* floorPointer);
		
	void doFloorSteps();
	
	void updateTile(const Pos& p);
	void updateExit();
	void updateRod();
	void updateLocust();
	void updateVoidTiles();
	
	void fullDraw();
	
	bn::optional<TileType> hasFloor(const u8& x, const u8& y);
	bn::optional<TileType> hasFloor(const Pos& p) { return hasFloor(p.x, p.y); } // i really enjoy this use of optional here.
	
	void stepOff(Pos p);
	void stepOn(Pos p);
	
	bool hasCollision(const Pos& p);
	
	bool exitRoom(); // just in here as a temporary measure
	bool enterRoom(); // just in here as a temporary measure
	void doVBlank();
};

