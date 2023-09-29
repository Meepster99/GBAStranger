
#include "Entity.h"

#include "EntityManager.h"

#include "Game.h"

// this is the only way i could get this to work. why is this syntax like this??
Palette* Sprite::spritePalette = &defaultPalette;

// default initilization, gets overwritten in game.h
EntityManager* Entity::entityManager = NULL;
EffectsManager* Entity::effectsManager = NULL;
TileManager* Entity::tileManager = NULL;
Game* Entity::game = NULL;

int LevStatue::rodUses = 0;
int LevStatue::totalLev = 0;

// Player

bn::pair<bool, bn::optional<Direction>> Player::doInput() {
	
	// if a direction was pressed, return that (true, dir)
	// if A was pressed, meaning we pick up a tile,,,, then (true, NULL)
	// should the move be invalid(picking up a tile with ENTITY on it, return false, null)
	
	
	if(bn::keypad::a_pressed()) {
		
		Pos tilePos(p);
		
		bool moveRes = tilePos.move(currentDir);
		
		if(!moveRes) {
			return {false, bn::optional<Direction>()};
		}
	
		// if there is a entity in this tile, this is an invalid move.
		
		if(entityManager->hasEntity(tilePos) || entityManager->hasCollision(tilePos)) {
			return {false, bn::optional<Direction>()};
		}
		
		// do the tile swap.
		
		FloorTile* tile = tileManager->floorMap[tilePos.x][tilePos.y];
		
		if(tile == NULL && rod == NULL) { 
			return {false, bn::optional<Direction>()};
		} else if (tile != NULL && rod != NULL) {
			return {false, bn::optional<Direction>()};
		} else if(tile == NULL && rod != NULL) {
			// put tile down 
			tileManager->floorMap[tilePos.x][tilePos.y] = rod;
			rod = NULL;
			entityManager->rodUse();
		} else if(tile != NULL && rod == NULL) {
			// pick tile up
			rod = tileManager->floorMap[tilePos.x][tilePos.y];
			tileManager->floorMap[tilePos.x][tilePos.y] = NULL;
			entityManager->rodUse();
		}

		nextMove = bn::optional<Direction>();
		
		return {true, bn::optional<Direction>()};
	}
	
	if(bn::keypad::down_pressed()) {		
		currentDir = Direction::Down;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::up_pressed()) {
		currentDir = Direction::Up;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::left_pressed()) {
		currentDir = Direction::Left;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::right_pressed()) {
		currentDir = Direction::Right;
		nextMove = bn::optional<Direction>(currentDir);
		return {true, bn::optional<Direction>(currentDir)};
	}
	
	
	nextMove = bn::optional<Direction>();
	return {false, bn::optional<Direction>()};
	
}

bn::optional<Direction> Player::getNextMove() {
	bn::optional<Direction> temp = nextMove;
	nextMove.reset();
	return temp; 
}

// Enemy

bn::optional<Direction> Enemy::getNextMove() {
	return bn::optional<Direction>(currentDir);
}

bn::optional<Direction> Bull::getNextMove() {
	
	if(idle) {
		// perform LOS check
		
		bn::optional<Direction> playerDir = entityManager->canSeePlayer(p);
		
		if(playerDir) {
			currentDir = playerDir.value();
			idle = false;
		} else {
			// if we are still idle, return, if not, kachow
			return bn::optional<Direction>();
		}
	}

	return bn::optional<Direction>(currentDir);
}

void Bull::moveFailed() {
	idle = true;
}

bn::optional<Direction> Chester::getNextMove() {
	// wow. it really is that simple.
	return entityManager->canSeePlayer(p);
}

bn::optional<Direction> Mimic::getNextMove() {
	
	if(!nextMove) {
		return nextMove;
	}
	
	Direction temp = nextMove.value();
	
	nextMove.reset();
	
	if(invertHorizontal) {
		if(temp == Direction::Left) {
			temp = Direction::Right;
		} else if(temp == Direction::Right) {
			temp = Direction::Left;
		}
	}
	
	if(invertVertical) {
		if(temp == Direction::Up) {
			temp = Direction::Down;
		} else if(temp == Direction::Down) {
			temp = Direction::Up;
		}
	}
	
	currentDir = temp;
	
	return temp; 
}
	
bn::optional<Direction> Diamond::getNextMove() {
	
	bn::optional<Direction> temp = nextMove;
	
	idle = !nextMove;
	
	nextMove.reset();

	return temp;
}

// Obstacle

bn::optional<Direction> Obstacle::getNextMove() {
	
	if(bumpDirections.size() == 0) {
		return bn::optional<Direction>();
	}
	
	int tempX = 0;
	int tempY = 0;
	
	for(int i=0; i<bumpDirections.size(); i++) {
		switch (bumpDirections[i]) {
			case Direction::Up:
				tempY -= 1;
				break;
			case Direction::Down:
				tempY += 1;
				break;
			case Direction::Left:
				tempX -= 1;
				break;
			case Direction::Right:
				tempX += 1;
				break;
			default:
				break;
		}
	}
		
	BN_ASSERT(!(tempX != 0 && tempY != 0), "a object was pushed in,, >=2 nonparallel directions???");
	
	bn::optional<Direction> res;
	
	if(tempX > 0) {
		res = Direction::Right;
	} else if(tempX < 0) {
		res = Direction::Left;
	} else if(tempY > 0) {
		res = Direction::Down;
	} else if(tempY < 0) {
		res = Direction::Up;
	} else {
		// push dirs canceled out, do nothing
	}
	
	bumpDirections.clear();
	return bn::optional<Direction>(res);
}

void Obstacle::startFall() {
	
	// copy over the actual sprite time zone into the falldata,
	// just so i dont have to go bs a bunch of code
	// but tbh, i rlly should.
				
	fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(spriteTilesArray[0], 9));
	
}

void EusStatue::startFall() {

	BN_ASSERT(tileManager->floorMap[p.x][p.y] == NULL, "with a eus statue, you tried pushing it onto an area that i hadnt nulled yet(a glass that just broke, or something). im to lazy rn to fix this, but if you see it msg me");

	tileManager->floorMap[p.x][p.y] = new FloorTile();

	Obstacle::startFall();
}

bn::optional<Direction> GorStatue::getNextMove() {
	
	if(startPos == p) {
		return Obstacle::getNextMove();
	}
	
	tileIndex = 1;
	
	bumpDirections.clear();
	return bn::optional<Direction>();
}

bn::optional<Direction> MonStatue::getNextMove() {
	
	if(entityManager->canSeePlayer(p)) {
		animationIndex = 1;
		doUpdate();
		entityManager->addKill(this);
	}

	return bn::optional<Direction>();
}

void LevStatue::startFall() {
	totalLev--;
	if(isActive) {
		entityManager->rodUse();
	}
	if(rodUses >= totalLev) {
		entityManager->addKill(entityManager->player); // ADDING THIS TO A KILL RIGHT HERE MIGHT BE A horrid idea, putting player to be safe
	}
}



