
#include "Entity.h"

#include "EntityManager.h"

#include "Game.h"

// this is the only way i could get this to work. why is this syntax like this??
Palette* Sprite::spritePalette = &defaultPalette;

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
		
		FloorTile* tile = game->floorMap[tilePos.x][tilePos.y];
		
		if(tile == NULL && rod == NULL) { 
			return {false, bn::optional<Direction>()};
		} else if (tile != NULL && rod != NULL) {
			return {false, bn::optional<Direction>()};
		} else if(tile == NULL && rod != NULL) {
			// put tile down 
			game->floorMap[tilePos.x][tilePos.y] = rod;
			rod = NULL;
		} else if(tile != NULL && rod == NULL) {
			// pick tile up
			rod = game->floorMap[tilePos.x][tilePos.y];
			game->floorMap[tilePos.x][tilePos.y] = NULL;
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


