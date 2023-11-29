
#pragma once 

#include "SharedTypes.h"

class EffectsManager;
class TileManager;
class EntityManager;
class Game;

class FloorTile { // default floor.
public:	

	static EffectsManager* effectsManager;
	static TileManager* tileManager;
	//static BackgroundMap* rawMap;
	static EntityManager* entityManager;
	static Game* game;

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
	
	virtual int getTileValue() const { return startIndex + currentTile; }

	// i rlly should try to do this with a const static.
	virtual bool drawDropOff() const { return true; }
	
	virtual bool isTransparent() const { return false; }
	
	virtual void draw();
	
	static void drawPit(int x, int y);
	
	static void drawDropOff(int x, int y);

	virtual ~FloorTile() = default;
	
	// -----
	
	bool isSteppedOn = false;
	
	virtual void isSteppedOnAnimation() { return; }
	
	virtual void stepOn() {
		isSteppedOn = true;
	}
	
	virtual void stepOff() {
		isSteppedOn = false;
	}
	
	// -----
	
	virtual bool shouldExit() { return false; }
	
	virtual TileType tileType() const { return TileType::Floor; }
	
};

class Glass : public FloorTile { // glass
public:

	Glass(Pos p) : FloorTile(p, 1+2, 7) {}
	
	TileType tileType() const override { return TileType::Glass; }
	
	void stepOn() override;
	void stepOff() override;
		
	bool drawDropOff() const override { return false; }
	
	void isSteppedOnAnimation() override;
	
	bool isTransparent() const override { return true; }
	
};

class Bomb : public FloorTile { // ones with a charge before break
public:

	Bomb(Pos p) : FloorTile(p, 1+2+7, 4) {}
	
	TileType tileType() const override { return TileType::Bomb; }
	
	int charge = 0;
	
	int getTileValue() const override {
		if(charge == 0) {
			return startIndex + 3; 
		}
		
		return startIndex + 0; 	
	}
	
	void stepOn() override;
	
};

class Death : public FloorTile { // the ones that just kill you if you touch them
public:

	Death(Pos p) : FloorTile(p, 1+2+7+4, 4) {}
	
	TileType tileType() const override { return TileType::Death; }
	
	void stepOn() override;
	
};

class Copy : public FloorTile { // create shadow clones
public:
	
	Copy(Pos p) : FloorTile(p, 1+2+7+4+4, 1) {}
	
	TileType tileType() const override { return TileType::Copy; }
	
	void isSteppedOnAnimation() override;
	
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
	
	void stepOn() override;
	
	void stepOff() override;
	
	TileType tileType() const override { return TileType::Switch; }
	
	void isSteppedOnAnimation() override;
};

class Exit : public FloorTile { // exit, INCLUDE LOCKED AND UNLOCKED VERSIONS
public:

	const char* nextRoom;
	bool locked = false;

	Exit(Pos p, char* nextRoom_ = NULL) : FloorTile(p, 1+2+7+4+4+1, 5), 
	nextRoom(nextRoom_)
	{ 

	}
	
	TileType tileType() const override { return TileType::Exit; }

	int getTileValue() const override {
		if(Switch::pressedCount != Switch::totalCount) {
			return startIndex + 4; 
		}
		return startIndex + 0; 	
	}
	
	bool shouldExit() { return Switch::pressedCount == Switch::totalCount; }
	
	bool isFirstCall = true;
	
	void isSteppedOnAnimation() override;
	
};

class WordTile : public FloorTile { // contains some string. 
public:
	// should i be using dependency injection for the draws here?

	char first = ' ';
	char second = ' ';
	
	WordTile(Pos p, char first_ = ' ', char second_ = ' ') : FloorTile(p, 58, 4),
	first(first_), second(second_) {}
	
	TileType tileType() const override { return TileType::WordTile; }
	
	bool drawDropOff() const override { return true; }
	
	int getTileValue() const override { BN_ERROR("getTileValue should not be called on a wordtile!"); return 0; }

	int getNumber() {
		// return the number that this tile is holding
		if(!(first >= '0' && first <= '9')) {
			return -1;
		}
		
		if(!(second >= '0' && second <= '9')) {
			return -1;
		}
		
		return ((first - '0') * 10) + (second - '0');
	}
	
	void draw() override;
	
};

class RodTile : public FloorTile { // tile displays rod state 
public:

	RodTile(Pos p) : FloorTile(p, 29, 10) {}
	
	bool drawDropOff() const override { return true; }

	TileType tileType() const override { return TileType::RodTile; }

	int getTileValue() const override;

};

class LocustTile : public FloorTile {
public:

	LocustTile(Pos p) : FloorTile(p, 27, 2) {}
	
	bool drawDropOff() const override { return true; }

	TileType tileType() const override { return TileType::LocustTile; }

	int getTileValue() const override;
	
};

class SpriteTile : public FloorTile {
public:

	int (*tileFunc)(void);

	SpriteTile(Pos p, int (*tileFunc_)(void)) : FloorTile(p, 51, 2), tileFunc(tileFunc_) {}
	
	bool drawDropOff() const override { return true; }

	TileType tileType() const override { return TileType::SpriteTile; }
	
	int getTileValue() const override {
		BN_ASSERT(tileFunc != NULL, "spritetile tilefunc was null??????");
		return tileFunc();
	}
	
};

