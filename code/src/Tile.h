
#pragma once 

#include "SharedTypes.h"

class EffectsManager;

class FloorTile { // default floor.
public:	

	static EffectsManager* effectsManager;

	Pos tilePos; // im not sure how much i like this, but for tiles to call/create effects, this is needed
	
	// index of where this tiles backgroundmap is 
	const int startIndex = 1;
	
	// how many tiles this tile has 
	const int tileCount = 2; 
	
	bool isAlive = true;
	
	int currentTile = 0; // current tile index.
	
	
	FloorTile(Pos p_) : tilePos(p_) {}
	FloorTile(Pos p_, int startIndex_, int tileCount_) : tilePos(p_), startIndex(startIndex_), tileCount(tileCount_) {
		
	}
	
	virtual int getTileValue() { return startIndex + currentTile; }
	
	virtual ~FloorTile() = default;
	
	// -----
	
	bool isSteppedOn = false;
	
	virtual void stepOn() {
		isSteppedOn = true;
	}
	
	virtual void stepOff() {
		isSteppedOn = false;
	}
	
	// -----
	
	virtual bool shouldExit() { return false; }
	
	virtual TileType tileType() { return TileType::Floor; }
	
};

class Glass : public FloorTile { // glass
public:

	Glass(Pos p) : FloorTile(p, 1+2, 7) {}
	
	TileType tileType() override { return TileType::Glass; }
	
	void stepOff() override;
		
	
};

class Bomb : public FloorTile { // ones with a charge before break
public:

	Bomb(Pos p) : FloorTile(p, 1+2+7, 4) {}
	
	TileType tileType() override { return TileType::Bomb; }
	
	int charge = 0;
	
	int getTileValue() override {
		if(charge == 0) {
			return startIndex + 3; 
		}
		
		return startIndex + 0; 	
	}
	
	void stepOn() override {
		charge++;
		if(charge == 2) {
			isAlive = false;
		}
	}
	
};

class Death : public FloorTile { // the ones that just kill you if you touch them
public:

	Death(Pos p) : FloorTile(p, 1+2+7+4, 4) {}
	
	TileType tileType() override { return TileType::Death; }
	
};

class Copy : public FloorTile { // create shadow clones
public:
	
	Copy(Pos p) : FloorTile(p, 1+2+7+4+4, 1) {}
	
	TileType tileType() override { return TileType::Copy; }
	
};

class Switch : public FloorTile { // switch
public:

	static int pressedCount;
	static int totalCount;
	
	Switch(Pos p) : FloorTile(p, 1+2+7+4+4+1+5, 3) {
		totalCount+=1;
	}
	
	// should this be an override?
	// this was overkill tbh, but im not touching it rn
	~Switch() {
		totalCount--;
		if(isSteppedOn) {
			pressedCount--;
		}
	}
	
	void stepOn() override {
		pressedCount++;
		isSteppedOn = true;
	}
	
	virtual void stepOff() {
		pressedCount--;
		isSteppedOn = false;
	}
	
	TileType tileType() override { return TileType::Switch; }
};

class Exit : public FloorTile { // exit, INCLUDE LOCKED AND UNLOCKED VERSIONS
public:

	const char* nextRoom;
	bool locked = false;

	Exit(Pos p, char* nextRoom_ = NULL) : FloorTile(p, 1+2+7+4+4+1, 5), 
	nextRoom(nextRoom_)
	{ 

	}
	
	TileType tileType() override { return TileType::Exit; }

	int getTileValue() override {
		if(Switch::pressedCount != Switch::totalCount) {
			return startIndex + 4; 
		}
		return startIndex + 0; 	
	}
	
	bool shouldExit() { return Switch::pressedCount == Switch::totalCount; }
	
};


