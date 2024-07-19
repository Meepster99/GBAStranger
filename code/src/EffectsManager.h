#pragma once

#include "SharedTypes.h"
//#include "Tile.h"
#include "Entity.h"
#include "EntityManager.h"

// i really, REALLY do not like the possibility of using this
// but in order to pass lambda funcs with captures as func pointers, its needed.
#include <functional>

class EffectsLayer : public Layer {
public:


	// using an alloced thingy allows us to modify vram
	//bn::regular_bg_tiles_ptr tilesPointer = bn::regular_bg_tiles_ptr::allocate(bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref().size(), bn::bpp_mode::BPP_4);
	// 128 is just a guess, bc im thinking that getting the tiles ref was causing (curse) to go down for some reason??

	// this is trash and i shouldnt be calling this alloc twice, but it wont work without it
	//bn::regular_bg_tiles_ptr tilesPointer = bn::regular_bg_tiles_ptr::allocate(1, bn::bpp_mode::BPP_4);



	EffectsLayer(const bn::regular_bg_tiles_item& tilesItem) : Layer(tilesItem, 0, 5) {

	}

	// details uses default everything
	EffectsLayer(bn::regular_bg_tiles_ptr tilesPointer_) : Layer(tilesPointer_, 0, 5)
	 {

		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				setBigTile(x, y, 0);
			}
		}

		// why is this needed??? is this needed each time i update (curse)?
		rawMap.bgPointer.set_tiles(tilesPointer_);

		reloadCells();
	}

	void clear() {
		BN_LOG("effectslayer clear called");
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				setBigTile(x, y, 0);
			}
		}
	}

	void black() {
		BN_LOG("effectslayer black called");
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				setBigTile(x, y, 1);
			}
		}
	}

	void setZoomTile(int x, int y, int t) {


		if(x < 1 || y < 1 || x > 28 || y > 18) {
			return;
		}

		setTile(x, y, t);
	};
};

class Effect {
public:

	// gods this is going to be complex
	// https://www.youtube.com/live/8Gt8BlNLckA?si=ajVBIlBz-OMMvdZC&t=446
	// remember that , and . can go frame by frame'
	// this class desperately needs a rewrite, but im not exactly sure how
	// i could,, always do a custom lambda for the animation func tbh?
	// honestly probs will, i have rlly liked doing that
	// ill try my best to keep the restriction of only having one sprite tho
	// ill have a create lambda func, and a animate func, and if the animate func returns true we destroy this thing.
	// additional variables can be kept as static vars inside the lambda func aswell
	// will i need to pass this to the lambda func?

	// for the void rod:
	// spr_void_rod
	// spr_sparkle

	// for the

	//Pos p;
	// using raw coords would be better now bc some effects arent aligned
	// are these cords going to have origin at center or at top left? im going to go with top left.
	int x;
	int y;
	Sprite sprite;


	bool waitFlag = false; // if true, wait for effect to finish before allowing movement

	//void (*createFunc)(Effect*);
	//bool (*animateFunc)(Effect*);

	std::function<void(Effect*)> createFunc;
	std::function<bool(Effect*)> animateFunc;

	int animationFrequency = 1;

	int graphicsIndex = 0;

	const bn::sprite_tiles_item* tiles = NULL;

	// having one of these is ok, but if it gets excessive i should switch back over to passing them inside the [] of the lambdas
	// i still worry heavily about these variables. are they a good idea over, just passing things into lambdas? i am not sure.
	// i pray this doesnt hurt readability to much
	int tempCounter = 0;
	int tempCounter2 = 0; // now we are excessive

	Effect(std::function<void(Effect*)> create_, std::function<bool(Effect*)> animate_, int animationFrequency_ = 1) :
	sprite(bn::sprite_tiles_items::dw_default_sprite_tiles_transparent), createFunc(create_), animateFunc(animate_), animationFrequency(animationFrequency_)
	{
		// sometimes, effects create NEW effects during vblank.
		// this can cause extremely (messed up) (things)
		// default_sprite_transparent
		// was created to stop all the random grays from appearing. altho they will still technically be there
		sprite.setVisible(false);
	}

	bool firstRun = true;

	bool animate() {

		if(frame % animationFrequency != 0) {
			return false;
		}

		if(firstRun) {
			firstRun = false;
			createFunc(this);
			sprite.setVisible(true);
			return false;
		}

		bool res = animateFunc(this);
		if(res) {
			sprite.setVisible(false);
			//sprite.spritePointer.set_z_order(0);
		}

		return res;
	}

	// used to just see where the pos of this effect is, for the purpose of hiding it for dialogue
	/*Pos getPos() {
		return Pos( (x + 8) / 16, (y + 8) / 16 );
	}*/

};

class BigSprite {
public:


	// i need to change all sprite arrays to just use 128
	// this func is completley overwritten with way to many params that only come from the effectholder struct sometimes, and are hardcoded other times

	// looking back on this from the future, this whole (curse)ing class is scuffed .
	// i rlly should of done 64x64 sprites from the start
	// and tbh this WHOLE class literally only animates tail.
	// i should of just had more specialized tail only stuff
	// much of this func was also written before i figured out i could use lambdas as well

	const int width;
	const int height;
	const bn::sprite_tiles_item* tiles;
	int xPos;
	int yPos;
	const bool collide;
	int optionCount;
	int animationIndex = 0;
	int priority;

	bool autoAnimate;
	int autoAnimateFrames = 32;
	//int (*customAnimate)(void) = NULL;
	std::function<int(void)> customAnimate;

	bool isBigSprite = false;

	bn::vector<Sprite, 128> sprites;


	BigSprite(const bn::sprite_tiles_item* tiles_, int x_, int y_, int width_, int height_, bool collide_, int priority_, bool autoAnimate_);

	void updatePalette(Palette* pal);

	void draw(int index);
	void bigDraw(int index);
	void firstDraw();

	void animate();

	void setVis(bool vis) {
		for(int i=0; i<sprites.size(); i++) {
			sprites[i].spritePointer.set_visible(vis);
		}
	}

	// -----

	void loadBoobTrap();
	void loadTailHead();
	void loadChest();
	void loadTree();
	void loadGorHead();
	void loadStink();

};

class MenuOption {
public:

	static int yIndex;


	// i could do this via an array, but its pretty dumb to do that for things like,, the room list
	// so we are going with jank
	//bn::vector<const char*, 1024> options;



	const char* optionName = NULL;
	//const char* (*getOption)() = NULL;
	//void (*changeOption)(int) = NULL;
	std::function<const char*(void)> getOption;
	std::function<void(int)> changeOption;
	void (*bPress)() = NULL;

	int yDraw = 0;
	int xDraw = 0;

	// why can this buffer only be static when in a function??
	char buffer[64];
	bool isActiveState = false;

	// gods what am i on
	// i was previously using the generator in effectsmanager, but like, this is the only way (curse) is feasable with length resizing
	bn::sprite_text_generator textGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSprites;


	MenuOption(const char* optionName_,
	std::function<const char*(void)> getOption_, std::function<void(int)> changeOption_,
	int xVal = -1);
	void fullDraw(bool isActive);
	void draw(bool isActive);
	void draw();

	void setVisible(bool vis);

};

namespace EffectsManager {

	BN_DATA_EWRAM extern Palette* spritePalette;
	BN_DATA_EWRAM extern bn::vector<Effect*, MAXEFFECTSPRITES> effectList;
	BN_DATA_EWRAM extern SaneSet<Effect*, MAXEFFECTSPRITES> removeEffectsList;
	BN_DATA_EWRAM extern Effect* dialogueEndPointer;
	BN_DATA_EWRAM extern bn::sprite_text_generator textGenerator;
	BN_DATA_EWRAM extern bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSpritesLine1;
	BN_DATA_EWRAM extern bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSpritesLine2;
	BN_DATA_EWRAM extern bn::sprite_text_generator verTextGenerator;
	BN_DATA_EWRAM extern bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> verTextSprites;
	BN_DATA_EWRAM extern bn::vector<BigSprite*, 128> bigSprites;
	BN_DATA_EWRAM extern bn::regular_bg_tiles_ptr tilesPointer;
	BN_DATA_EWRAM extern EffectsLayer effectsLayer;
	BN_DATA_EWRAM extern bn::vector<MenuOption*, 16> menuOptions;
	BN_DATA_EWRAM extern bool playerWonLastExit;
	BN_DATA_EWRAM extern int rodNumber;
	BN_DATA_EWRAM extern bn::vector<Effect*, 16> roomDustTracker;
	BN_DATA_EWRAM extern int questionMarkCount;
	BN_DATA_EWRAM extern int exitGlowCount;
	BN_DATA_EWRAM extern int rotateTanStatuesCount;
	BN_DATA_EWRAM extern int rotateTanStatuesFrames;

	void EffectsManager();

	void createEffect(std::function<void(Effect*)> create_, std::function<bool(Effect*)> animate_);

	void createEffect(std::function<void(Effect*)> create_, std::function<bool(Effect*)> animate_, int animationFrequency);

	void createEffect(std::function<void(Effect*)> create_, std::function<bool(Effect*)> animate_, bool waitFlag);

	void removeEffect(Effect* effect);

	// this is going to be where i define my effects, as static vars, to be called with EffectManager::effectname()
	// these things should be static,, but i need them to access the effectList,,, so like, ugh
	// this should maybe have been a namespace, but idk

	void updatePalette(Palette* pal);

	// -----



	bool zoomEffect(bool inward, bool autoSpeed = true);
	bool topDownEffect(bool downward);

	void setDebugDisplay(bool black = true);
	void setBorderColor(bool black = true);

	//#define EFFECTSMANAGERATTRIBUTES __attribute__((noinline, target("arm"), section(".iwram")))
	//#define EFFECTSMANAGERATTRIBUTES __attribute__((section(".ewram")))
	//#define EFFECTSMANAGERATTRIBUTES __attribute__((section(".iwram")))
	#define EFFECTSMANAGERATTRIBUTES
	EFFECTSMANAGERATTRIBUTES bool exitRoom();
	EFFECTSMANAGERATTRIBUTES bool enterRoom();
	EFFECTSMANAGERATTRIBUTES void doVBlank();

	void loadEffects(EffectHolder* effects, int effectsCount);

	// -----

	void hideForDialogueBox(bool vis, bool isCutscene);

	// dialogue needs to become its own class.
	// more than it already is
	// this being O2, scares me now that i think about it
	// oh gods
	__attribute__((noinline, target("thumb"), optimize("O2"))) void doDialogue(const char* data, bool isCutscene = false, const bn::sound_item* sound = NULL);
	void doDialogue(const char* data, const bn::sound_item* sound);

	void setBrandColor(int x, int y, bool isTile);
	void doMenu();

	void setMenuVis(bool vis);

	__attribute__((noinline, target("thumb"), optimize("O2"))) bool restRequest(const char* questionString = NULL, bool getOption = true);

	void glassBreak(Pos p);
	void voidRod(Pos p, Direction dir);
	void superRodNumber();
	void wings();
	void explosion(Pos p);
	void sword(Pos p, Direction dir);
	void monLightning(Pos p, Direction dir);
	void sparkle(Pos p, int sparkleLength = 8);
	Effect* getRoomDustEffect(bool isCutscene = false);
	void roomDust();
	void entityKill(Entity* entity);
	void entityFall(Entity* entity);
	void entityFall(EntityType t, Pos p);
	void playerBrandRoomBackground();
	Effect* generateSweatEffect(Entity* sweatEntity = NULL);
	Effect* generateDialogueEndpointer();
	void questionMark();
	void treeLeaves();
	void chestBonus(Chest* chest);
	Effect* levStatueActive(LevStatue* levStatue);
	void levKill();
	void fadeBrand();
	void glassShineSpark(const Pos& p);
	void switchGlow(const Pos& p);
	void exitGlow(const Pos& p);
	void copyGlow(const Pos& p);
	void shadowCreate(const Pos& p);
	void shadowDeath(Shadow* shadow);
	void smokeCloud(Pos p, const Direction dir);
	void deathTile(Pos p);
	void deathTileAnimate(Pos p);
	void bombTileAnimate(Pos p);
	void corpseSparks();
	void corpseFuzz();
	void stinkLines(const Pos p);
	void rotateTanStatues();
	void corrupt(int frames = 30);
	void locustGet(bool isFirstLocust);
	void generateSecretSparks(const Pos p);
	void secretSparks(const Pos p);

};

class Dialogue {
public:

	const char* originalData;

	const char* data;

	Dialogue(const char* data_);

	int getNextDialogue(char* res);

private:

	const char* getNextWord();

	int getNextLine();

	bool isWordEnd(char c) {
		return c == ' ' || c == '\n' || c == '\0' || c == '\r' || c == '`';
	}

	bool isLineEnd(char c) {
		return c == '\n' || c == '\0' || c == '\r';
	}

};
