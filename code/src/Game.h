#pragma once 

#include "SharedTypes.h"

#include "Tile.h"
#include "TileManager.h"
#include "RoomManager.h"
#include "EntityManager.h"
#include "EffectsManager.h"
#include "CutsceneManager.h"
//#include "LinkManager.h"

extern Game* globalGame;
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
	
	//LinkManager linkManager;
	
	bn::timer miscTimer;
	
	GameState state = GameState::Loading;
	
	bool needRedraw = false;
	
	int mode = 0;
	const char* strangerNames[3] = {"Gray\0", "Lillie\0", "Cif\0"};

	Game() : collision(),
	details(&collision),
	entityManager(this),
	effectsManager(this),
	tileManager(this, &collision),
	cutsceneManager(this)
	//linkManager(this)
	{
		
		// goofy
		tileManager.entityManager = &entityManager;
		tileManager.effectsManager = &effectsManager;
		tileManager.cutsceneManager = &cutsceneManager;
		
		Entity::entityManager = &entityManager;
		Entity::effectsManager = &effectsManager;
		Entity::tileManager = &tileManager;
		Entity::cutsceneManager = &cutsceneManager;
		
		entityManager.effectsManager = &effectsManager;
		entityManager.tileManager = &tileManager;
		effectsManager.tileManager = &tileManager;
		effectsManager.entityManager = &entityManager;
		
		FloorTile::effectsManager = &effectsManager;
		FloorTile::tileManager = &tileManager;
		FloorTile::entityManager = &entityManager;
		FloorTile::floorLayer = &tileManager.floorLayer;
		//FloorTile::rawMap = &(tileManager.floorLayer.rawMap);
	
		FloorTile::game = this;
		Entity::game = this;
		BigSprite::game = this;
		
		BigSprite::entityManager = &entityManager;
		BigSprite::tileManager = &tileManager;
		BigSprite::effectsManager = &effectsManager;
		
		MenuOption::effectsManager = &effectsManager;
	
		//bn::regular_bg_tiles_ptr backgroundTiles = bn::regular_bg_tiles_ptr::allocate(512, bn::bpp_mode::BPP_4); 
		bn::regular_bg_tiles_ptr backgroundTiles = bn::regular_bg_tiles_ptr::allocate(896, bn::bpp_mode::BPP_4); 
		
		collision.rawMap.bgPointer.set_tiles(backgroundTiles);
	
		cutsceneManager.effectsManager = &effectsManager;
		cutsceneManager.tileManager = &tileManager;
		
		
		//linkManager.entityManager = &entityManager;
		//linkManager.effectsManager = &effectsManager;
		//linkManager.tileManager = &tileManager;		
		
		
	}

	~Game();
	
	void run();
	
	__attribute__((noinline, optimize("O0"))) void save();
	__attribute__((noinline, optimize("O0"))) void load();

	__attribute__((noinline, optimize("O0"))) void saveRNG();
	void loadCustomSave();
	__attribute__((noinline, optimize("O0"))) uint64_t getSaveHash();
	
	// putting this in ewram stopped the glitching between rooms when in debug mode 
	// weird, i didnt think it would help bc of how much loading from rom im doing, and the use of memcpy
	__attribute__((section(".ewram"))) void loadTiles();
	void createExitEffects();
	void findNextRoom();
	void loadLevel(bool debug = false);
	void resetRoom(bool debug = false);
	
	// HOLY SHIT. THIS ACTUALY WORKED. THIS ACTUALLY FIXED THE FRAMEDROP?
	// and now, i am extremely scared of EVER using these things. they seem to cause
	// weird, stupid, unknown bugs 
	//__attribute__((target("arm"), section(".iwram"))) void drawCollisionAndDetails();
	//__attribute__((section(".iwram"))) void drawCollisionAndDetails();
	//__attribute__((section(".ewram"))) void drawCollisionAndDetails();
	
	void fullDraw();
	void fullTileDraw();
	
	// read the top of palette.h for an explination on this bs
	void doVBlank();
	//__attribute__((target("arm"), section(".iwram"))) void doVBlank();
	//__attribute__((section(".iwram"))) void doVBlank();
	//__attribute__((section(".ewram"))) void doVBlank();
	//__attribute__((section(".iwram"))) void doVBlank();
	
	int paletteIndex = 0;
	void changePalette(int offset);
	Palette* pal = NULL;
	void fadePalette(const int index);
	int fadePaletteIndex = -1;

	void changeMusic();
	
	void doButanoUpdate();
	//void uncompressData(u8 res[126], u8* input);
	
	void changeMode(int val) {
		mode += val;
		mode = ((mode % 3) + 3) % 3;
		roomManager.setMode(mode);
	}
	
	const char* getMode() {
		return strangerNames[mode];
	}
	

	// could maybe have a queue to store sounds that could be played on future frames?
	// but in my case, im going to have a SaneSet for sounds already played on this frame 
	// or maybe just a vector 
	// actually, no, SaneSet
	
	#define MAXSOUNDS 4
	
	// this should maybe be its own file
	void playSound(const bn::sound_item* sound);
	void removeSound(const bn::sound_item* sound);
	
	void doSoundVBlank();

	bool goofyahhfirstsave = false;

private:
	SaneSet<const bn::sound_item*, MAXSOUNDS> queuedSounds;
	SaneSet<const bn::sound_item*, MAXSOUNDS*2> removedSounds;
	
};

