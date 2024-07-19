#pragma once

#include "SharedTypes.h"

#include "Tile.h"
#include "TileManager.h"
#include "RoomManager.h"
#include "EntityManager.h"
#include "EffectsManager.h"
#include "CutsceneManager.h"


extern bn::fixed sinTable[360];


// collision and details should REALLY be on the same layer. this is just legacy bs
class Collision : public Layer {
public:

	// collision uses default everything
	Collision() :
	Layer(bn::regular_bg_tiles_items::dw_default_bg_tiles, 2)
	{}

};

class Details {
public:

	Collision* collisionPointer = NULL;

	int collisionTileCount = 0;

	// this class is now legacy, but im going to reimpliment funcs in here and pass them off to collision
	Details(Collision* collisionPointer_) : collisionPointer(collisionPointer_) {}

	//inline void setBigTile(int x, int y, int tile, bool flipX = false, bool flipY = false) {
	USEIWRAM void setBigTile(int x, int y, int tile, bool flipX = false, bool flipY = false) {
		// flip the highest bit of the tile to get a details tile.
		// grabbing the size here each call may be expensive. i could set it to a int
		collisionPointer->setBigTile(x, y, tile + collisionTileCount, flipX, flipY);
	}

	//inline void setTile(int x, int y, int tileIndex, bool flipX=false, bool flipY=false) {
	USEIWRAM void setTile(int x, int y, int tileIndex, bool flipX=false, bool flipY=false) {
		collisionPointer->rawMap.setTile(x, y, tileIndex + collisionTileCount, flipX, flipY);
	}

	void draw(u8 (&detailsMap)[14][9], u8 (&collisionMap)[14][9]) {
		BN_ERROR("DONT CALL THE ACTUAL DRAW METHOD IN DETAILS");
		(void)detailsMap;
		(void)collisionMap;
	}


};

// ----

struct GameSave {

	// could use a bool, but i dont want to risk junk sram data.
	// 42 will be a custom room
	int isCustomSave = 0;

	u8 locustCount = 0;
	bool isVoided = false;
	int roomIndex = 1;
	int paletteIndex = 0;
	int mode = 0;
	volatile bool hasMemory = false;
	volatile bool hasWings = false;
	volatile bool hasSword = false;

	volatile bool hasRod = false;
	volatile bool hasSuperRod = false;

	unsigned playerBrand[6] = {0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000};

	unsigned eggCount = 0;

	int delay = -1; // this will be the number of ticks,,, to delay. i could use frames, but im worried abt the framecounter overflowing, altho that like,,,, when even does it overflow?

	bool debug = false;

	int col1Save = 0;
	int col2Save = 32767;
	int col3Save = 25368;
	int col4Save = 16912;

	// this variable is very stupid
	// essentially, on boot, for displaying the splash screen, we need a source of initial randomness before any player input
	// however, i like deadass like,,,,, because the title occurs before any randomness, (playerinput)
	// we need this.
	// i could also use rtc, but when i put this on a real cart, i dont want to have to rely on any sort of battery
	// (ill be using fram, instead of sram too)
	unsigned short randomSeed = 0xFFFF;

	uint64_t hash = 0;

	uint64_t getSaveHash();
};

namespace Game {

	BN_DATA_EWRAM extern Collision* collision;
	BN_DATA_EWRAM extern Details* details;
	BN_DATA_EWRAM extern u8 collisionMap[14][9];
	BN_DATA_EWRAM extern u8 detailsMap[14][9];
	BN_DATA_EWRAM extern GameSave saveData;
	BN_DATA_EWRAM extern bn::timer miscTimer;
	BN_DATA_EWRAM extern GameState state;
	BN_DATA_EWRAM extern bool needRedraw;
	BN_DATA_EWRAM extern int mode;
	BN_DATA_EWRAM extern const char* strangerNames[3];
	BN_DATA_EWRAM extern int paletteIndex;
	BN_DATA_EWRAM extern Palette* pal;
	BN_DATA_EWRAM extern int fadePaletteIndex;
	#define MAXSOUNDS 4
	BN_DATA_EWRAM extern SaneSet<const bn::sound_item*, MAXSOUNDS> queuedSounds;
	BN_DATA_EWRAM extern SaneSet<const bn::sound_item*, MAXSOUNDS*2> removedSounds;

	void Game();

	void run();

	void save();
	void load();

	void saveRNG();
	void loadCustomSave();

	void loadTiles();
	void createExitEffects();
	void findNextRoom();
	void loadLevel(bool debug = false);
	void resetRoom(bool debug = false);

	void fullDraw();
	void fullTileDraw();

	void doVBlank();

	void changePalette(int offset);

	void fadePalette(const int index);


	void changeMusic();

	void doButanoUpdate();

	void changeMode(int val);

	const char* getMode();

	// this should maybe be its own file
	void playSound(const bn::sound_item* sound);
	void removeSound(const bn::sound_item* sound);

	void doSoundVBlank();
};
