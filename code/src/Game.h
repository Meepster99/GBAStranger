#pragma once 

#include "SharedTypes.h"

#include "Tile.h"
#include "TileManager.h"
#include "RoomManager.h"
#include "EntityManager.h"
#include "EffectsManager.h"
#include "CutsceneManager.h"

extern Game* globalGame;
extern bn::fixed sinTable[360];


// collision and details should REALLY be on the same layer. this is just legacy bs

class Collision : public Layer {
public:

	// collision uses default everything
	Collision() :
	Layer(bn::regular_bg_tiles_items::dw_default_bg_tiles, 3) 
	{}
	
};

class Details {
public:

	Collision* collisionPointer = NULL;

	int collisionTileCount = 0;
	
	// this class is now legacy, but im going to reimpliment funcs in here and pass them off to collision
	Details(Collision* collisionPointer_) : collisionPointer(collisionPointer_) {}
	
	void setBigTile(int x, int y, int tile, bool flipX = false, bool flipY = false) {
		// flip the highest bit of the tile to get a details tile.
		// grabbing the size here each call may be expensive. i could set it to a int
		collisionPointer->setBigTile(x, y, tile + collisionTileCount, flipX, flipY);
	}
	
	void setTile(int x, int y, int tileIndex, bool flipX=false, bool flipY=false) { 
		collisionPointer->rawMap.setTile(x, y, tileIndex + collisionTileCount, flipX, flipY);
	}
	
	void draw(u8 (&detailsMap)[14][9], u8 (&collisionMap)[14][9]) {
		
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				if(collisionMap[x][y] == 0 || collisionMap[x][y] == 1 || collisionMap[x][y] == 2) {					
					u8 tile = detailsMap[x][y];
					if(tile != 0) {
						setTile(x * 2 + 1, y * 2 + 1, 4 * tile + 0);
						setTile(x * 2 + 2, y * 2 + 1, 4 * tile + 1);
						setTile(x * 2 + 1, y * 2 + 2, 4 * tile + 2);
						setTile(x * 2 + 2, y * 2 + 2, 4 * tile + 3);	
					}
				}
			}
		}
		
		collisionPointer->rawMap.reloadCells();
	}
	
	
};

// ----

struct GameSave {
	u8 locustCount = 0;
	bool isVoided = false;
	int roomIndex = 1;
	int paletteIndex = 0;
	int mode = 0;
	bool hasMemory = false;
	bool hasWings = false;
	bool hasSword = false;
	
	bool hasRod = false;
	bool hasSuperRod = false;
	
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
	
	CutsceneManager cutsceneManager;
	
	bn::timer miscTimer;
	
	GameState state = GameState::Loading;
	
	bool needRedraw = false;
	
	int mode = 0;
	const char* strangerNames[3] = {"Gray\0", "Lillie\0", "Cif\0"};

	Game() : collision(),
	details(&collision),
	entityManager(this),
	effectsManager(this),
	tileManager(this),
	cutsceneManager(this)
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
	
		FloorTile::game = this;
		Entity::game = this;
		BigSprite::game = this;
		
		BigSprite::entityManager = &entityManager;
		BigSprite::tileManager = &tileManager;
		BigSprite::effectsManager = &effectsManager;
		
		MenuOption::effectsManager = &effectsManager;
	
		bn::regular_bg_tiles_ptr backgroundTiles = bn::regular_bg_tiles_ptr::allocate(512, bn::bpp_mode::BPP_4); 
		
		collision.rawMap.bgPointer.set_tiles(backgroundTiles);
	
		cutsceneManager.effectsManager = &effectsManager;
		
	}
	
	void run();
	
	void save();
	void load();
	uint64_t getSaveHash();
	
	void loadTiles();
	void loadLevel(bool debug = false);
	void resetRoom(bool debug = false);
	
	void fullDraw();
	void fullTileDraw();
	
	void doVBlank();
	
	int paletteIndex = 0;
	void changePalette(int offset);
	Palette* pal = NULL;
	
	void doButanoUpdate();
	void uncompressData(u8 res[126], u8* input);
	
	void changeMode(int val) {
		mode += val;
		mode = ((mode % 3) + 3) % 3;
		roomManager.setMode(mode);
	}
	
	const char* getMode() {
		return strangerNames[mode];
	}
	
	
	// this should maybe be its own file
	void playSound(const bn::sound_item* sound);
	
};

