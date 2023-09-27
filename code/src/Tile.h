
#pragma once 

#include "SharedTypes.h"

class FloorTile { // default floor.
public:	


	// index of where this tiles backgroundmap is 
	const int startIndex = 1;
	
	// how many tiles this tile has 
	const int tileCount = 2; 
	
	bool isAlive = true;
	
	int currentTile = 0; // current tile index.
	
	FloorTile() {}
	FloorTile(int startIndex_, int tileCount_) : startIndex(startIndex_), tileCount(tileCount_) {
		
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

	Glass() : FloorTile(1+2, 7) {}
	
	TileType tileType() override { return TileType::Glass; }
	
	void stepOff() override {
		isAlive = false;
	}
	
};

class Bomb : public FloorTile { // ones with a charge before break
public:

	Bomb() : FloorTile(1+2+7, 4) {}
	
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

	Death() : FloorTile(1+2+7+4, 4) {}
	
	TileType tileType() override { return TileType::Death; }
	
};

class Copy : public FloorTile { // create shadow clones
public:
	
	Copy() : FloorTile(1+2+7+4+4, 1) {}
	
	TileType tileType() override { return TileType::Copy; }
	
};

class Exit : public FloorTile { // exit, INCLUDE LOCKED AND UNLOCKED VERSIONS
public:

	const char* nextRoom;
	bool locked = false;

	Exit(char* nextRoom_ = NULL, bool locked_ = false) : FloorTile(1+2+7+4+4+1, 5), 
	nextRoom(nextRoom_), locked(locked_)
	{ 

	}
	
	TileType tileType() override { return TileType::Exit; }

	int getTileValue() override {
		if(locked) {
			return startIndex + 4; 
		}
		return startIndex + 0; 	
	}
	
	bool shouldExit() { return true; }
	
};

class Switch : public FloorTile { // switch
public:

	Switch() : FloorTile(1+2+7+4+4+1+5, 3) {}
	
	TileType tileType() override { return TileType::Switch; }
};

