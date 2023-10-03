#pragma once 

#include "SharedTypes.h"

class Game;

class DebugText {
public:
	
	Game* game;
	
	//bn::sprite_text_generator debugText;
	
	//bn::string<MAXDEBUGSPRITES> debugString;
	
	//bn::ostringstream stringStream;

	char debugString[MAXDEBUGSPRITES];
	
	bn::vector<bn::sprite_ptr, MAXDEBUGSPRITES> text_sprites;

	DebugText(Game* game_) : game(game_)
		{
		
		memset(debugString, 0, MAXDEBUGSPRITES);
	
		}
	
	void updateText();
	
	
};
