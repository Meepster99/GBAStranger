#pragma once 

#include "SharedTypes.h"

class Game;
class EntityManager;
class TileManager;
class EffectsManager;


struct ShortReturn {
	unsigned short data[4];
};


class LinkManager {
public:

	EntityManager* entityManager;
	EffectsManager* effectsManager;
	TileManager* tileManager;
	Game* game;
	
	int ID = -1;
	int playerCount = -1;

	LinkManager(Game* game_) : game(game_) {}
	
	bn::vector<Packet, 256> allPackets[4]; // array for each id.

	void sendPacket(const Packet packet);
	
	bool recvPacket();

	void sendState();
	
	void recvState();

private:
	
	ShortReturn recvShort();

};





