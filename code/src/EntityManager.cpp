

#include "EntityManager.h"

#include "Game.h"
#include "TileManager.h"
#include "EffectsManager.h"

// should, all managers (and game) be namespaces? it seems so

void EntityManager::loadEntities(EntityHolder* entitiesPointer, int entitiesCount) {

	shouldTickPlayer = true;

	LevStatue::rodUses = 0;
	LevStatue::totalLev = 0;

	// delete old data 
	// conspiricy time, NULL EVERYTHING.
	killedPlayer.clear();
	
	shadowQueue.clear();
	
	kickedList.clear();
	
	// saving is jank as fuck, and this is evidence of it
	//int locustCountBackup = 0;
	//bool isVoidedBackup = false;
	
	if(player != NULL) {
		
		//locustCountBackup = player->locustCount;
		//isVoidedBackup = player->isVoided;
		
		for(int i=0; i<player->rod.size(); i++) {
			delete player->rod[i];
			player->rod[i] = NULL;
		}
		player->rod.clear();
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
	
	SaneSet<Pos, MAXENTITYSPRITES> posSet;
	
	for(int i=0; i<entitiesCount; i++) {
		// i shouldnt have to do this, but despite getting a literal 1:1 memory recreation, it still chose to shit itself 
		// it just didnt want to work.
		// a byte would just get read on a loop for some reason?
		// i cant believe i changed my endianness for this, and padded bytes and shitomfg
		// the numbers getting read in were always repeats of one byte, but idk what it happens
		// so im just doing it manually here
		// and undoing all the endianness changes
		
		//BN_LOG("bruh ", static_cast<int>(entitiesPointer[0].t));
		//BN_LOG("bruh ", entitiesPointer[0].x);
		//BN_LOG("bruh ", entitiesPointer[0].y);
		
		
		EntityType tempType = entitiesPointer[i].t;
		unsigned short tempX = entitiesPointer[i].x;
		unsigned short tempY = entitiesPointer[i].y;
		
		
		
		if((reinterpret_cast<unsigned>(entitiesPointer) & 0x0E000000) == 0x0E000000) {
			
			u8* tempPtr = reinterpret_cast<u8*>(entitiesPointer);
			tempType = static_cast<EntityType>(tempPtr[ (i*3) + 0]);
			tempX = tempPtr[ (i*3) + 1];
			tempY = tempPtr[ (i*3) + 2];
			
		}
		
		EntityHolder temp = {tempType, tempX, tempY};
		
		Pos tempPos(temp.x, temp.y);
		
		// why doesnt this line work omfg
		//BN_ASSERT(!posSet.contains(tempPos), "the entitymangager tried loading in 2 entities on the same position: ", tempPos);
		BN_ASSERT(!posSet.contains(tempPos), "the entitymangager tried loading in 2 entities on the same position: ", tempPos.x, " ", tempPos.y);
		posSet.insert(tempPos);
		
		//BN_ASSERT(hasFloor(tempPos), "tryed loading entity without floor under it????");
		if(hasFloor(tempPos)) {
			tileManager->stepOn(tempPos);
		}
		
		if(i == 0) {
			BN_ASSERT(temp.t == EntityType::Player, "first entity loaded into a room MUST be the player");
		}
		
		switch(temp.t) {
			case EntityType::Player:
			
				BN_ASSERT(player == NULL, "tried to load in a player when player wasnt NULL in room ", game->roomManager.currentRoomName());
			
				player = new Player(tempPos);
				
				// i do not like that we set these vars here!
				player->locustCount = game->saveData.locustCount;
				player->isVoided = game->saveData.isVoided;
				
				player->hasMemory = game->saveData.hasMemory;
				player->hasWings  = game->saveData.hasWings;
				player->hasSword  = game->saveData.hasSword; 
				
				player->hasRod = game->saveData.hasRod; 
				player->hasSuperRod = game->saveData.hasSuperRod; 
				
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
			case EntityType::Mimic:
				BN_ASSERT(player != NULL, "player was null when loading in a mimic"); // player should be the first entity in the entity list from the roomdata, this just just a sanity chek
				
				if(tempPos.x == player->p.x) {
					entityList.insert(new GrayMimic(tempPos));
				} else if(tempPos.y == player->p.y) {
					entityList.insert(new WhiteMimic(tempPos));
				} else {
					entityList.insert(new BlackMimic(tempPos));
				}	
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
			case EntityType::EmptyChest:
				entityList.insert(new Chest(tempPos, true));
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
				BN_ERROR("unknown entity tried to get loaded in, wtf. id=", static_cast<int>(temp.t), " entityCount was ", entitiesCount);
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
	
	/*
	for(auto it = deadList.begin(); it != deadList.end(); ++it) {
		(*it)->sprite.spritePointer.set_palette(pal->getSpritePalette());
	}
	*/
	
	Sprite::spritePalette = pal;
	
}

void EntityManager::addEntity(Entity* e) {
	
	BN_ASSERT(entityMap[e->p.x][e->p.y].size() == 0, "tried adding entity to non zero position");
	BN_ASSERT(futureEntityMap[e->p.x][e->p.y].size() == 0, "tried adding entity to non zero position");
	//BN_ASSERT(hasFloor(e->p) || hasCollision(e->p), "tried adding entity to position without floor or collision");
	
	entityList.insert(e);
	if(e->isObstacle()) {
		obstacleList.insert(e);
	}
	
	if(e->isEnemy()) {
		enemyList.insert(e);
	}
	
	entityMap[e->p.x][e->p.y].insert(e);
	futureEntityMap[e->p.x][e->p.y].insert(e);
	
	
}

EntityManager::~EntityManager() {
	
	killedPlayer.clear();
	
	shadowQueue.clear();
	
	kickedList.clear();
	
	if(player != NULL) {
		for(int i=0; i<player->rod.size(); i++) {
			if(player->rod[i] != NULL) {
				delete player->rod[i];
				player->rod[i] = NULL;
			}
		}
		player->rod.clear();
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
	
	if(e->isEnemy() && testPos == player->p) {
		addKill(e);
	}
	
	if(!e->canFall() && !hasFloor(testPos)) {
		e->moveFailed();
		return false;
	}
	
	if(e->canPush() && hasObstacle(testPos)) {
		// if we can push something, update that things bump, and return

		SaneSet<Entity*, 4>& tempMap = getMap(testPos);
		
		BN_ASSERT(tempMap.size() == 1, "in dodirectionmove, tried to push when there were,,, multiple objects???");
		
		Entity* tempE = *(tempMap.begin());
		BN_ASSERT(tempE->isObstacle(), "something was pushed that wasnt an obstacle. wtf");
		Obstacle* toPush = static_cast<Obstacle*>(tempE);
		
		e->currentDir = move;
		
		toPush->bumpDirections.push_back(move);
		kickedList.insert(toPush);
		return false;
	}
	
	if(e->isEnemy() && (hasEnemy(testPos) || hasObstacle(testPos))) {
		// enemies cannot move to squares which have an obstacle/enemy
		// EXCEPT FOR SHADOWS?
		e->moveFailed();
		return false;
	}
	
	if(e->isObstacle() && (hasObstacle(testPos) || hasPlayer(testPos))) {
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
	
	e->moveSucceded();
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

void EntityManager::doMoves() {
	
	// return an entity if we died an need a reset
	bn::optional<Entity*> res;
	
	
	player->hasWingsTile = player->inRod(tileManager->wingsTile);
	bool prevHasWingsTile = player->hasWingsTile;
	
	// insert player's move into its func 
	// IF THE PLAYER CHANGES THE FLOOR, DO IT HERE.
	bn::pair<bool, bn::optional<Direction>> playerRes = player->doInput();
	
	if(prevHasWingsTile && !player->inRod(tileManager->wingsTile)) {
		player->hasWingsTile = false;
		player->wingsUse = 0;
	}
	
	if(!playerRes.first) {
		return;
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
		
		// diamonds will be inserted AFTER obstacles move.
		// (maybe) insert diamonds move into its func? (MORE NEEDS TO BE DONE ON THIS)
		// nvm we gon be goofy here and do this twice.
		// nvm x2, gods that sounds horrid for efficiency. 
		
	}
	
	// (maybe) insert shadows move into its func? (MORE NEEDS TO BE DONE ON THIS)
	
	// do player move.
	playerStart = player->p;
	bool playerMoved = moveEntity(player);
	//updateMap(); // now that i dont care abt framedrops, 
	
	// telling the difference between if the player failed a move vs is talking to something is gonna be weird
	// actually, i suppose we have the movefailed func, i can just overload that?
	
	
	
	/*res = updateMap();
	if(res) {
		return res;
	}
	player->doUpdate();*/
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y] = futureEntityMap[x][y];
		}
	}
	
	// a small check is needed here tho for if we walked backward into a shadow
	if(playerMoved && getMap(player->p).size() >= 2) {
		SaneSet<Entity*, 4> tempMap = getMap(player->p);
		tempMap.erase(player);
		BN_ASSERT(tempMap.begin() != tempMap.end(), "something has gone horribly wrong with the backwards walk into shadow detection");
		addKill(*tempMap.begin());
		return;
	}
	
	// IF WE ARE ONE TILE AWAY FROM EXIT, AND SHADOWS ARE ON BUTTONS, WE DO NOT LEAVE THE LEVE
	// still tho, calling doFloorSteps will update the shadows, which is needed 
	
	player->pushAnimation = player->p == playerStart;
	if(player->p != playerStart) {
		player->doUpdate(); // previously, the player would update their direction after falling, this fixes that
	} else {
		//,, this will play at the same time as a boulder push,,, but im tired ok
		// we,, we can make ways around this right
		
		Pos temp = playerStart;
		if(playerRes.second.has_value() && temp.move(player->currentDir) && hasNonInteractableObstacle(temp)) {
			game->playSound(&bn::sound_items::snd_push_small);
		}	
	}
	
	// the playermoved bool is basically useless 
	// i think this might mean that you will never die if you,, ugh idek
	if(!bn::keypad::a_pressed()) {
		playerMoveCount++;
	}
	
	tileManager->doFloorSteps();
	
	// ----- the above code was a doUpdate replacement
	
	// slightly hacky, but works.
	bn::optional<Direction> shadowMove = playerRes.second;
	if(player->p == playerStart) {
		shadowMove.reset();
	}
	
	
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		if((*it)->entityType() == EntityType::Diamond) {	
			Diamond* temp = static_cast<Diamond*>(*it);
			// account for the goofy ahh edge case in which a diamond is one move away from 
			// the player, have it move to the players previous position
			// nvm turned out to be less goofy than i thought(no if statements at least)
			if(!hasFloor(player->p)) {
				temp->nextMove = canPathToPlayer(temp, playerStart);
				//BN_LOG("diamond pathing with player start pos ", playerStart, " dir=", temp->nextMove.has_value());
			} else {
				temp->nextMove = canPathToPlayer(temp, player->p);
				//BN_LOG("diamond pathing with player current pos ", player->p, " dir=", temp->nextMove.has_value());
			}
		}
	}

	// TODO when you have a shadow, and go from being onto a shadow tile to falling(with wings) it spawns the default gray sprite, fix that
	manageShadows(shadowMove);
	updateMap(); // this call may not be needed, but im not rishing it
	if(hasKills()) {
		return;
	}
	
	
	// do i even need this var to be a class var
	playerStart = player->p;
	
	// should this be happening here, or on the if(player->p != playerStart) {
	//player->doUpdate();

	// check if player has moved into enemy, if so, die.
	// DO STATUE CHECKS
	// TODO: ABOVE NEEDS IMPLIMENTATION
	
	// check if player is moving into an obstacle
	// move obstacle
	// if that obstacle kills something, remove it from all sublists.
	moveObstacles();
	updateMap();
	if(hasKills()) {
		return;
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
	updateMap();
	if(hasKills()) {
		return;
	}
	

	moveObstacles();
	updateMap();
	if(hasKills()) {
		return;
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
	updateMap();
	if(hasKills()) {
		return;
	}
	
	//player->hasWingsTile = player->inRod(tileManager->wingsTile);
	// DO STATUE CHECKS
	// THIS STILL AINT DONE

	// copy over new entityMap to old, and update all subarrays.
	// delete anything which needs to be killed
	
			
	sanity();
	
	updateScreen();
	
}

// -----

bn::vector<Entity*, 4>::iterator EntityManager::killEntity(Entity* e) {
	
	// this func really needs to be rewritten to just work with killing players tbh, also its return value is scuffed af.
	
	if(e->entityType() == EntityType::Player || e->entityType() == EntityType::Shadow) {
		BN_ERROR("tried to kill either a player or a shadow or an interactable. thats not a thing you can do fool.");
	}
	
	if(e->entityType() == EntityType::Interactable) {
		// hopefully this doesnt fuck anything up
		return entityMap[e->p.x][e->p.y].end();
	}
	
	Pos tempPos = e->p;
	
	/*
	if(tempPos != player->p) {
		// IS THIS OK
		// yes, i think?
		// one thing though is,, enimies deaths NEED to be made into an effect so they can play during a gameover
		e->doUpdate();
	}
	*/
	
	
	
	bn::vector<Entity*, 4>::iterator res = entityMap[tempPos.x][tempPos.y].erase(e);
	entityList.erase(e);
	obstacleList.erase(e);
	enemyList.erase(e);		

	// this was legacy, but is now here just to make sure i dont leak memory in rlly stupid ways
	// and also for EUS STATUES, NOT MODIFYING THE FUCKIN BS IN THE MIDDLE OF A GAMEUPDATE
	deadList.insert(e);
	
	
	// this initially called on all entities, should it?
	// if two entities collide on a tile, the tile should collapse
	/*
	if(tileManager->hasFloor(tempPos) && e->entityType() == EntityType::TanStatue) {
		tileManager->stepOff(tempPos);
	}
	
	if(e->entityType() == EntityType::LevStatue) {
		// this happening here is fucking dumb, and i would prefer it occur with the destructor, but idk
		// actually,,, we have the isdead func, which gets called during vblank. i could call it here? to some potential repercussions
		//i cannot spell
		// tan statue should also have that
		//LevStatue::totalLev--;
	}*/
	
	// this may be horrid
	e->isDead();
	
	if(!hasFloor(tempPos)) {
		effectsManager->entityFall(e);
	} else {
		game->playSound(&bn::sound_items::snd_enemy_explosion);
		e->sprite.setVisible(false);
		effectsManager->explosion(tempPos);
	}
	
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
	
	if(shadowQueue.size() == 0 && shadowList.size() == 0) {
		return;
	}
	
	Direction moveDir = playerDir.value();
	
	Pos nextPos = player->p;
	
	//BN_LOG("moving shadows (",nextPos.x,",",nextPos.y,")");
	
	// i rlly should have a func for this bynow, screw it, now it kinda does
	// i should rlly impliment this in mimic

	BN_ASSERT(nextPos.moveInvert(moveDir, true, true), "somehow, in shadowmanager when reversing a playermove, the move failed??");
	
	//BN_LOG("moving shadows (",nextPos.x,",",nextPos.y,")");
	
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
		
		
		effectsManager->shadowCreate(queuePos);
		Shadow* temp = new Shadow(queuePos);
		shadowList.push_back(temp); 
		entityList.insert(temp);
		
		// this doupdate call fixes buggy sprites why dying right as a shadow is created
		temp->doUpdate();
		
		// shadows should be able to be added to enemylist,, since their moves will be used earlier?
		// nvm i dont like that.
		// do i????
		//enemyList.insert(temp);

		getMap(queuePos).insert(temp);
		// also insert this into futuremap
		futureEntityMap[queuePos.x][queuePos.y].insert(temp);
	}
	
}

void EntityManager::updateMap() { 

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
		
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y] = futureEntityMap[x][y];
			//futureEntityMap[x][y].clear();
		}
	}
	
	// do floor updates.
	
	tileManager->doFloorSteps();
	// i could(and maybe should) do haskills here,,, but,, this causes issues with shadows
	// ill do haskills, and null in kills?
	if(hasKills()) {
		return;
	}
	
	// do the rest.
	
	Shadow* doShadowDeath = NULL;

	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			
			// i tried haveing a iscollision check here, but it slowed it down??
			
			Entity* temp = NULL;
			
			// should probs use a swtitch statement here, maybe?
			// yup, made it a lil faster
			switch(entityMap[x][y].size()) {
				case 0: [[likely]]
					continue;
				case 1: [[likely]]
					
					temp = *entityMap[x][y].begin();
					
					if(!hasFloor(Pos(x, y)) && !hasCollision(Pos(x, y))) {
						if(temp->entityType() == EntityType::Shadow) {
							//effectsManager->shadowDeath(static_cast<Shadow*>(temp));
							doShadowDeath = static_cast<Shadow*>(temp);
							//doShadowDeath = true;
							addKill(temp);
						} else if(temp->entityType() == EntityType::Player) {
							if(!player->hasWings || player->hasWingsTile) {
								BN_LOG("no floor kill");
								addKill(temp);
							} else {
								if(player->wingMoveCheck != playerMoveCount && !hasKills()) { // removed one indent layer down here yeet
									player->wingMoveCheck = playerMoveCount;
									
									player->wingsUse++;
									if(player->wingsUse == 2) {
										
										if(playerStart == player->p) { // ladies and gents, 10 layers of indentation. (not counting the switch statements weirdness)
											// tbh, i rlly should of just made the,,, specialized predeath anims effects? but its easier this way, and i dont need main for anything else
											
											// THIS SHOULD 100% BE IN EFFECTS!
											// I LITERALLY HAVE DOKILLEFFECTS FOR A REASON
											shouldTickPlayer = false;
											
											unsigned tempCount = 0;
											while(true) { // 11
												player->doTick();
												for(int j=0; j<6; j++) { // 12
													game->doButanoUpdate();
												}
												if(!player->pushAnimation) {
													tempCount++;
													if(tempCount == 2) { // 13
														break;
													}
												}
											}
											
											player->wingsUse = 0;
											Effect* sweatEffect = effectsManager->generateSweatEffect();
											
											for(int i=0; i<8; i++) {
												player->doTick();
												for(int j=0; j<6; j++) {
													game->doButanoUpdate();
												}
											}
											
											effectsManager->removeEffect(sweatEffect);
											shouldTickPlayer = true;
										}
										addKill(temp);
									} else {
										// spawn wing anim/sound here
										effectsManager->wings();
									}
								}
							}

						} else {
							killEntity(temp);
						}
					}
					
					continue;
				default: [[unlikely]]
					// remove all enemies, keep obstacles
					//for(int i=0; i<entityMap[x][y].size(); i++) {
					for(auto it = entityMap[x][y].begin(); it != entityMap[x][y].end(); ) {
						
						temp = *it;
						
						if(temp->entityType() == EntityType::Player) {
							
							// grab another(any) one entity in this square to be the death reason
							SaneSet<Entity*, 4> tempMap = entityMap[x][y];
							
							tempMap.erase(temp);
							
							Entity* tempKiller = *tempMap.begin();
							
							BN_LOG("intersect kill with a ", tempKiller->entityType());
							//res = *tempMap.begin();
							addKill(tempKiller);
							++it;
							// break, NOT ITERATE here so that we dont delete the thing that killed us
							// for death animation reasons.
							//break;
						} else if(temp->entityType() == EntityType::Shadow) {
							//res = temp;
							addKill(temp);
							++it;
						} else if(temp->isObstacle()) {
							++it;
							continue;
						} else if(temp->isEnemy()) {
							// kill
							
							// the it wasnt being set here but stuff was still running fine for a while. why??
							it = killEntity(temp);
							continue;
						} else {
							BN_ERROR("a entity was somehow not a player, obstacle, or enemy. wtf.");
						}
					}
					continue;
			}
		}
	}
	
	if(doShadowDeath != NULL) {
		// SHOULD 100% BE IN DOKILLEFFECTS
		//effectsManager->shadowDeath(doShadowDeath);
	}
	
	if(hasFloor(player->p)) {
		player->wingsUse = 0;
	}
	
	// should this loop be here, or in domove
	if(enemyList.size() == 0 && shadowList.size() == 0) {
		for(auto it = obstacleList.begin(); it != obstacleList.end(); ) {
			if((*it)->entityType() == EntityType::TanStatue) {
				killEntity(*it);
				it = obstacleList.begin(); // trash code, killentity only returns the getpos index, this needs to be fixed, TODO
			} else {
				++it;
			}
		}
	}
	
	int levcount = 0;
	
	LevStatue* hasLevStatue = NULL;
	// critical levels of goofyness
	for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
		if((*it)->entityType() == EntityType::MonStatue) { 
			// mon statues werent fucking like, working properly at the end of a tick, i think this fixes that
			bn::optional<Direction> res = canSeePlayer((*it)->p);
			if(res.has_value()) {
				// puttint this mon line after, the *it, is it a useafterfree?
				
				
				// TODO, HAVE MONS LIGHTNING TAKE UP THE MAIN THREAD!!!!!!
				// THIS IS HOW WE SOLVE THE SCREEN TRANSITION TIMING ISSUES
				effectsManager->monLightning((*it)->p, res.value());
				
				for(int i=0; i<30; i++)  {
					game->doButanoUpdate();
				}
				
				
				addKill(*it);
			}
		} else if((*it)->entityType() == EntityType::LevStatue) { 
			++levcount;
			//BN_LOG("levcount = ", ++levcount, "rodUses = ", LevStatue::rodUses);
			hasLevStatue = static_cast<LevStatue*>(*it);
		}
	}
	
	BN_ASSERT(LevStatue::totalLev == levcount, "lev statue count was somehow off??", LevStatue::totalLev, "!=", levcount);
	
	bool doLevKill = false;
	if(levKill) {
		BN_LOG("levkill");
		if(hasLevStatue != NULL) {
			doLevKill = true;
		}
		levKill = false;
	}
	
	if(doLevKill) {
		
		effectsManager->levKill();
		addKill(hasLevStatue);
	}
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			futureEntityMap[x][y] = entityMap[x][y];
		}
	}
	
	sanity();
}


// -----

void EntityManager::doDeaths() {
	
	
	if(deadList.size() == 0) {
		return;
	}
	
	for(auto it = deadList.begin(); it != deadList.end(); ) {
		
		BN_ASSERT(*it != NULL, "a entity in deadlist was null, this should never happen!");
		
		//(*it)->isDead();
		
		Entity* temp = *it;
			
		delete temp;
		
		it = deadList.erase(it);
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
		if(!shouldTickPlayer && *it == player) {
			continue;
		}
		(*it)->doTick();
	}
	
}	

void EntityManager::fullUpdate() {
	doTicks();
	updateScreen();
}

// -----

void EntityManager::hideForDialogueBox(bool vis, bool isCutscene) {
	
	int compareVal = isCutscene ? 0 : 6;
	
	/*
	for(auto it = deadList.cbegin(); it != deadList.cend(); ++it) {
		if((*it) == NULL) {
			continue;
		}
		if((*it)->p.y >= compareVal) {
			(*it)->sprite.spritePointer.set_visible(vis);
		}
    }
	*/
	
	for(auto it = entityList.cbegin(); it != entityList.cend(); ++it) {
		if((*it) == NULL) {
			continue;
		}
		if((*it)->entityType() == EntityType::Interactable) {
			continue;
		}
		if((*it)->p.y >= compareVal) {
			(*it)->sprite.spritePointer.set_visible(vis);
		}
    }
	
}

bool EntityManager::exitRoom() { // this func is absolutely horrid. rewrite it to do one initial check for what entity is above you, and maybe also,, ugh idek
	
	// return true when done
	
	// dying entities (including the player) should really all be converted to being effects
	
	BN_ASSERT(hasKills(), "entityManager exitroom called when there were no kills?");
	
	
	// is this ok? should this be false
	return true;
}

void EntityManager::createKillEffects() const {
	
	Entity* hasShadowKill = NULL;
	
	for(auto it = killedPlayer.cbegin(); it != killedPlayer.cend(); ++it) {
		if(*it == NULL) {
			continue;
		}
		if((*it)->entityType() == EntityType::Shadow) {
			hasShadowKill = *it;
			break;
		}
	}
	
	if(hasShadowKill != NULL) {		
		effectsManager->shadowDeath(static_cast<Shadow*>(hasShadowKill));
	}
	
	for(auto it = killedPlayer.cbegin(); it != killedPlayer.cend(); ++it) {
		if(*it == NULL) {
			continue;
		}
		if((*it)->entityType() != EntityType::Player && (*it)->entityType() != EntityType::LevStatue && (*it)->entityType() != EntityType::Shadow) {
			effectsManager->entityKill(*it);
		}
	}
	
}

bool EntityManager::enterRoom() {
	return true;
}

void EntityManager::doVBlank() { profileFunction();
	
	// is modulo expensive???
	
	
	if(frame % 33 == 0) { // ticks should occur at roughly 110bpm
		//BN_LOG("TICKS");
		doTicks();
	}
	
	if(frame % 2 == 0) {
		//BN_LOG("PUSH");
		if(player->pushAnimation) {
			player->pushAnimation++;
			if(player->pushAnimation == 8) {
				player->pushAnimation = 0;
			}
			player->doUpdate();
		}
	}
	
	if(kickedList.size() != 0) {	
		//BN_LOG("KICK");
		for(auto it = kickedList.begin(); it != kickedList.end(); ) {
			
			Obstacle* o = (*it);
			
			// this appears to somehow be a load bearing assert. not having it here causes crashes?
			BN_ASSERT(o != NULL, "an obstacle which was kicked was somehow null. i have no clue how this occurs");
			
			//BN_LOG("doing kick");
			bool res = o->kicked();
			//BN_LOG("kick done");
			if(res) {
				it = kickedList.erase(it);
			} else {
				++it;
			}
		}
	}
	
	if(obstacleList.size() != 0) {
		for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
			static_cast<Obstacle*>((*it))->specialBumpFunction();
		}
	}
	
	// deaths is done last bc in case something is both dead, and kicked, we dont want a use after free.
	// this is a MASSIVE bug, how did i only just catch it now 
	// hell, under what circumstances do i want something to like,,, not be removed from the kicklist on the same tick its added?
	// im going to change that
	// actually no, its needed for tails boobs 
	// i knew i should of made tails boobs an effect
	// additionally, now that doDeaths is no longer animating death sequences,, 
	// it doesnt need to only be called once every 8 frames
	//if(frame % 8 == 0) {
	//BN_LOG("DEATHS");
	doDeaths();
	
	
	//BN_LOG("DONE ENTITYMANAGER VBLANK");
	
}

// -----

bool EntityManager::hasEntity(const Pos& p) const {
		
	const SaneSet<Entity*, 4>& temp = getMap(p);
		
	return temp.size() != 0;
}

bool EntityManager::hasPlayer(const Pos& p) const {
	if(!hasEntity(p)) {
		return false;
	}
	
	return getMap(p).contains(player);
}

bool EntityManager::hasNonPlayerEntity(const Pos& p) const {
	// this function exists entirely because of the goofyness with shadows technically being players
	if(!hasEntity(p)) {
		return false;
	}

	const SaneSet<Entity*, 4>& tempMap = getMap(p);
	
	if(tempMap.size() == 1 && tempMap.contains(player)) {
		return false;
	} 
	
	return true;
}
	
bool EntityManager::hasEnemy(const Pos& p) const {
	
	const SaneSet<Entity*, 4>& temp = getMap(p);
	
	if(temp.size() == 0) {
		return false;
	}
	
	for(auto it = temp.cbegin(); it != temp.cend(); ++it) {
		if((*it)->isEnemy()) {
			return true;
		}
	}
	
	return false;
}

bool EntityManager::hasObstacle(const Pos& p) const {
	
	const SaneSet<Entity*, 4>& temp = getMap(p);
	
	if(temp.size() == 0) {
		return false;
	}
	
	for(auto it = temp.cbegin(); it != temp.cend(); ++it) {
		if((*it)->isObstacle()) {
			return true;
		}
	}
	
	return false;
}

bool EntityManager::hasCollision(const Pos& p) const {
	
	const u8 temp = game->collisionMap[p.x][p.y];
	
	//if(temp == 12 || temp < 3) {
	if(temp == 1 || temp == 12) {
		return false;
	}
	
	return true;
}

bool EntityManager::hasNonInteractableObstacle(const Pos& p) const { //nice function name dumbass

	const SaneSet<Entity*, 4>& temp = getMap(p);
	
	if(temp.size() == 0) {
		return false;
	}
	
	for(auto it = temp.cbegin(); it != temp.cend(); ++it) {
		if((*it)->isObstacle() && (*it)->entityType() != EntityType::Interactable) {
			return true;
		}
	}
	
	return false;

}

bn::optional<TileType> EntityManager::hasFloor(const Pos& p) const {
	return tileManager->hasFloor(p);
}

bn::optional<Direction> EntityManager::canSeePlayer(const Pos& p) const { 
	
	Pos playerPos = player->p;
	
	BN_ASSERT(getMap(player->p).contains(player), "wtf");
	
	// first, do some inexpensive checks
	if(!(playerPos.x == p.x || playerPos.y == p.y)) {
		return bn::optional<Direction>();
	}
	
	bool checkY = playerPos.x == p.x;
	//bool checkX = playerPos.y == p.y;
	
	int sharedValue = checkY ? p.x : p.y;
	int startValue = checkY ? MIN(playerPos.y, p.y) + 1 : MIN(playerPos.x, p.x) + 1;
	int stopValue = checkY ? MAX(playerPos.y, p.y) - 1 : MAX(playerPos.x, p.x) - 1;

	//BN_ASSERT(startValue != stopValue, "in player LOS checks, the start and stop values were the same!");
	
	for(int i=startValue; i<=stopValue; i++) {
		
		Pos testPos = checkY ? Pos(sharedValue, i) : Pos(i, sharedValue);
	
		//BN_LOG("Entity at ", p, " checking LOS at ", testPos.x, " ", testPos.y, " ", hasNonPlayerEntity(testPos));
	
		if(hasCollision(testPos) || hasNonPlayerEntity(testPos)) {
			//BN_LOG("FOUND COLLISION");
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

bn::optional<Direction> EntityManager::canPathToPlayer(const Pos& p, const Pos& playerPos) const { 
	
	// oh boy. this is going to be fun.
	
	// B186 is a decent level for testing 
	// it seems that the diamonds will path if the players start OR END position is reachable.
	// weird
	// SIDESTEPPING A DIAMOND KILLS YOU??? (in gba ver, i needa fix that
	
	//Pos playerPos = player->p;
	
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
	
	// TODO, WHAT IS THE INGAME DIR PRIO. NOT KNOWING IS A SIMPLE FIX OMFG
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
			
			//if(!hasEntity(test)) {
			if(!hasNonPlayerEntity(test)) {
			//if(true) {
				queue.push_back(test);
				continue;
			}
		
		}
	}

	return bn::optional<Direction>();
}

bn::optional<Direction> EntityManager::canPathToPlayer(Diamond* e, Pos playerStartPos) { 
	
	const Direction testDirections[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
	
	// this whole loop is an affront to the gods 
	// nvm this whole func is. 
	// jesus it really is
	for(int i=0; i<4; i++) {
		Pos testPos = e->p;
		if(!testPos.move(testDirections[i])) {
			continue;
		}
		
		if(testPos == playerStartPos) {
			
			e->nextMove = testDirections[i];
			
			if(moveEntity(e, true)) {
				return testDirections[i];
			}
		}
	}
	
	return canPathToPlayer(e->p, playerStartPos);
}

void EntityManager::rodUse() {
	
	bool foundLevStatue = false;
	
	LevStatue* temp = NULL;
	
	for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
		if((*it)->entityType() == EntityType::LevStatue) {
			
			temp = static_cast<LevStatue*>(*it);
			
			if(!temp->isActive) {
				temp->activate();
				foundLevStatue = true;
				break;
			}
		}
	}
	
	//BN_LOG(!foundLevStatue, " ", temp != NULL);
	//BN_LOG(LevStatue::rodUses, " ", LevStatue::totalLev, " ", LevStatue::rodUses != 0);
	if(!foundLevStatue && temp != NULL) {
		//addKill(temp);
		levKill = true;
	}
	
	// THIS COULD BE VERY BAD IF SOMEONE PUSHES OFF A LEV STATUE WITH ONLY ONE ON SCREEN
	
	
	if(LevStatue::rodUses >= LevStatue::totalLev && LevStatue::rodUses != 0) {
		levKill = true;
	}
	
}

// -----

void EntityManager::sanity() const {
	
	// this func kills performance(should it?)
	// do iterators like fuck performance as well?
	// i could easily let saneset have indexing(and tbh, why didnt i earlier?)
	// well, if i want to remove i sorta need iterators.
	
	return;
	
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
			
			for(auto it = entityMap[x][y].cbegin(); it != entityMap[x][y].cend(); ++it) {
				BN_ASSERT((*it)->p == Pos(x, y), "A sprite at ", x, " ", y, " was not updated in the map properly! and instead is at ", (*it)->p.x, " ", (*it)->p.y);
			}
		}
	}
	
	
}



