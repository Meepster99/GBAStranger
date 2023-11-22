#pragma once 

#include "SharedTypes.h"
#include "Tile.h"

class Game;
class EffectsManager;
class EntityManager;
class CutsceneManager;

class Floor : public Layer {
public: 

	// Floor overrides the draw method. 

	Floor() :
	Layer(bn::regular_bg_tiles_items::dw_customfloortiles, 2) 
	{}
	
	void draw(u8 (&collisionMap)[14][9]) override { (void)collisionMap; BN_ERROR("dont call floordraw with only one param"); }

	void draw(u8 (&collisionMap)[14][9], FloorTile* (&floorMap)[14][9]) {
		
		// THIS COULD, AND SHOULD BE OPTIMIZED INTO ONE LOOP OVER THE THING.
		// also the whole background doesnt need a redraw, only the stuff that changed
		
		for(int x=0; x<14; x++) {
			// we now do this in reverse to preserve the dropoffs
			//for(int y=0; y<9; y++) {
			for(int y=8; y>=0; y--) {
				
				if(floorMap[x][y] != NULL && !floorMap[x][y]->isAlive) {
					delete floorMap[x][y];
					floorMap[x][y] = NULL;
				}
				
				// i pray this doesnt (curse) my performance.
				if(floorMap[x][y] == NULL) {
					FloorTile::drawPit(x, y);
				} else {
					floorMap[x][y]->draw();
					
					// y < 7 here bc row 8 is for ui, and we dont want dropoffs going there,, do we?

					if(floorMap[x][y]->drawDropOff() && y < 8 && 
					(floorMap[x][y+1] == NULL || floorMap[x][y+1]->isTransparent()) &&
					collisionMap[x][y+1] < 3) {
						//rawMap.setTile(x * 2 + 1, (y + 1) * 2 + 1, 4 * 2); 
						//rawMap.setTile(x * 2 + 2, (y + 1) * 2 + 1, 4 * 2 + 1); 
						//rawMap.setTile(x * 2 + 1, (y + 1) * 2 + 2, 4 * 2 + 2); 
						//rawMap.setTile(x * 2 + 2, (y + 1) * 2 + 2, 4 * 2 + 3); 
						FloorTile::drawDropOff(x, y+1);
					}
				}
				
			}
		}
		
		rawMap.reloadCells();
	}

};

class TileManager {
public:

	FloorTile* floorMap[14][9];
	
	// these tiles are getting,, like gods this is just excessive
	Exit* exitTile = NULL;
	RodTile* rodTile = NULL;
	LocustTile* locustTile = NULL;
	WordTile* locustCounterTile = NULL;
	
	WordTile* voidTile1 = NULL;
	WordTile* voidTile2 = NULL;
	
	WordTile* floorTile1 = NULL;
	WordTile* floorTile2 = NULL;
	
	SpriteTile* memoryTile = NULL;
	SpriteTile* wingsTile = NULL;
	SpriteTile* swordTile = NULL;

	Game* game = NULL;
	Floor floorLayer;
	
	EntityManager* entityManager;
	EffectsManager* effectsManager;
	CutsceneManager* cutsceneManager;
	
	
	constexpr static unsigned addBrand[6] = {0b100001, 0b000110, 0b011111, 0b111110, 0b011000, 0b100001};
	constexpr static unsigned eusBrand[6] = {0b110011, 0b001100, 0b110001, 0b111011, 0b110111, 0b110011};
	constexpr static unsigned beeBrand[6] = {0b000001, 0b001100, 0b111001, 0b100111, 0b110011, 0b111001};
	constexpr static unsigned monBrand[6] = {0b100011, 0b011101, 0b110011, 0b011101, 0b100011, 0b111000};
	constexpr static unsigned tanBrand[6] = {0b101101, 0b001100, 0b101101, 0b110011, 0b101101, 0b110011};
	constexpr static unsigned gorBrand[6] = {0b001100, 0b001100, 0b100100, 0b110001, 0b111100, 0b111100};
	constexpr static unsigned levBrand[6] = {0b100011, 0b001111, 0b100100, 0b001100, 0b000001, 0b110011};
	constexpr static unsigned cifBrand[6] = {0b110001, 0b010101, 0b010010, 0b101000, 0b100100, 0b110001};
	
	constexpr static unsigned disBrand[6] = {0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000};

	// unsure if storeing this here is the best option tbh
	unsigned playerBrand[6] = {0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000};
	
	const unsigned* allBrands[10] = {addBrand, eusBrand, beeBrand, monBrand, tanBrand, gorBrand, levBrand, cifBrand, disBrand, playerBrand};
		
	const char* destinations[10] = {"rm_secret_001", "rm_secret_002", "rm_secret_003", "rm_secret_004", "rm_secret_005", "rm_secret_006", "rm_secret_007", "rm_secret_008", "rm_e_intermission", "rm_rm4"};
	
	
	// goofy, but will work
	// should i instead pass the entity* into here so that death tiles can properly kill?
	bn::vector<bn::pair<EntityType, bn::pair<Pos, Pos>>, MAXENTITYSPRITES> floorSteps;
	
	TileManager(Game* game_) : game(game_), 
	floorLayer()
	{
		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				floorMap[x][y] = NULL;
			}
		}
	}
	
	~TileManager();
	
	const char* exitDestination = NULL;
	bn::vector<bn::pair<const char*, Pos>, 8> secretDestinations;

	void loadTiles(u8* floorPointer, SecretHolder* secrets, int secretsCount, const char* exitDest);
		
	void doFloorSteps();
	
	void updateTile(const Pos& p);
	void updateExit();
	void updateRod();
	void updateLocust();
	void updateVoidTiles();
	void updateBurdenTiles();
	
	int getLocustCount();
	int getRoomIndex();
	
	void fullDraw();
	
	bn::optional<TileType> hasFloor(const u8& x, const u8& y);
	bn::optional<TileType> hasFloor(const Pos& p) { return hasFloor(p.x, p.y); } // i really enjoy this use of optional here.
	
	void stepOff(Pos p);
	void stepOn(Pos p);
	
	const char* checkBrand();
	int checkBrandIndex(const unsigned (&testBrand)[6]);
	
	bool hasCollision(const Pos& p);
	
	bool exitRoom(); // just in here as a temporary measure
	bool enterRoom(); // just in here as a temporary measure
	void doVBlank();
};

