#pragma once 

#include "SharedTypes.h"
//#include "Tile.h"
#include "Entity.h"
#include "EntityManager.h"

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

	bn::regular_bg_tiles_ptr tilesPointer;
	
	// details uses default everything
	EffectsLayer(bn::regular_bg_tiles_ptr tilesPointer_) : Layer(tilesPointer_, 0, 5), tilesPointer(tilesPointer_)
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
	

	
};

class Effect {
public:
	
	Pos p;
	Sprite sprite;
	
	bn::vector<bn::pair<bn::sprite_tiles_item, int>, 4> effectData;
	
	Effect(Pos p_) : p(p_), sprite(bn::sprite_tiles_items::dw_spr_gray_w_d)
	{ 
		sprite.setVisible(false);
		sprite.updatePosition(p);
	}
	
	bool firstRun = true;
	
	
	bool animate() {
		// return true once animation done
		
		if(firstRun) {
			sprite.spritePointer.set_tiles(
				effectData[0].first,
				abs(effectData[0].first.graphics_count() - effectData[0].second) % effectData[0].first.graphics_count()
			);
			sprite.setVisible(true);
			firstRun = false;
		}
	
		if(effectData.size() == 0) {
			sprite.setVisible(false);
			return true;
		}

		sprite.spritePointer.set_tiles(
			effectData[0].first,
			abs(effectData[0].first.graphics_count() - effectData[0].second) % effectData[0].first.graphics_count()
		);
		
		effectData[0].second--;
		
		if(effectData[0].second == 0) {
			effectData.erase(effectData.begin());
		}
		
		return false;
	}
	
};

class BigSprite {
public:

	
	// i need to change all sprite arrays to just use 128
	// this func is completley overwritten with way to many params that only come from the effectholder struct sometimes, and are hardcoded other times
	
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
	int (*customAnimate)(void) = NULL;
	
	

	bn::vector<Sprite, 128> sprites;

	
	BigSprite(const bn::sprite_tiles_item* tiles_, int x_, int y_, int width_, int height_, bool collide_, int priority_, bool autoAnimate_);
	
	void updatePalette(Palette* pal);
	
	void draw(int index);
	void firstDraw();
	
	void animate();
	
	
};

// these are defines instead of typedefs bc typedef doesnt allow for constexpr
#define EffectTypeArray constexpr bn::pair<const bn::sprite_tiles_item, int> 
#define EffectType bn::pair<const bn::sprite_tiles_item, int> 
#define EffectTypeCast bn::span<const bn::pair<const bn::sprite_tiles_item, int>>

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
	
	// why can this buffer only be static when in a function??
	char buffer[64];
	bool isActiveState = false;
	
	// gods what am i on 
	// i was previously using the generator in effectsmanager, but like, this is the only way shit is feasable with length resizing
	bn::sprite_text_generator textGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSprites;
	
	
	MenuOption(const char* optionName_, const char* (*getOption_)(), void (*changeOption_)(int));
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
	
	bn::sprite_text_generator textGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSprites;
	
	bn::sprite_text_generator verTextGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> verTextSprites;
	
	bn::vector<BigSprite*, 128> bigSprites;

	// the amount of time spent on this is pathetic.
	// WHY, when i declared tilesPointer as a member var in effects layer did this not work?? but this does???
	bn::regular_bg_tiles_ptr tilesPointer = bn::regular_bg_tiles_ptr::allocate(128, bn::bpp_mode::BPP_4);
	EffectsLayer effectsLayer = EffectsLayer(tilesPointer);	

	bn::vector<MenuOption, 16> menuOptions;

	EffectsManager(Game* game_);

	void createEffect(Pos p, const bn::span<const bn::pair<const bn::sprite_tiles_item, int>>& inputData) {
		// i think using a span here is a good idea,,, but, ugh should i vector?
		// i still cant initialize the type with braces either way bc butanos fucking weird with it
		Effect* e = new Effect(p);
		
		for(int i=0; i<inputData.size(); i++) {
			e->effectData.push_back(inputData[i]);
		}
		
		effectList.push_back(e);
	} 
	
	void updatePalette(Palette* pal);
	
	// -----

	bool playerWonLastExit = true;
	
	bool zoomEffect(bool inward);
	bool topDownEffect(bool downward);
	
	bool exitRoom();
	bool enterRoom();
	void doVBlank();
	
	void loadEffects(EffectHolder* effects, int effectsCount);
	
	// -----
	
	void hideForDialogueBox(bool vis);
	void doDialogue(const char* data);
	
	void doMenu();
	
	
};


