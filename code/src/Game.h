#pragma once 

#include "SharedTypes.h"

#include "Tile.h"
#include "RoomManager.h"
#include "EntityManager.h"

#include "DebugText.h"



class BackgroundMap {
public:

	bn::regular_bg_map_cell cells[32 * 32];
	bn::regular_bg_map_item mapItem;
	bn::regular_bg_item bg_item;
	bn::regular_bg_ptr bgPointer;
	bn::regular_bg_map_ptr bgMap;

	static Palette* backgroundPalette;
	
	// bn::bg_palette_item palette
	// how in tarnation can butano look you in the face, and say "yea do this for a background map"
	BackgroundMap(bn::regular_bg_tiles_item tileset, int zIndex) :
		mapItem(cells[0], bn::size(32, 32)),
		
		bg_item(tileset, 
		
		backgroundPalette->getBGPalette()
		
		,mapItem),
		bgPointer(bg_item.create_bg(8, 48)),
		bgMap(bgPointer.map())
		{
			
			// for all tilesets from the game, tile 0 
			// is just a checkerboard. if possible, i rlly, and i mean really, 
			// should overwrite that with transparent here.
			// actually, fuck it, ill just do it in preprocessing.
			
			
			bgPointer.set_z_order(zIndex);
		}
	
	void setTile(int x, int y, int tileIndex) {
		
		bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
		bn::regular_bg_map_cell_info current_cell_info(current_cell);

		current_cell_info.set_tile_index(tileIndex);
		current_cell = current_cell_info.cell(); 
	
		// if lag happens to occur, we could not update this until a frame is done
		// yep, lag occured.
		// sorta fixed it, but to be safe, im ballin here
		//bgMap.reload_cells_ref();
	}
	
	void reloadCells() {
		bgMap.reload_cells_ref();
	}
	
};

class Layer {
public:

	BackgroundMap rawMap;

	// dont ask
	Layer(bn::regular_bg_tiles_item tileset, int zIndex, int fillIndex = 0) : rawMap(tileset, zIndex)
		{
			

			//setup black border., just black the whole screen
			for(int i=0; i<30; i++) {
				for(int j=0; j<20; j++) {
			//for(int i=0; i<32; i++) {
			//	for(int j=0; j<32; j++) {
					rawMap.setTile(i, j, fillIndex);
				}
			}
			rawMap.reloadCells();
		}
	virtual ~Layer() = default;
	
	virtual void draw(u8 (&gameMap)[14][9]) {

		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				
				u8 tile = gameMap[x][y];
			
				rawMap.setTile(x * 2 + 1, y * 2 + 1, 4 * tile); 
				rawMap.setTile(x * 2 + 2, y * 2 + 1, 4 * tile + 1); 
				rawMap.setTile(x * 2 + 1, y * 2 + 2, 4 * tile + 2); 
				rawMap.setTile(x * 2 + 2, y * 2 + 2, 4 * tile + 3); 
			}
		}
		
		rawMap.reloadCells();
	}
	
};

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
	
	DebugText debugText;
	
	int miscDebug = 0; // just here for when i want to disp something on the screen
	
	Game() : collision(bn::regular_bg_tiles_items::dw_tile_bg_1, 2),
	details(bn::regular_bg_tiles_items::dw_tile_edges, 1),
	floor(bn::regular_bg_tiles_items::dw_customfloortiles, 0),
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

