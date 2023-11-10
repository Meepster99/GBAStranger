#pragma once 

#include "SharedTypes.h"

class Game;
class EntityManager;
class TileManager;
class EffectsManager;

class LinkManager {
public:

	bn::optional<bn::link_state> linkStateOpt;
	
	EntityManager* entityManager;
	EffectsManager* effectsManager;
	TileManager* tileManager;
	Game* game;

	LinkManager(Game* game_) : game(game_) {}

	void sendState();
	
	void recvState();

};





