

#include "Tile.h"
#include "EffectsManager.h"
#include "EntityManager.h"

EffectsManager* FloorTile::effectsManager = NULL;
TileManager* FloorTile::tileManager = NULL;
BackgroundMap* FloorTile::rawMap = NULL;
EntityManager* FloorTile::entityManager = NULL;

int Switch::pressedCount = 0;
int Switch::totalCount = 0;

// the gods created constexpr related errors to test mankind, and mankind failed.
// i should: be able to oneline this, be able to do it anonymously, and be able to inline it
EffectTypeArray glassAnimation[] = {EffectType(bn::sprite_tiles_items::dw_spr_glassfloor, 8)};
//constexpr bn::span<const bn::pair<const bn::sprite_tiles_item, int>> bruh(glassAnimation);

void Glass::stepOff() {
	if(isAlive) {
		effectsManager->createEffect(tilePos, 
		EffectTypeCast(glassAnimation));
	}
	isAlive = false;
	tileManager->updateTile(tilePos);
}

void Bomb::stepOn() {
	charge++;
	if(charge == 2) {
		isAlive = false;
	}
	tileManager->updateTile(tilePos);
}

void Switch::stepOn() {
	pressedCount++;
	isSteppedOn = true;
	tileManager->updateExit();
}

void Switch::stepOff() {
	pressedCount--;
	isSteppedOn = false;
	tileManager->updateExit();
}

int RodTile::getTileValue() const {

	if(!entityManager->player->hasRod) {
		return startIndex;
	}
	
	FloorTile* rodTile = entityManager->player->rod; 
	
	if(rodTile == NULL) {
		return startIndex + 1;
	}
	
	TileType rodTileType = rodTile->tileType();
	
	// a switch statement with tiletype is what caused dofloorsteps to lag. will this also cause problems?
	switch(rodTileType) {
		case TileType::Death:
			return startIndex + 1 + 1;
			break;
		case TileType::Floor:
			return startIndex + 1 + 2;
			break;
		case TileType::Glass:
			return startIndex + 1 + 3;
			break;
		case TileType::Switch:
			return startIndex + 1 + 4;
			break;
		case TileType::Exit:
			return startIndex + 1 + 5;
			break;
		case TileType::Copy:
			return startIndex + 1 + 6;
			break;
		case TileType::Bomb:
			// program didnt like static casting to a bomb here, so we are doing this
			return startIndex + 1 + 7 + (rodTile->getTileValue() == rodTile->startIndex);
			break;
		default:
			return startIndex + 1 + 9;
			break;	
	}
	
}

int LocustTile::getTileValue() const {
	return startIndex + (entityManager->player->locustCount != 0);	
}
	
	



