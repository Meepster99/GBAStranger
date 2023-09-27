#pragma once 

#include "SharedTypes.h"

class Game;

class DebugText {
public:
	
	Game* game;
	
	bn::sprite_text_generator debugText;
	
	bn::string<MAXDEBUGSPRITES> debugString;
	
	bn::ostringstream stringStream;

	bn::vector<bn::sprite_ptr, MAXDEBUGSPRITES> text_sprites;

	DebugText(Game* game_) : game(game_),
		debugText(common::fixed_8x8_sprite_font),
		debugString(bn::string<MAXDEBUGSPRITES>(MAXDEBUGSPRITES, ' ')),
		stringStream(debugString)
		{
		debugText.set_one_sprite_per_character(true);	
	
	}
	
	void updateText();
	
	
};
