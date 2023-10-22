

#include "Tile.h"
#include "EffectsManager.h"
#include "EntityManager.h"
#include "Game.h"

EffectsManager* FloorTile::effectsManager = NULL;
TileManager* FloorTile::tileManager = NULL;
BackgroundMap* FloorTile::rawMap = NULL;
EntityManager* FloorTile::entityManager = NULL;
Game* FloorTile::game = NULL;

int Switch::pressedCount = 0;
int Switch::totalCount = 0;

// the gods created constexpr related errors to test mankind, and mankind failed.
// i should: be able to oneline this, be able to do it anonymously, and be able to inline it
EffectTypeArray glassAnimation[] = {EffectType(bn::sprite_tiles_items::dw_spr_glassfloor, 8)};
//constexpr bn::span<const bn::pair<const bn::sprite_tiles_item, int>> bruh(glassAnimation);

void Glass::stepOn() {
	if(game->state == GameState::Normal) {
		bn::sound_items::snd_stepglassfloor.play();
	}
}

void Glass::stepOff() {
	if(isAlive) {
		effectsManager->createEffect(tilePos, 
		EffectTypeCast(glassAnimation));
	}
	isAlive = false;
	tileManager->updateTile(tilePos);
	if(game->state == GameState::Normal) {
		bn::sound_items::snd_breakglassfloor.play();
	}
}

void Bomb::stepOn() {
	charge++;
	if(charge == 2) {
		isAlive = false;
		
		// this may be bad 
		// break adjacent bomb tiles,,,, i think?
		Direction testDirections[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
		for(int i=0; i<4; i++) {
			Pos testPos = tilePos;
			if(!testPos.move(testDirections[i])) {
				continue;
			}
			
			if(tileManager->hasFloor(testPos) == TileType::Bomb) {
				Bomb* tempBomb = static_cast<Bomb*>(tileManager->floorMap[testPos.x][testPos.y]);
				if(tempBomb->charge == 1) {
					tempBomb->stepOn();
				}
			}
		}
	}
		
	tileManager->updateTile(tilePos);
}

void Death::stepOn() {
	
	// this needs to call updatemap as well. gods
	/*SaneSet<Entity*, 4>& tempMap = entityManager->getMap(tilePos);
	
	for(auto it = tempMap.begin(); it != tempMap.end(); ++it) {
		if((*it)->entityType() == EntityType::Player) {
			// how can i make it play a death anim?
			entityManager->addKill(*it);
		} else {
			entityManager->killEntity(*it);
		}
	}
	*/
}

void Switch::stepOn() {
	pressedCount++;
	isSteppedOn = true;
	tileManager->updateExit();
	bn::sound_items::snd_activate.play();
}

void Switch::stepOff() {
	pressedCount--;
	isSteppedOn = false;
	tileManager->updateExit();
	bn::sound_items::snd_activate.play();
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
	
	



