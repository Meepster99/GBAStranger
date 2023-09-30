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
	
	u8 tempTileIndicies[4];
	
	void setBigTile(int x, int y, int tile, bool flipX = false, bool flipY = false) {
		
		// this func actually being able to flip shit properly is UNCONFIRMED bc I AM SLEEPY
		
		tempTileIndicies[0] = 4 * tile + ((flipY << 1) | flipX);
		tempTileIndicies[1] = 4 * tile + ((flipY << 1) | !flipX);
		tempTileIndicies[2] = 4 * tile + ((!flipY << 1) | flipX);
		tempTileIndicies[3] = 4 * tile + ((!flipY << 1) | !flipX);
		
		rawMap.setTile(x * 2 + 1, y * 2 + 1, tempTileIndicies[0], flipX, flipY); 
		rawMap.setTile(x * 2 + 2, y * 2 + 1, tempTileIndicies[1], flipX, flipY); 
		rawMap.setTile(x * 2 + 1, y * 2 + 2, tempTileIndicies[2], flipX, flipY); 
		rawMap.setTile(x * 2 + 2, y * 2 + 2, tempTileIndicies[3], flipX, flipY); 	
	}
	
	void update() {
		rawMap.reloadCells();
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

class GlassBreak : public Effect {
public:


	GlassBreak(Pos p_) : Effect(p_) {
		effectData.push_back(bn::pair<bn::sprite_tiles_item, int>(bn::sprite_tiles_items::dw_spr_glassfloor, 8));
	}
	
};

class EffectsManager {
public:

	Game* game = NULL;
	EntityManager* entityManager = NULL;
	TileManager* tileManager = NULL;
	
	EffectsLayer effectsLayer;
	
	// do these even need to be pointers?
	bn::vector<Effect*, 32> effectList;

	EffectsManager(Game* game_) : game(game_) {}

	void newEffect(Effect* e) {
		effectList.push_back(e);
	}

	bool playerWonLastExit = true;
	
	bool zoomEffect(bool inward);
	bool topDownEffect(bool downward);
	
	bool exitRoom();
	bool enterRoom();
	void doVBlank();
	
	
	void reset();
	
};

