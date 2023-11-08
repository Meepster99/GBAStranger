#pragma once 

#include "SharedTypes.h"
//#include "Tile.h"
#include "Entity.h"
#include "EntityManager.h"

// i really, REALLY do not like the possibility of using this
// but in order to pass lambda funcs with captures as func pointers, its needed.
#include <functional>

class Game;
class EntityManager;
class TileManager;

// while effects will be entities, they may also involve background tiles. 
// for that reason, we are balling here, instead of going more into entityManager
// additionally, while Effects and entities will share a lot of stuff, i am still 
// going to create a completely seperate effects class.
//,,,, ya know what? im going to plan this shit out ahead of time and actualluse my brain.

/*


what is an effect?:

background art(call the background layer from game.h)
glass(and other tiles) animated breaking.
the zoom in which occurs on end/start of room
gray's staff
statues killing you

(maybe, if i want to) a,,, text box?
(maybe, if i want to) the sweat when gray is falling?
(maybe) DIS projector, and also the DIS background 
(maybe), and i mean maybe. 
cutscenes? 
that will be a whole other class, but like 
gods i,,,, how far am i going to take this shit

gods before this tho i should probs make a floor/Tile manager.
unless i want that func in entitymanager to get huge 
also bc like, then tiles can have effect creators.
ughhh 
ya know what? its only 1am, but im going to goto bed. ive only gotten 6 hours of sleep per night for the past few anytways.
i deserve a break, i made good progress today.

*/

class EffectsLayer : public Layer {
public:


	// using an alloced thingy allows us to modify vram 
	//bn::regular_bg_tiles_ptr tilesPointer = bn::regular_bg_tiles_ptr::allocate(bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref().size(), bn::bpp_mode::BPP_4);
	// 128 is just a guess, bc im thinking that getting the tiles ref was causing shit to go down for some reason??

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
		
		// why is this needed??? is this needed each time i update shit?
		rawMap.bgPointer.set_tiles(tilesPointer_);
		
		reloadCells();
	}
	
	void clear() {
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				setBigTile(x, y, 0);
			}
		}
	}
	

	
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
	
	//void (*createFunc)(Effect*);
	//bool (*animateFunc)(Effect*);
	
	std::function<void(Effect*)> createFunc;
	std::function<bool(Effect*)> animateFunc;

	int animationFrequency = 1;
	
	int graphicsIndex = 0;
	
	const bn::sprite_tiles_item* tiles = NULL;
	
	// having one of these is ok, but if it gets excessive i should switch back over to passing them inside the [] of the lambdas
	int tempCounter = 0;
	int tempCounter2 = 0; // now we are excessive
	
	Effect(std::function<void(Effect*)> create_, std::function<bool(Effect*)> animate_, int animationFrequency_ = 1) : 
	sprite(bn::sprite_tiles_items::dw_spr_gray_w_d), createFunc(create_), animateFunc(animate_), animationFrequency(animationFrequency_)
	{ 
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
	int getY() {
		return y;
	}
	
};

class BigSprite {
public:

	
	// i need to change all sprite arrays to just use 128
	// this func is completley overwritten with way to many params that only come from the effectholder struct sometimes, and are hardcoded other times
	
	// looking back on this from the future, this whole fucking class is scuffed .
	// i rlly should of done 64x64 sprites from the start
	// and tbh this WHOLE class literally only animates tail.
	// i should of just had more specialized tail only stuff
	// much of this func was also written before i figured out i could use lambdas as well
	
	static Game* game;
	static EntityManager* entityManager;
	static EffectsManager* effectsManager;
	static TileManager* tileManager;
	
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
	
	
};

// these are defines instead of typedefs bc typedef doesnt allow for constexpr
//#define EffectTypeArray constexpr bn::pair<const bn::sprite_tiles_item, int> 
//#define EffectType bn::pair<const bn::sprite_tiles_item, int> 
//#define EffectTypeCast bn::span<const bn::pair<const bn::sprite_tiles_item, int>>

class MenuOption {
public:

	static EffectsManager* effectsManager;
	static int yIndex;

	
	// i could do this via an array, but its pretty dumb to do that for things like,, the room list 
	// so we are going with jank
	//bn::vector<const char*, 1024> options;
	

	
	const char* optionName = NULL;
	const char* (*getOption)() = NULL;
	void (*changeOption)(int) = NULL;
	int yDraw = 0;
	int xDraw = 0;
	
	// why can this buffer only be static when in a function??
	char buffer[64];
	bool isActiveState = false;
	
	// gods what am i on 
	// i was previously using the generator in effectsmanager, but like, this is the only way shit is feasable with length resizing
	bn::sprite_text_generator textGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSprites;
	
	
	MenuOption(const char* optionName_, const char* (*getOption_)(), void (*changeOption_)(int), int xVal = -1);
	void fullDraw(bool isActive);
	void draw(bool isActive);
	void draw();
	
};

class EffectsManager {
public:

	Game* game = NULL;
	EntityManager* entityManager = NULL;
	TileManager* tileManager = NULL;
	
	static Palette* spritePalette;
	
	// do these even need to be pointers?
	bn::vector<Effect*, MAXEFFECTSPRITES> effectList;
	SaneSet<Effect*, MAXEFFECTSPRITES> removeEffectsList;
	
	Effect* dialogueEndPointer = NULL;
	
	bn::sprite_text_generator textGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSpritesLine1;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSpritesLine2;
	
	bn::sprite_text_generator verTextGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> verTextSprites;
	
	bn::vector<BigSprite*, 128> bigSprites;

	// the amount of time spent on this is pathetic.
	// WHY, when i declared tilesPointer as a member var in effects layer did this not work?? but this does???
	bn::regular_bg_tiles_ptr tilesPointer = bn::regular_bg_tiles_ptr::allocate(128, bn::bpp_mode::BPP_4);
	EffectsLayer effectsLayer;	


	bn::vector<MenuOption, 16> menuOptions;

	EffectsManager(Game* game_);
	
	~EffectsManager();

	void createEffect(std::function<void(Effect*)> create_, std::function<bool(Effect*)> animate_, int animationFrequency = 1) {
		Effect* e = new Effect(create_, animate_, animationFrequency);
		effectList.push_back(e);
	}
	
	void removeEffect(Effect* effect) {
		// should effectlist be a saneset?
		// actually, ill just add this to a list, and remove it from like,,, the vblankfunc
		removeEffectsList.insert(effect);
	}
	
	// this is going to be where i define my effects, as static vars, to be called with EffectManager::effectname()
	// these things should be static,, but i need them to access the effectList,,, so like, ugh
	// this should maybe have been a namespace, but idk
	

	
	void updatePalette(Palette* pal);
	
	// -----

	bool playerWonLastExit = true;
	
	bool zoomEffect(bool inward, bool autoSpeed = true);
	bool topDownEffect(bool downward);
	
	bool exitRoom();
	bool enterRoom();
	void doVBlank();
	
	void loadEffects(EffectHolder* effects, int effectsCount);
	
	// -----
	
	void hideForDialogueBox(bool vis, bool isCutscene);
	
	// dialogue needs to become its own class.
	// more than it already is
	void doDialogue(const char* data, bool isCutscene = false, const bn::sound_item* sound = &bn::sound_items::snd_voice2);
	void doDialogue(const char* data, const bn::sound_item* sound) {
		doDialogue(data, false, sound);
	}
	
	void setBrandColor(int x, int y, bool isTile);
	void doMenu();
	
	bool restRequest(const char* questionString = "Rest?\0");
	
	
	void glassBreak(Pos p);
	void voidRod(Pos p, Direction dir);
	void wings();
	void explosion(Pos p);
	void sword(Pos p, Direction dir);
	void monLightning(Pos p, Direction dir);
	void sparkle(Pos p, int sparkleLength = 8);
	Effect* getRoomDustEffect(bool isCutscene = false);
	void roomDust();
	void entityKill(Entity* entity);
	void entityFall(Entity* entity);
	void playerBrandRoomBackground();
	Effect* generateSweatEffect();
	Effect* generateDialogueEndpointer();
	
	int questionMarkCount = 0;
	void questionMark();
	
	void treeLeaves();
	
};

class Dialogue {
public:

	EffectsManager* effectsManager;
	const char* originalData;

	const char* data;
	
	Dialogue(EffectsManager* effectsManager_, const char* data_);
	
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


