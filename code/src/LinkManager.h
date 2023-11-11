#pragma once 

#include "SharedTypes.h"

class Game;
class EntityManager;
class TileManager;
class EffectsManager;

class LinkManager {
public:

	EntityManager* entityManager;
	EffectsManager* effectsManager;
	TileManager* tileManager;
	Game* game;
	
	int ID = -1;
	int playerCount = -1;

	LinkManager(Game* game_) : game(game_) {}
	
	bn::vector<Packet, 256> allData;

	void sendPacket(const Packet packet);
	
	bn::optional<Packet> recvPacket();

	void backupState();

	void sendState();
	
	void recvState();
	
	void init();
	
private:
	
	static unsigned short recvShort();

};





