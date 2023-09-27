

#include "EntityManager.h"

#include "Game.h"
#include "TileManager.h"
#include "EffectsManager.h"

void EntityManager::loadEntities(EntityHolder* entitiesPointer, int entitiesCount) {
	
	// delete old data 
	// conspiricy time, NULL EVERYTHING.
	
	shadowQueue.clear();
	
	if(player != NULL) {
		delete player->rod;
		player->rod = NULL;
	}
	
	for(auto it = deadList.begin(); it != deadList.end(); ++it) {
		if(*it != NULL) { // sanity check
			delete *it;
		}
		*it = NULL;
	}
	
	for(auto it = entityList.begin(); it != entityList.end(); ++it) {
		if(*it != NULL) { // sanity check
			delete *it;
		}
		*it = NULL;
	}
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y].clear();
			futureEntityMap[x][y].clear();
		}
	}

	entityList.clear();
	enemyList.clear();
	obstacleList.clear();
	shadowList.clear();
	
	deadList.clear();
	
	
	player = NULL;
	
	for(int i=0; i<entitiesCount; i++) {
		
		EntityHolder temp = entitiesPointer[i];
		Pos tempPos(temp.x, temp.y);

		switch(temp.t) {
			case EntityType::Player:
			
				BN_ASSERT(player == NULL, "tried to load in a player when player wasnt NULL in room ", game->roomManager.currentRoomName());
			
				player = new Player(tempPos);
				entityList.insert(player);
				break;
			case EntityType::Leech:
				entityList.insert(new Leech(tempPos));
				break;
			case EntityType::Maggot:
				entityList.insert(new Maggot(tempPos));
				break;
			case EntityType::Eye:
				entityList.insert(new Eye(tempPos));
				break;
			case EntityType::Bull:
				entityList.insert(new Bull(tempPos));
				break;
			case EntityType::Chester:
				entityList.insert(new Chester(tempPos));
				break;
			case EntityType::WhiteMimic:
				entityList.insert(new WhiteMimic(tempPos));
				break;
			case EntityType::GrayMimic:
				entityList.insert(new GrayMimic(tempPos));
				break;
			case EntityType::BlackMimic:
				entityList.insert(new BlackMimic(tempPos));
				break;
			case EntityType::Diamond:
				entityList.insert(new Diamond(tempPos));
				break;
			case EntityType::Shadow:
				entityList.insert(new Shadow(tempPos));
				break;
			case EntityType::Boulder:
				entityList.insert(new Boulder(tempPos));
				break;
			case EntityType::Chest:
				entityList.insert(new Chest(tempPos));
				break;
			case EntityType::AddStatue:
				entityList.insert(new AddStatue(tempPos));
				break;
			case EntityType::EusStatue:
				entityList.insert(new EusStatue(tempPos));
				break;
			case EntityType::BeeStatue:
				entityList.insert(new BeeStatue(tempPos));
				break;
			case EntityType::MonStatue:
				entityList.insert(new MonStatue(tempPos));
				break;
			case EntityType::TanStatue:
				entityList.insert(new TanStatue(tempPos));
				break;
			case EntityType::GorStatue:
				entityList.insert(new GorStatue(tempPos));
				break;
			case EntityType::LevStatue:
				entityList.insert(new LevStatue(tempPos));
				break;
			case EntityType::CifStatue:
				entityList.insert(new CifStatue(tempPos));
				break;
			default:
				BN_ERROR("unknown entity tried to get loaded in, wtf");
				break;
		}
		
	}
	
	BN_ASSERT(entityList.size() == entitiesCount, "why does the entitylist size not equal entity count?? size=",entityList.size(), " count=",entitiesCount);
	
	BN_ASSERT(player != NULL, "finished loading in all sprites, but a player wasnt found");
	
	// create starting map.
	for(auto it = entityList.begin(); it != entityList.end(); ++it) {
		
		Entity* temp = *it;

		// set the entityManager pointer, could be done in constructor, but the constr is already complex enough
		(*it)->entityManager = this;
		(*it)->effectsManager = effectsManager;
		(*it)->game = game;
		
		SaneSet<Entity*, 4>& mapRef = getMap(temp->p);
		
		mapRef.insert(temp);

		if(temp->isEnemy()) {
			
			BN_ASSERT(enemyList.size() + 1 != enemyList.maxSize(), "enemyList maxed out, why?");
			
			enemyList.insert(temp);
		} else if(temp->isObstacle()) {
			
			BN_ASSERT(obstacleList.size() + 1 != obstacleList.maxSize(), "obstacleList maxed out, why?");
			
			obstacleList.insert(temp);
		}
		
	}
	
	// TODO determine leech and maggot starting directions here.
	// this could be done with the moveEntity and movefailed funcs, but, no.
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
	
		if((*it)->entityType() != EntityType::Leech && (*it)->entityType() != EntityType::Maggot) {
			continue;
		}
		
		Pos testPos((*it)->p);
		
		Direction testDir;
		Direction resetDir; 
		
		if((*it)->entityType() == EntityType::Leech) {
			testDir = Direction::Right;
			resetDir = Direction::Left;
		} else {
			testDir = Direction::Down;
			resetDir = Direction::Up;
		}
		
		if(!testPos.move(testDir)) {
			(*it)->currentDir = resetDir;
		} else if(hasCollision(testPos) || !hasFloor(testPos) || hasEntity(testPos)) {
			(*it)->currentDir = resetDir;
		}
	
	}
		
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			futureEntityMap[x][y] = entityMap[x][y];
		}
	}
	
	updateMap();
}

void EntityManager::updatePalette(Palette* pal) {
	
	for(auto it = entityList.begin(); it != entityList.end(); ++it) {
		(*it)->sprite.spritePointer.set_palette(pal->getSpritePalette());
	}
	
	for(auto it = deadList.begin(); it != deadList.end(); ++it) {
		(*it)->sprite.spritePointer.set_palette(pal->getSpritePalette());
	}
	
	Sprite::spritePalette = pal;
}

// -----

bool EntityManager::moveEntity(Entity* e, bool dontSet) {
	
	bn::optional<Direction> nextMove = e->getNextMove();
	
	if(!nextMove) {
		e->moveFailed();
		return false;
	}
	
	Direction move = nextMove.value();
	
	Pos testPos(e->p);
	if(!testPos.move(move)) {
		e->moveFailed();
		return false;
	}
	
	if(hasCollision(testPos)) {
		e->moveFailed();
		return false;
	}
	
	if(!e->canFall() && !hasFloor(testPos)) {
		e->moveFailed();
		return false;
	}
	
	if(e->canPush() && hasObstacle(testPos)) {
		// if we can push something, update that things bump, and return

		SaneSet<Entity*, 4>& tempMap = getMap(testPos);
		
		BN_ASSERT(tempMap.size() == 1, "in dodirectionmove, tried to push when there were,,, multiple objects???");
		
		Obstacle* toPush = static_cast<Obstacle*>(*(tempMap.begin()));
		
		e->currentDir = move;
		
		toPush->bumpDirections.push_back(move);
		return false;
	}
	
	if(e->isEnemy() && (hasEnemy(testPos) || hasObstacle(testPos))) {
		// enemies cannot move to squares which have an obstacle/enemy
		// EXCEPT FOR SHADOWS?
		e->moveFailed();
		return false;
	}
	
	if(e->isObstacle() && hasObstacle(testPos)) {
		// obstacles cannot move onto other obstacles
		e->moveFailed();
		return false;
	}
	

	if(dontSet) {
		e->moveFailed();
		return true;
	}
	
	BN_ASSERT(futureEntityMap[e->p.x][e->p.y].contains(e), "tryed to move an entity in futuremap that wasnt there? x=", e->p.x, " y=", e->p.y);
	futureEntityMap[e->p.x][e->p.y].erase(e);
	
	//stepOffs.insert(e->p);
	//stepOns.insert(testPos);
	
	bn::pair<EntityType, bn::pair<Pos, Pos>> tempFloorStep(e->entityType(), bn::pair<Pos, Pos>(e->p, testPos));
	
	tileManager->floorSteps.push_back(tempFloorStep);
	
	e->currentDir = move;
	e->p = testPos;

	futureEntityMap[e->p.x][e->p.y].insert(e);
	return true;
}

void EntityManager::moveEntities(bn::vector<Entity*, MAXENTITYSPRITES>::iterator start, bn::vector<Entity*, MAXENTITYSPRITES>::iterator end) {
	for(auto it = start; it != end; ++it) {
		moveEntity(*it);
	}
}

void EntityManager::moveEnemies() {
	moveEntities(enemyList.begin(), enemyList.end());
}

void EntityManager::moveObstacles() {
	moveEntities(obstacleList.begin(), obstacleList.end());
}

bn::optional<Entity*> EntityManager::doMoves() {
	
	// return an entity if we died an need a reset
	bn::optional<Entity*> res;
	
	// insert player's move into its func 
	// IF THE PLAYER CHANGES THE FLOOR, DO IT HERE.
	bn::pair<bool, bn::optional<Direction>> playerRes = player->doInput();
	
	if(!playerRes.first) {
		return res;
	}
	

	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		
		// intsert mimics move into its func 	
		if((*it)->entityType() == EntityType::WhiteMimic ||
			(*it)->entityType() == EntityType::GrayMimic ||
			(*it)->entityType() == EntityType::BlackMimic) {
			// not ideal.
			Mimic* temp = static_cast<Mimic*>(*it);
			temp->nextMove = playerRes.second;
			continue;
		}
		
		// (maybe) insert diamonds move into its func? (MORE NEEDS TO BE DONE ON THIS)
		
		if((*it)->entityType() == EntityType::Diamond) {
			
			Diamond* temp = static_cast<Diamond*>(*it);
			
			temp->nextMove = canPathToPlayer(temp->p);
		}
	}

	
	// (maybe) insert shadows move into its func? (MORE NEEDS TO BE DONE ON THIS)
	
	// do player move.
	Pos playerStart = player->p;
	moveEntity(player);
	/*res = updateMap();
	if(res) {
		return res;
	}
	player->doUpdate();*/
	
	// IF WE ARE ONE TILE AWAY FROM EXIT, AND SHADOWS ARE ON BUTTONS, WE DO NOT LEAVE THE LEVE
	// still tho, calling doFloorSteps will update the shadows, which is needed 
	
	tileManager->doFloorSteps();
	
	// slightly hacky, but works.
	bn::optional<Direction> shadowMove = playerRes.second;
	if(player->p == playerStart) {
		shadowMove.reset();
	}
	manageShadows(shadowMove);
	res = updateMap(); // this call may not be needed, but im not rishing it
	if(res) {
		return res;
	}
	
	// check if player has moved into enemy, if so, die.
	// DO STATUE CHECKS
	// TODO: ABOVE NEEDS IMPLIMENTATION
	
	// check if player is moving into an obstacle
	// move obstacle
	// if that obstacle kills something, remove it from all sublists.
	moveObstacles();
	res = updateMap();
	if(res) {
		return res;
	}
	
	
	// for all mimics
	// check if mimic is moving into obstacle
	// average out the movement of the obstacle
	// then move all obstacles
	// if that obstacle kills something, remove it from all sublists.
	
	// obstacle collision is done via: if (not obstacle) and (not collision), do move
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		if((*it)->entityType() == EntityType::WhiteMimic ||
			(*it)->entityType() == EntityType::GrayMimic ||
			(*it)->entityType() == EntityType::BlackMimic) {
			// i only want the obstacle updates here, so im reseting the pos after
			
			Mimic* temp = static_cast<Mimic*>(*it);
			bn::optional<Direction> tempNextMove = temp->nextMove;
			
			bool mimicResult = moveEntity(temp, true);
			// if the mimics move resulted in a obstacle being pushed, or just in general failed 
			// we DONT want to give it the direction back
			// if true, we do
			if(mimicResult) {
				temp->nextMove = tempNextMove;
			}
			
		}
	}
	res = updateMap();
	if(res) {
		return res;
	}
	

	moveObstacles();
	res = updateMap();
	if(res) {
		return res;
	}
	
	// -----
	
	// duplicate entityMap.
	// old entityMap WILL BE USED FOR COLLISION.
	
	// for enemy in enemyList
	// domove
	// enemy collision is done via: if (not obstacle) and (not enemy) and (not collision), do move
	
	// go through each pos in entityMap, and determine what happens 
	// only one entity, chill 
	// >=2 entities, killall
	// >=1 obstacle, and any entities, this shouldnt happen i believe?
	// also possibly kill the player.
	
	moveEnemies();
	res = updateMap();
	if(res) {
		return res;
	}
	
	
	// DO STATUE CHECKS
	// THIS STILL AINT DONE

	// copy over new entityMap to old, and update all subarrays.
	// delete anything which needs to be killed
	
			
	sanity();
	
	updateScreen();
	
	return res;
}

// -----

bn::vector<Entity*, 4>::iterator EntityManager::killEntity(Entity* e) {
	
	if(e->entityType() == EntityType::Player || e->entityType() == EntityType::Shadow) {
		BN_ERROR("tried to kill either a player or a shadow. thats not a thing you can do fool.");
	}
	
	Pos tempPos = e->p;
	
	bn::vector<Entity*, 4>::iterator res = entityMap[tempPos.x][tempPos.y].erase(e);
	entityList.erase(e);
	obstacleList.erase(e);
	enemyList.erase(e);		

	deadList.insert(e);
	
	return res;
}

void EntityManager::manageShadows(bn::optional<Direction> playerDir) {
	
	// IMPORTANT. SHADOWS WILL ALWAYS SPRING UP FROM THE TILE THEY WERE STEPPED ON FROM
	// HOWEVER, IF THERE IS ANOTHER SHADOW(POSSIBLY ANY ENTITY?) ON SAID TILE 
	// IT WILL WAIT UNTIL CLEAR.
	// ADDITIONALLY, NEW SHADOWS WILL SPAWN AFTER OLD ONES MOVE

	if(!playerDir) {
		return;
	}
	
	Direction moveDir = playerDir.value();
	
	Pos nextPos = player->p;
	
	BN_LOG("moving shadows (",nextPos.x,",",nextPos.y,")");
	
	// i rlly should have a func for this bynow, screw it, now it kinda does
	// i should rlly impliment this in mimic

	BN_ASSERT(nextPos.moveInvert(moveDir, true, true), "somehow, in shadowmanager when reversing a playermove, the move failed??");
	
	BN_LOG("moving shadows (",nextPos.x,",",nextPos.y,")");
	
	for(int i=0; i<shadowList.size(); i++) {
		
		Pos currentPos = shadowList[i]->p;
	
		BN_LOG("moving shadows (",currentPos.x,",",currentPos.y,") (",nextPos.x,",",nextPos.y,")");
		
		BN_ASSERT(currentPos != nextPos, "error finding next shadow move, current and next pos were equal?? i=", i);
	
		int xDif = nextPos.x - currentPos.x;
		int yDif = nextPos.y - currentPos.y;
		
		BN_ASSERT(!(xDif != 0 && yDif != 0), "error finding next shadow move, it was diag?");
		
		
		if(xDif > 0) {
			shadowList[i]->p.move(Direction::Right);
			shadowList[i]->currentDir = Direction::Right;
		} else if(xDif < 0) {
			shadowList[i]->p.move(Direction::Left);
			shadowList[i]->currentDir = Direction::Left;
		} else if(yDif > 0) {
			shadowList[i]->p.move(Direction::Down);
			shadowList[i]->currentDir = Direction::Down;
		} else if(yDif < 0) {
			shadowList[i]->p.move(Direction::Up);
			shadowList[i]->currentDir = Direction::Up;
		}
		
		// dont do moveentity, as shadows are nonstandard!
		//moveEntity(shadowList[i]);
		
		BN_ASSERT(currentPos != shadowList[i]->p, "somehow a shadow move failed?? how?");
		
		BN_ASSERT(futureEntityMap[currentPos.x][currentPos.y].contains(shadowList[i]), "in shadowmanager, tryed to move an entity in futuremap that wasnt there? x=", currentPos.x, " y=", currentPos.y);
		futureEntityMap[currentPos.x][currentPos.y].erase(shadowList[i]);
		
		bn::pair<EntityType, bn::pair<Pos, Pos>> tempFloorStep(shadowList[i]->entityType(), bn::pair<Pos, Pos>(currentPos, shadowList[i]->p));
		tileManager->floorSteps.push_back(tempFloorStep);

		futureEntityMap[shadowList[i]->p.x][shadowList[i]->p.y].insert(shadowList[i]);
		
		nextPos = currentPos;
	}


	// hacky, but needs to be done since movedir like, changes the future, and i need that future as the present
	// but,,, ugh ijust hope this doesnt cause problems down the road
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y] = futureEntityMap[x][y];
		}
	}
	
	for(auto it = shadowQueue.begin(); it != shadowQueue.end(); ) {
		
		Pos queuePos = *it;
		
		if(hasEntity(queuePos)) {
			++it;
			continue;
		}
		
		it = shadowQueue.erase(it);
		
		Shadow* temp = new Shadow(queuePos);
		shadowList.push_back(temp); 
		entityList.insert(temp);

		getMap(queuePos).insert(temp);
		// also insert this into futuremap
		futureEntityMap[queuePos.x][queuePos.y].insert(temp);
	}
	
}

bn::optional<Entity*> EntityManager::updateMap() {

	// return an entity if we died an need a reset

	// update the entity map, and sublists, and also kill enemies if needed
	
	// static to avoid realloc, cleared to prevent double use
	// or,, i could use iterators, tbh ima do that.
	// butanos vector doesnt have a remove by value, save these for delete later
	// honestly after what ive seen, i think i might as well impliment my own remove tbh
	// nvm, finally going to figure out the butano sets.
	//static bn::vector<Entity*, MAXSPRITES> killList;
	
	// CHECK IF PLAYER IS ON EXIT, AND EXIT IS OPEN HERE.
	
	// TODO: THIS CODE SHOULD DO STATUE CHECKS!!!!!!!!!!!!!!!!!!
	
	bn::optional<Entity*> res;
		
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y] = futureEntityMap[x][y];
			futureEntityMap[x][y].clear();
		}
	}
	
	// do floor updates.
	
	res = tileManager->doFloorSteps();
	if(res) {
		return res;
	}
	
	// do the rest.

	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			
			// should probs use a swtitch statement here, maybe?
			
			if(entityMap[x][y].size() == 0) {
				continue;
			}
			
			if(entityMap[x][y].size() == 1) {
				
				Entity* temp = *entityMap[x][y].begin();
				
				if(!hasFloor(Pos(x, y))) {
					if(temp->isPlayer() || temp->entityType() == EntityType::Shadow) {
						res = temp;
					} else {
						killEntity(temp);
					}
				}
				
				continue;
			}
			
			// remove all enemies, keep obstacles
			//for(int i=0; i<entityMap[x][y].size(); i++) {
			for(auto it = entityMap[x][y].begin(); it != entityMap[x][y].end(); ) {
				
				Entity* temp = *it;
				
				if(temp->isPlayer()) {
					
					// grab another(any) one entity in this square to be the death reason
					SaneSet<Entity*, 4> tempMap = entityMap[x][y];
					
					tempMap.erase(temp);
					
					res = *tempMap.begin();
					++it;
				} else if(temp->entityType() == EntityType::Shadow) {
					res = temp;
					++it;
				} else if(temp->isObstacle()) {
					++it;
					continue;
				} else if(temp->isEnemy()) {
					// kill
					killEntity(temp);
					continue;
				} else {
					BN_ERROR("a entity was somehow not a player, obstacle, or enemy. wtf.");
				}
			}
		}
	}

	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			futureEntityMap[x][y] = entityMap[x][y];
		}
	}
	
	return res;
}


// -----

void EntityManager::doDeaths() {
	
	if(deadList.size() == 0) {
		return;
	}
	
	for(auto it = deadList.begin(); it != deadList.end(); ) {
		(*it)->updatePosition();
		bool res = (*it)->fallDeath();
		
		if(res) {
			
			Entity* temp = *it;
			it = deadList.erase(it);
			delete temp;
			
			continue;
		}
		
		++it;
		
	}
	
	
	
}

void EntityManager::updateScreen() {
	
	// update all onscreen sprites
	// NOT EFFICENT.
	
	for(auto it = entityList.begin(); it != entityList.end(); ++it) {
		(*it)->doUpdate();
	}
	
}

void EntityManager::doTicks() {
	
	for(auto it = entityList.begin(); it != entityList.end(); ++it) {
		(*it)->doTick();
	}
	
}	

void EntityManager::fullUpdate() {
	doTicks();
	updateScreen();
}

// -----

bool EntityManager::hasEntity(Pos p) {
		
		SaneSet<Entity*, 4>& temp = getMap(p);
		
		return temp.size() != 0;
	}
	
bool EntityManager::hasEnemy(Pos p) {
	
	SaneSet<Entity*, 4>& temp = getMap(p);
	
	if(temp.size() == 0) {
		return false;
	}
	
	for(auto it = temp.begin(); it != temp.end(); ++it) {
		if((*it)->isEnemy()) {
			return true;
		}
	}
	
	return false;
}

bool EntityManager::hasObstacle(Pos p) {
	
	SaneSet<Entity*, 4>& temp = getMap(p);
	
	if(temp.size() == 0) {
		return false;
	}
	
	for(auto it = temp.begin(); it != temp.end(); ++it) {
		if((*it)->isObstacle()) {
			return true;
		}
	}
	
	return false;
}

bool EntityManager::hasCollision(Pos p) {
	
	u8 temp = game->collisionMap[p.x][p.y];
	
	if(temp == 12 || temp < 3) {
		return false;
	}
	
	return true;
}

bn::optional<TileType> EntityManager::hasFloor(Pos p) {
	return tileManager->hasFloor(p);
}

bn::optional<Direction> EntityManager::canSeePlayer(Pos p) {
	
	Pos playerPos = player->p;
	
	// first, do some inexpensive checks
	if(!(playerPos.x == p.x || playerPos.y == p.y)) {
		return bn::optional<Direction>();
	}
	
	bool checkY = playerPos.x == p.x;
	//bool checkX = playerPos.y == p.y;
	
	int sharedValue = checkY ? p.x : p.y;
	int startValue = checkY ? MIN(playerPos.y, p.y) + 1 : MIN(playerPos.x, p.x) + 1;
	int stopValue = checkY ? MAX(playerPos.y, p.y) - 1 : MAX(playerPos.x, p.x) - 1;

	for(int i=startValue; i<stopValue; i++) {
		
		bool collision = checkY ? hasCollision(Pos(sharedValue, i)) : hasCollision(Pos(i, sharedValue));
		
		if(collision) {
			return bn::optional<Direction>();
		}
	}
	
	if(checkY && playerPos.y < p.y) {
		return Direction::Up;
	} else if(checkY && playerPos.y > p.y) {
		return Direction::Down;
	} else if(!checkY && playerPos.x < p.x) {
		return Direction::Left;
	} else if(!checkY && playerPos.x > p.x) {
		return Direction::Right;
	} else {
		BN_ERROR("something went horribly wrong in the canSeePlayer func");
	}

	return bn::optional<Direction>();
}

bn::optional<Direction> EntityManager::canPathToPlayer(Pos p) {
	
	// oh boy. this is going to be fun.
	
	Pos playerPos = player->p;
	
	BN_ASSERT(playerPos != p, "why in tarnation do you have a diamond and player at the same pos????");
	
	static u8 visited[14][9];
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			visited[x][y] = 0;
		}
	}
	
	// 128 may be overkill, but im paranoid.
	// actually tbh, thats like,,, way overkill 
	// worst case scenario, i get a crash and fix it.
	bn::deque<Pos, 16> queue;
	
	visited[playerPos.x][playerPos.y] = 1;
	
	// this should basically never happen until wings?)
	if(!hasFloor(playerPos)) {
		return bn::optional<Direction>();
	}
	
	queue.push_back(playerPos);
	
	const Direction testDirections[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
	const Direction invertTestDirections[4] = {Direction::Down, Direction::Up, Direction::Right, Direction::Left};
	
	int queueLoops = 0;
	
	while (!queue.empty()) {

		queueLoops++;
		
		if(queueLoops > 1000) {
			BN_ERROR("canPathToPlayer was in a infinite loop");
		}
	
		Pos tempPos = queue.front();
		queue.pop_front();
		
		for(int i=0; i<4; i++) {
			Pos test(tempPos);
			
			if(!test.move(testDirections[i])) {
				continue;
			}
			
			if(visited[test.x][test.y]) {
				continue;
			}
			
			visited[test.x][test.y] = 1;

			if(hasCollision(test) || !hasFloor(test)) {
				continue;
			}

			if(test == p) {
				// found self, return inverted direction to decide movedir
				return invertTestDirections[i];
			}
			
			if(!hasEntity(test)) {
				queue.push_back(test);
				continue;
			}
		
		}
	}

	return bn::optional<Direction>();
}


// -----

void EntityManager::sanity() {
	
	// check that all data structures are holding up
	
	// plus1 is for the player.
	BN_ASSERT(entityList.size() == 1 + enemyList.size() + obstacleList.size() + shadowList.size(), 
		"EntityManager list sizes didnt add up! ",
		entityList.size(), " ", 1, " ", enemyList.size(), " ", obstacleList.size(), " ", shadowList.size());
	
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {	
			if(entityMap[x][y].size() == 0) {
				continue;
			}
			
			for(auto it = entityMap[x][y].begin(); it != entityMap[x][y].end(); ++it) {
				BN_ASSERT((*it)->p == Pos(x, y), "A sprite at ", x, " ", y, " was not updated in the map properly! and instead is at ", (*it)->p.x, " ", (*it)->p.y);
			}
		}
	}
	
	
}


