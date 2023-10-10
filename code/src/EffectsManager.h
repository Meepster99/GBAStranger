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

	// details uses default everything
	EffectsLayer() :
	Layer(bn::regular_bg_tiles_items::dw_customeffecttiles, 0) {}

	
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

// these are defines instead of typedefs bc typedef doesnt allow for constexpr
#define EffectTypeArray constexpr bn::pair<const bn::sprite_tiles_item, int> 
#define EffectType bn::pair<const bn::sprite_tiles_item, int> 
#define EffectTypeCast bn::span<const bn::pair<const bn::sprite_tiles_item, int>>


class EffectsManager {
public:

	Game* game = NULL;
	EntityManager* entityManager = NULL;
	TileManager* tileManager = NULL;
	
	static Palette* spritePalette;
	
	EffectsLayer effectsLayer;
	
	// do these even need to be pointers?
	bn::vector<Effect*, MAXEFFECTSPRITES> effectList;
	
	bn::sprite_text_generator textGenerator;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSprites;

	EffectsManager(Game* game_) : game(game_), textGenerator(dw_fnt_text_12_sprite_font) {
		
		// may not be the best idea?
		textGenerator.set_one_sprite_per_character(true);
	}

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
	
	void reset();
	
	// -----
	
	void hideForDialogueBox(bool vis);
	void doDialogue(const char* data);
	
	
	
};

