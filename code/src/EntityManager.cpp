

#include "EntityManager.h"

#include "Game.h"
#include "TileManager.h"
#include "EffectsManager.h"

// should, all managers (and game) be namespaces? it seems so

void EntityManager::loadEntities(EntityHolder* entitiesPointer, int entitiesCount) {

	posTracker.clear();
	
	menuOpened = false;

	playerPush = NULL;

	shouldTickPlayer = true;

	LevStatue::rodUses = 0;
	LevStatue::totalLev = 0;
	
	levKill = false;

	// delete old data 
	// conspiricy time, NULL EVERYTHING.
	killedPlayer.clear();
	
	shadowQueue.clear();
	
	kickedList.clear();
	
	// saving is jank as (curse), and this is evidence of it
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
		// i shouldnt have to do this, but despite getting a literal 1:1 memory recreation, it still chose to (curse) itself 
		// it just didnt want to work.
		// a byte would just get read on a loop for some reason?
		// i cant believe i changed my endianness for this, and padded bytes and (curse)omfg
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
		/*
		if(hasFloor(tempPos)) {
			// if this is a player who spawns on a copy tile(like in bee's hole), this wont work properly since we dont have updatemap!
			tileManager->stepOn(tempPos);
		}
		*/
		
		if(i == 0) {
			BN_ASSERT(temp.t == EntityType::Player, "first entity loaded into a room MUST be the player");
		}
		
		switch(temp.t) {
			case EntityType::Player:
			
				BN_ASSERT(player == NULL, "tried to load in a player when player wasnt NULL in room ", game->roomManager.currentRoomName());
			
				player = new Player(tempPos);
				
				// having duplicates causes so many problems
				// i do not like that we set these vars here!
				player->locustCount = game->saveData.locustCount;
				player->isVoided = game->saveData.isVoided;
				if(game->mode == 2) { // cif is always voided!
					player->isVoided = true;
				}
				
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
				if(game->roomManager.currentRoomHash() == hashString("rm_secret_005")) {
					entityList.insert(new Eye(tempPos + Pos(0, 1)));
				} else {
					entityList.insert(new Eye(tempPos));
				}
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
				//entityList.insert(new Boulder(tempPos));
				entityList.insert(getBoulder(tempPos));
				break;
			case EntityType::Chest:
				entityList.insert(new Chest(tempPos));
				if(tileManager->floorMap[tempPos.x][tempPos.y] == NULL) { 
					tileManager->floorMap[tempPos.x][tempPos.y] = new FloorTile(tempPos);
				}
				break;
			case EntityType::EmptyChest:
				//entityList.insert(new Chest(tempPos, true));
				// emptychests are becoming interactables
				entityList.insert(getEmptyChest(tempPos));
				if(tileManager->floorMap[tempPos.x][tempPos.y] == NULL) {
					tileManager->floorMap[tempPos.x][tempPos.y] = new FloorTile(tempPos);
				}
				break;
			case EntityType::AddStatue:
				//entityList.insert(new AddStatue(tempPos));
				entityList.insert(getAddStatue(tempPos));
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
		
		if(hasFloor(temp->p)) {
			tileManager->stepOn(temp->p);
		}

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

AddStatue* EntityManager::getAddStatue(Pos p) {
	
	// should these,,,,, be interactables???
	// no. they move.
	// shit
	// fuck 
	// fuckkkkkkkkkkkkkk 
	// they,,, need to be both?
	// i could 
	// omfg 
	// now that specialBumpFunction is being called every tick, i can use that. 
	// but my gods does it 
	// i despise my code 
	// normally id use like,,,, an effect for this? 
	// but i feel so dumb for doing that 
	// like i hate using the effects for stuff like that, 
	// and using the specialbump func is just stupid bc its literally not named that 
	// gods 
	// also omfg special bump func isnt even a func pointer, its a thing which needs to be overloaded.
	// this is going to be shit
	
	AddStatue* res = new AddStatue(p);
	
	std::function<void(AddStatue*)> specialBumpFunctionPointer = [](AddStatue* obj) -> void {
		(void)obj;
		return;
	};
	
	unsigned roomHash = game->roomManager.currentRoomHash();
	
	if(roomHash == hashString("rm_e_019\0")) { // the floor where it needs to move up every pause
		
		specialBumpFunctionPointer = [this](AddStatue* e) -> void {
			
			if(menuOpened) {
				
				menuOpened = false;
				
				BN_LOG("move");
				
				Pos pastPos = e->p;

				futureEntityMap[e->p.x][e->p.y].erase(e);
				
				e->p.move(Direction::Up);

				bn::pair<EntityType, bn::pair<Pos, Pos>> tempFloorStep(e->entityType(), bn::pair<Pos, Pos>(pastPos, e->p));
				
				tileManager->floorSteps.push_back(tempFloorStep);
				
				futureEntityMap[e->p.x][e->p.y].insert(e);
				
				e->doUpdate();
				
				tileManager->doFloorSteps();
			}
		};
		
	} else if(roomHash == hashString("rm_e_intermission\0")) {
		// starting room of dis
		
		// leech, eye, maggot.
		// does killing the bull,, matter? if done before this case, yes i suppose
		// this is going to be coded like shit. i dont care anymore 
		
		specialBumpFunctionPointer = [this,
		stage = 0,
		hasLeech = true,
		hasEye = true,
		hasMaggot = true,
		hasBull = true
		](AddStatue* e) mutable -> void {
			
			(void)e;
			
			hasLeech = false;
			hasEye = false;
			hasMaggot = false;
			hasBull = false;
			
			for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
				switch((*it)->entityType()) {
					case EntityType::Leech:
						hasLeech = true;
						break;
					case EntityType::Eye:
						hasEye = true;
						break;
					case EntityType::Maggot:
						hasMaggot = true;
						break;
					case EntityType::Bull:
						hasBull = true;	
						break;
					default:
						break;
				}
			}
			
			if(stage == 0) {
				if(!hasLeech && hasEye && hasMaggot && hasBull) {
					stage++;
				}
			} else if(stage == 1) {
				if(!hasEye && hasMaggot && hasBull) {
					stage++;
				}
			} else if(stage == 2) {
				if(!hasMaggot && hasBull) {
					stage++;
					needKillAllAddStatues = true;
				}
			}
		};	
	} else if(roomHash == hashString("rm_e_023\0")) {
		
		specialBumpFunctionPointer = [this](AddStatue* e) mutable -> void {
			(void)e;
			
			BN_ASSERT(tileManager->memoryTile != NULL, "memoryTile tile null for some reason");
			BN_ASSERT(tileManager->wingsTile != NULL, "wingsTile tile null for some reason");
			BN_ASSERT(tileManager->swordTile != NULL, "swordTile tile null for some reason");
			
			// (4, 4) (6, 4) (8, 4)
			
			// this shit is dumb
			SaneSet<Pos, 3> temp;
			
			temp.insert(Pos(4, 4));
			temp.insert(Pos(6, 4));
			temp.insert(Pos(8, 4));
			
			temp.erase(tileManager->memoryTile->tilePos);
			temp.erase(tileManager->wingsTile->tilePos);
			temp.erase(tileManager->swordTile->tilePos);
			
			if(temp.size() == 0) {
				needKillAllAddStatues = true;
			}
		};
	} else if(roomHash == hashString("rm_e_018\0")) {
		
		// check if any glass.
		// shit code 
		
		specialBumpFunctionPointer = [this](AddStatue* e) -> void {
			(void)e;

			for(int x=0; x<14; x++) {
				for(int y=0; y<9; y++) {
					if(tileManager->hasFloor(x, y) == TileType::Glass) {
						return;
					}
				}
			}
			
			needKillAllAddStatues = true;
		};
	} else if(roomHash == hashString("rm_e_022\0")) {
		
		
		// sword the leechs
		// deathtile the maggots
		// drop the eyes 
		
		// this is going to be,,,, annoying 
		// it would of been nice if i programed in a reason for the things in deathlist 
		// i dont want to add an obscene amount of overhead for this one room ugh
		// ugh i also need to program in the boulder dialogueeee
		// ugh
		// i could add something to the deadlist vector, but it not being unique would,,, scare me 
		// i could add another variable to entity, but that would also not be ideal since,, 
		// it would make allocing all entitys more expensive 
		// but like,,,, do i rlly care?
		// probs not ig 
		// ugh and then discerning between,,, falls and collisions and deathtiles?
		// i could have,,, it,, i could have the death reason be an optional entity type. 
		// the optional being missing means its,, a tile related death, check whats under it?
		// the optional being there means, profit? 
		// this will be scuffed as fuck, but its ok.
		// specialbump func is called BEFORE deadlist meaning i can use deadlist
		
		// 4 leeches, 2 maggots, 2 eyes
		
		specialBumpFunctionPointer = [this,
		leechSuccessCount = 0,
		maggotSuccessCount = 0,
		eyeSuccessCount = 0
		](AddStatue* e) mutable -> void {
			(void)e;
			
			static unsigned prevFrame = 0;
			
			if(frame == prevFrame) {
				return;
			}
			
			prevFrame = frame;
			
			for(auto it = deadList.cbegin(); it != deadList.cend(); ++it) {
				switch( (*it)->entityType() ) {
					case EntityType::Leech:
						if((*it)->deathReason == EntityType::Player) {
							leechSuccessCount++;
						}
						break;
					case EntityType::Maggot:
						if(!(*it)->deathReason.has_value() && hasFloor((*it)->p) == TileType::Death) {
							maggotSuccessCount++;
						}
						break;
					case EntityType::Eye:
						if(!(*it)->deathReason.has_value() && !hasFloor((*it)->p)) {
							eyeSuccessCount++;
						}
						break;						
					default:
						break;
				}
			}
			
			if(leechSuccessCount == 4 && maggotSuccessCount == 2 && eyeSuccessCount == 2) {
				needKillAllAddStatues = true;
				maggotSuccessCount = 0; // stop repeated calls, which i should REALLY have for the other funcs
			}
		};		
	} else if(roomHash == hashString("rm_e_021\0")) {
		
		// send to "rm_e_027\0" if success
		
		specialBumpFunctionPointer = [this](AddStatue* e) mutable -> void {
			(void)e;
			
			if(enemyList.size() != 0) { // eyes need to be dead
				return;
			}
			
			// boulder is on (6, 4)
			
			// i pray that the exceptions for 4 i made dont fuck me here
			
			SaneSet<Pos, 4> temp;
			
			temp.insert(Pos(5, 4));
			temp.insert(Pos(7, 4));
			temp.insert(Pos(6, 3));
			temp.insert(Pos(6, 5));
			
			// i probs should of just used getpos calls for this ugh
			
			for(auto it = obstacleList.cbegin(); it != obstacleList.cend(); ++it) {
				
				switch( (*it)->entityType() ) {
					case EntityType::EusStatue:
						return; // eus needs to not be there
						break;
					case EntityType::Boulder:
						if( (*it)->p != Pos(6, 4) ) {
							return;
						}
						break;				
					case EntityType::LevStatue:
						temp.erase((*it)->p);
						break;
					default:
						break;
				}
			}
		
			if(temp.size() == 0) {
				// add statues dont die here
				tileManager->exitDestination = "rm_e_027\0";
			}
		};
	} else if(roomHash == hashString("rm_secret_008\0")) {
		
		// cif room, should give 99 locusts when the tan statue is moved
		
		specialBumpFunctionPointer = [this](AddStatue* e) mutable -> void {
			(void)e;
			
			if(player->p != Pos(8, 1) && getMap(Pos(8, 1)).size() == 1) {
				needKillAllAddStatues = true;
			}
		};
	}
	
	res->specialBumpFunctionPointer = specialBumpFunctionPointer;
	
	return res;
	
}

Interactable* EntityManager::getEmptyChest(Pos p) {
	
	// reassignment doesnt work here when its auto, but does for defining the type??
	std::function<void(void*)> interactFunc = [](void* unused) mutable -> void {
		(void)unused;
	};
	
	std::function<bool(void*)> kickFunc = [](void* unused) mutable -> bool {
		(void)unused;
		globalGame->playSound(&bn::sound_items::snd_push_small);
		return true;
	};
	
	std::function<void(Interactable*)> initFunc = [](Interactable* inter) mutable -> void {
		inter->sprite.setVisible(true);
		inter->spriteTilesArray.clear();
		inter->spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_regular);
		inter->animationIndex = 1;
	};
	
	std::function<void(Interactable*)> specialBumpFunc = [](Interactable* inter) mutable -> void {
		(void)inter;
	};
	
	int roomIndex = game->roomManager.roomIndex;
	unsigned roomHash = game->roomManager.currentRoomHash();
	
	if(roomIndex == 83 && p == Pos(3, 2)) {
		interactFunc = [interactCount = 0](void* unused) mutable -> void {
			(void)unused;
			//if(globalGame->roomManager.roomIndex == 83 && p == Pos(3, 2)) {
			// GOOFY WARNING
			if(globalGame->entityManager.player->p == Pos(3, 2+1)) {
				// shortcut chest;
				
				interactCount++;
				
				switch(interactCount) {
					case 1:
						globalGame->effectsManager.doDialogue("It's empty\0");
						break;
					case 2:
						globalGame->effectsManager.doDialogue("It's empty\0");
						break;
					case 3:
						globalGame->effectsManager.doDialogue("It's empty\0");
						break;
					default:
						globalGame->effectsManager.doDialogue("?? ??? ?\0");
						globalGame->entityManager.addKill(NULL);
						globalGame->tileManager.exitDestination = "rm_mon_shortcut_003\0";
						break;
				}
				
				return;
			}
		};
		
	} else if(roomHash == hashString("rm_secret_001\0")) {
	
		// i would(in the past) put this type of code into the roomconv file, but i am TIRED.
		// these chests should also use the special chest sprite
	
		// memory room.
		// chest just gives it to you
		// [You aquired a strange feeling]
		// [Your mind feels heavier]
		// [You don't know what to make of it]
		
		// rock tells you to hit the chest 6 times, wait 6sec, add statue dissapear
		
		interactFunc = [interactCount = 0](void* obj) mutable -> void {
			//(void)unused;
			BN_ASSERT(obj != NULL, "WTF IN interactfunc ");
			
			Interactable* inter = static_cast<Interactable*>(obj);
			
			//if(globalGame->roomManager.roomIndex == 83 && p == Pos(3, 2)) {
			// GOOFY WARNING
			
			if(globalGame->entityManager.player->p == Pos(6, 2+1)) {
				if(interactCount == 0 && !globalGame->entityManager.player->hasMemory) {
					globalGame->effectsManager.doDialogue(""
					"[You aquired a strange feeling]\n"
					"[Your mind feels heavier]\n"
					"[You don't know what to make of it]\0");
					interactCount++;
					globalGame->entityManager.player->hasMemory = true;
					globalGame->tileManager.fullDraw();
					
					inter->animationIndex = 1;
					inter->doUpdate();
				} else {
					globalGame->effectsManager.doDialogue("It's empty\0");
				}
			}
		};
		
		initFunc = [hasMemory = player->hasMemory](Interactable* inter) mutable -> void {
			inter->sprite.setVisible(true);
			inter->spriteTilesArray.clear();
			inter->spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_small);
			inter->animationIndex = hasMemory;
		};
		
		kickFunc = [](void* obj) -> bool {
			
			globalGame->playSound(&bn::sound_items::snd_push_small);
			
			BN_ASSERT(obj != NULL, "WTF IN kickedfunc ");
			
			Interactable* inter = static_cast<Interactable*>(obj);

			if(ABS(playerIdleFrame - inter->playerIdleStart) > 60) {
				inter->specialBumpCount = 0;
			}
			
			inter->playerIdleStart = playerIdleFrame;
			
			inter->specialBumpCount++;
			
			return true;
		};
		
		// this is bad,, how do i detect the 6sec wait 
		// godssssssssssss
		// i suppose i could return false?
		//.... i could overload specialBumpFunction???
		specialBumpFunc = [trig = false](void* obj) mutable -> void {
		
			BN_ASSERT(obj != NULL, "WTF IN SPECIAL BUMP FUNC");
		
			Interactable* inter = static_cast<Interactable*>(obj);
		
			if(!trig && inter->specialBumpCount == 6 && playerIdleFrame == inter->playerIdleStart && (frame - inter->playerIdleStart) >= 60 * 6) {
				trig = true;
				//globalGame->entityManager.killAllAddStatues();
				globalGame->entityManager.needKillAllAddStatues = true;
			}
			
		};
		
	} else if(roomHash == hashString("rm_secret_003\0")) {
		// wings room
		// straight up just gives you wings 
		
		interactFunc = [interactCount = 0](void* obj) mutable -> void {
			//(void)unused;
			Interactable* inter = static_cast<Interactable*>(obj);
			
			//if(globalGame->roomManager.roomIndex == 83 && p == Pos(3, 2)) {
			// GOOFY WARNING
			
			if(globalGame->entityManager.player->p == Pos(6, 2+1)) {
				if(interactCount == 0 && !globalGame->entityManager.player->hasWings) {
					globalGame->effectsManager.doDialogue(""
					"[Wings wings wings, yeet]"
					"\0");
					interactCount++;
					globalGame->entityManager.player->hasWings = true;
					globalGame->tileManager.fullDraw();
					
					inter->animationIndex = 1;
					inter->doUpdate();
				} else {
					globalGame->effectsManager.doDialogue("It's empty\0");
				}
			}
		};
		
		initFunc = [hasWings = player->hasWings](Interactable* inter) mutable -> void {
			inter->sprite.setVisible(true);
			inter->spriteTilesArray.clear();
			inter->spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_small);
			inter->animationIndex = hasWings;
		};
		
		
	} else if(roomHash == hashString("rm_secret_005\0")) {
		// sword room
		// straight up just gives you sword(well,,, tan fight). also, make sure to prekill the eyes in here. it aint possibe to solve the room 
		// without the wings bc we arent doing tan
		
		interactFunc = [interactCount = 0](void* obj) mutable -> void {
			//(void)unused;
			Interactable* inter = static_cast<Interactable*>(obj);
			
			//if(globalGame->roomManager.roomIndex == 83 && p == Pos(3, 2)) {
			// GOOFY WARNING
			
			if(globalGame->entityManager.player->p == Pos(7, 5+1)) {
				if(interactCount == 0 && !globalGame->entityManager.player->hasSword) {
					globalGame->effectsManager.doDialogue(""
					"[Swords swords swords, yeet]"
					"\0");
					interactCount++;
					globalGame->entityManager.player->hasSword = true;
					globalGame->tileManager.fullDraw();
					
					inter->animationIndex = 1;
					inter->doUpdate();
				} else {
					globalGame->effectsManager.doDialogue("It's empty\0");
				}
			}
		};
		
		initFunc = [hasSword = player->hasSword](Interactable* inter) mutable -> void {
			inter->sprite.setVisible(true);
			inter->spriteTilesArray.clear();
			inter->spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_small);
			inter->animationIndex = hasSword;
		};
		
		
	} else if(roomHash == hashString("rm_secret_008\0")) {
		
		initFunc = [](Interactable* inter) mutable -> void {
			inter->sprite.setVisible(true);
			inter->spriteTilesArray.clear();
			inter->spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_small);
			inter->animationIndex = 0;
		};
		
		interactFunc = [](void* obj) mutable -> void {
		
			Interactable* inter = static_cast<Interactable*>(obj);
			
			if(inter->animationIndex == 1) {
				globalGame->effectsManager.doDialogue("It's empty\0");
				return;
			}
			
			if(globalGame->entityManager.player->p == Pos(11, 3+1)) {
				globalGame->effectsManager.doDialogue("[You aquired 99 locust idols]\n[I N C R E D I B L E !]\0");
				inter->animationIndex = 1;
				globalGame->entityManager.player->locustCount = 99;
				globalGame->tileManager.locustCounterTile->first = '9';
				globalGame->tileManager.locustCounterTile->second = '9';
				globalGame->tileManager.updateLocust();
				inter->doUpdate();
			}
		};
	}
	
	Interactable* res = new Interactable(p, 
		interactFunc,
		kickFunc,
		NULL,
		NULL,
		initFunc,
		specialBumpFunc
	);
	
	return res;
}

Boulder* EntityManager::getBoulder(Pos p) {
	
	Boulder* res = new Boulder(p);
	
	// should this be switch or if else?
	
	switch(game->roomManager.currentRoomHash()) {
		case hashString("rm_e_intermission\0"):
			res->specialDialogue = "That leech, that lazy eye, that maggot...\nTraitors' agents who roam here...\nHear me, reach them, slay them...\nAnd thus you shall prove your mettle...\0";
			break;
		case hashString("rm_e_023\0"):
			res->specialDialogue = "Your blade...\nYour wings...\nYour memories...\nHow much do you treasure them...\0";
			break;
		case hashString("rm_e_018\0"):
			res->specialDialogue = "How brittle...\nBe careful where you step...\nAnd leave nothing behind...\nDon't ask how I know...\0";
			break;
		case hashString("rm_e_019\0"):
			res->specialDialogue = "This room was my joker moment.\rJust press start until the statue moves to the button\0";
			break;
		case hashString("rm_e_022\0"):
			res->specialDialogue = "May the blade of beginnings slash through the leeches...\nMay the ever vigilant lightning strike down the maggots...\nMay nothingness claim the lazy eyes...\nLike the traitorous lords were defeated a long time ago...\0";
			break;
		case hashString("rm_e_020\0"):
			res->specialDialogue = "There's another room...\nJust like this one...\nI just can't remember...\nWhat it looked like...\0";
			break;
		case hashString("rm_e_021\0"):
			res->specialDialogue = "Were you always there...\nHow strange...\nEverything's so different...\nDoes it even matter...\0";
			break;
		case hashString("rm_e_000\0"):
			res->specialDialogue = "Programming this would of been really hard...\nAnd im tired, and my tummy hurt...\nDon't ask how I knows...\nAlso,,, i dont have the next rooms programmed in, so this just sends you to the start of mons funhouse. sry\n<3\0";
			break;
		default:
			break;		
	}
	
	return res;
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

bool EntityManager::moveEntity(Entity* e, bool dontSet) { profileFunction();
	
	switch(e->entityType()) {
		//case EntityType::Chest:
			
			//(static_cast<Obstacle*>(e))->bumpDirections.clear(); // is this ok?
			//return false;
		case EntityType::Interactable:
			return false;
		default:
			break;
	}
	
	if(!dontSet) {
		posTracker.insert(e->p);
	}
	
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
	
	posTracker.insert(testPos);
	
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

void EntityManager::doMoves() { profileFunction();

	//BN_LOG("\n\ndoMoves START");
	
	// return an entity if we died an need a reset
	bn::optional<Entity*> res;
	
	
	player->hasWingsTile = player->inRod(tileManager->wingsTile);
	bool prevHasWingsTile = player->hasWingsTile;
	
	// insert player's move into its func 
	// IF THE PLAYER CHANGES THE FLOOR, DO IT HERE.
	bn::pair<bool, bn::optional<Direction>> playerRes = player->doInput();
	
	if(prevHasWingsTile && !player->inRod(tileManager->wingsTile)) {
		player->hasWingsTile = false;
		//player->wingsUse = 0;
	}
	
	if(!playerRes.first) {
		return;
	}
	
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		
		switch((*it)->entityType()) {
			case EntityType::WhiteMimic:
			case EntityType::GrayMimic:
			case EntityType::BlackMimic:
				static_cast<Mimic*>(*it)->nextMove = playerRes.second;
				break;
			default:
				break;
		}
	
		// diamonds will be inserted AFTER obstacles move.
		// (maybe) insert diamonds move into its func? (MORE NEEDS TO BE DONE ON THIS)
		// nvm we gon be goofy here and do this twice.
		// nvm x2, gods that sounds horrid for efficiency.	
	}
	
	// (maybe) insert shadows move into its func? (MORE NEEDS TO BE DONE ON THIS)
	
	//stupid code 
	Pos playerStartBackup = player->p;
	
	// do player move.
	playerStart = player->p;
	bool playerMoved = moveEntity(player);
	//updateMap(); // now that i dont care abt framedrops, 
	
	// telling the difference between if the player failed a move vs is talking to something is gonna be weird
	// actually, i suppose we have the movefailed func, i can just overload that?
	
	
	
	// UNSURE IF THIS SHOULD BE HERE
	//updateMap();
	//if(hasKills()) {
	//	return;
	//
	
	//player->doUpdate();*/
	/*
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			// this is expensive, and fucking stupid,,,, is it even needed?
			// this literally updates the whole ass map, for ONE. PLAYER
			entityMap[x][y] = futureEntityMap[x][y];
		}
	}
	*/
	
	entityMap[playerStart.x][playerStart.y] = futureEntityMap[playerStart.x][playerStart.y];
	entityMap[player->p.x][player->p.y] = futureEntityMap[player->p.x][player->p.y];
	
	
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
	
	// THIS MIGHT BE FUCKING BAD
	// THIS CASE IS HERE TO NOT UPDATE SHADOWS WHEN WE JUST JUMP OFF A CLIFF
	// having this extra call here is bad, and wastes time. omfg
	//BN_LOG("player move");
	updateMap(); 
	if(hasKills()) {
		return;
	}
	
	Diamond* tempDiamond;
	Bull* tempBull;
	Chester* tempChester;
	
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		switch((*it)->entityType()) {
			case EntityType::Diamond:
				tempDiamond = static_cast<Diamond*>(*it);
				// account for the goofy ahh edge case in which a diamond is one move away from 
				// the player, have it move to the players previous position
				// nvm turned out to be less goofy than i thought(no if statements at least)
				if(!hasFloor(player->p)) {
					tempDiamond->nextMove = canPathToPlayer(tempDiamond, playerStart);
					//BN_LOG("diamond pathing with player start pos ", playerStart, " dir=", temp->nextMove.has_value());
				} else {
					tempDiamond->nextMove = canPathToPlayer(tempDiamond, player->p);
					//BN_LOG("diamond pathing with player current pos ", player->p, " dir=", temp->nextMove.has_value());
				}
				
				// i swear to the gods, none of this makes any sense
				for(int i=0; i<4; i++) {
					Pos tempPos = tempDiamond->p;
					if(tempPos.move(static_cast<Direction>(i)) && tempPos == player->p) {
						tempDiamond->idle = false;
						tempDiamond->tileIndex = 0;
						addKill(tempDiamond);
					}
				}
				break;
			case EntityType::Bull:
				// this code was originally in the actual entity area. maybe,, i should have a queuemove func for each entity? idek
				// having it in here is ugly af, but,, it be like that
				
				tempBull = static_cast<Bull*>(*it);
		
				if(tempBull->idle) {
					// perform LOS check
					
					bn::optional<Direction> playerDir = canSeePlayer(tempBull->p);
					
					if(playerDir) {
						tempBull->nextMove = playerDir.value();
						tempBull->currentDir = tempBull->nextMove.value();
						tempBull->idle = false;
					} else {
						// if we are still idle, return, if not, kachow
						tempBull->nextMove = bn::optional<Direction>();
					}
				} else {
					tempBull->nextMove = tempBull->currentDir;
				}					
			
				break;
			case EntityType::Chester:
			
				// ok,, what if,,,,, gods 
				// chesters also have some sort of idle state??
				// the mechanics with them interacting with well, i suppose anything but in this case shadows are very weird
				
				tempChester = static_cast<Chester*>(*it);
				
				if(tempChester->idle) {
					tempChester->nextMove = canSeePlayer(tempChester->p);
					if(tempChester->nextMove.has_value()) {
						tempChester->currentDir = tempChester->nextMove.value();
					} else {
						tempChester->idle = false;
					}
					
				} else {
					tempChester->nextMove = tempChester->currentDir;
				}
		
				break;
			default:
				break;
		}
	}
	


	if( (shadowMove) && !(shadowQueue.size() == 0 && shadowList.size() == 0)) {	
		manageShadows(shadowMove);
		//BN_LOG("shadow move");
		// TBH, THE MANAGESHADOW SHADOW EXISTANCE CHECK SHOULD OCCUR OUT HERE, AND THIS CALL SHOULDNT EVEN HAPPEN IF NO SHADDOWS EXIST!
		updateMap(); // this call may not be needed, but im not risking it
	}
	
	// leaving this here bc technically, the above switch statement with the diamonds can cause a death
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
	// THIS SHOULD REAAAAAAAAAAAAAAAAAAALLLLLLY only move the obstacle the player bumped into(if they did?);
	// this code is still not ideal, but the issue of kickedlist not being actually, cleared means,,, yea 
	// ugh,, but tbh the idea of saving a whole loop,,,, sounds wayyyy to good.
	// ima do some jank
	// actually,,, no.
	// additionally,,,, all obstacles need a update every tick in case their floor was bombed,,, a bit not ideal tbh 
	
	//moveObstacles();
	if(playerPush != NULL) {
		// my first attempt at this didnt do a if null, and was constantly jumping to undef code(bc ofc it was) im so dumb
		moveEntity(playerPush);
		playerPush = NULL;
	}
	
	// i rlly rlly think that,, the playerpush move was the right move here.
	
	/*
	//for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
	for(auto it = kickedList.begin(); it != kickedList.end(); ++it) { // unsure if this change is ok!
		if( (static_cast<Obstacle*>(*it))->bumpDirections.size() != 0 ) {
			moveEntity(*it);
		}
	}
	*/
	
	//BN_LOG("PLAYER obstacle move");
	updateMap();
	if(hasKills()) {
		return;
	}
	
	
	
	
	// for all mimics
	// check if mimic is moving into obstacle
	// average out the movement of the obstacle
	// then move all obstacles
	// if that obstacle kills something, remove it from all sublists.
	
	Mimic* tempMimic = NULL;
	bn::optional<Direction> tempMimicNextMove;
	
	// obstacle collision is done via: if (not obstacle) and (not collision), do move
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		switch((*it)->entityType()) {
			case EntityType::WhiteMimic:
			case EntityType::GrayMimic:
			case EntityType::BlackMimic:
			
				// i only want the obstacle updates here, so im reseting the pos after
				
				tempMimic = static_cast<Mimic*>(*it);
				tempMimicNextMove = tempMimic->nextMove;
				
				// if the mimics move resulted in a obstacle being pushed, or just in general failed 
				// we DONT want to give it the direction back
				// if true, we do
				if(moveEntity(tempMimic, true)) {
					tempMimic->nextMove = tempMimicNextMove;
				}
				
				break;
			default:
				break;	
		}
	}
	
	/*
	// does this call even,, do ANYTHING???
	updateMap();
	if(hasKills()) {
		return;
	}
	*/
	
	//for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
	for(auto it = kickedList.begin(); it != kickedList.end(); ++it) { // unsure if this change is ok!
		if( (static_cast<Obstacle*>(*it))->bumpDirections.size() != 0 ) {
			moveEntity(*it);
		}
	}

	//moveObstacles();
	//BN_LOG("GENERAL obstacle move");
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
	
	posTracker.insert(player->p); // a chester, moving onto and destroying a section of bomb tiles with you on it, wasnt properly activiating wings?
	
	moveEnemies();
	//BN_LOG("enemy move");
	updateMap();
	if(hasKills()) {
		return;
	}
	
	
	
	// THIS SHOULD RLLY BE PUT INTO THE BELOW LOOP!
	// its such a bummer that the behavours for these objects arent stored in the object itself.
	
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
	
	bn::optional<Direction> tempDir;
	
	LevStatue* hasLevStatue = NULL;
	// critical levels of goofyness
	for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
		switch((*it)->entityType()) {
			//case EntityType::TanStatue:
				// i,, i could destroy them here, and save a loop, but i would have to do multiple enemyList.size() == 0 && shadowList.size() == 0) checks?
				//break;
			case EntityType::MonStatue:
				// mon statues werent (curse)ing like, working properly at the end of a tick, i think this fixes that
				tempDir = canSeePlayer((*it)->p);
				if(tempDir.has_value()) {
					// puttint this mon line after, the *it, is it a useafterfree?
					
					
					updateScreen();
					
					// TODO, HAVE MONS LIGHTNING TAKE UP THE MAIN THREAD!!!!!!
					// THIS IS HOW WE SOLVE THE SCREEN TRANSITION TIMING ISSUES
					effectsManager->monLightning((*it)->p, tempDir.value());
					
					for(int i=0; i<30; i++)  {
						game->doButanoUpdate();
					}
					
					
					addKill(*it);
				}
				break;
			case EntityType::LevStatue:
				++levcount;
				//BN_LOG("levcount = ", ++levcount, "rodUses = ", LevStatue::rodUses);
				hasLevStatue = static_cast<LevStatue*>(*it);
				break;
			default:
				break;
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
	
	//player->hasWingsTile = player->inRod(tileManager->wingsTile);
	// DO STATUE CHECKS
	// THIS STILL AINT DONE

	// copy over new entityMap to old, and update all subarrays.
	// delete anything which needs to be killed
	
	// WARNING, SHIT CODE ALERT
	// there aint a easy way for me to do this, im just going to see if this works 
	// and if it does work(without lag), ill be pissed bc that means my overly complex tile system was for nothing
	// it does, and it doesnt drop. 
	// why the fuck did i end up going with the system i have? i mean it is good tbh, but still 
	// it was probs just other areas of unoptimized code that were actally causing the lag.
	// ok nvm, now its lagging(now that i had it properly erase old tiles? 
	// which for some reason, makes me happy
	
	
	if(game->roomManager.isWhiteRooms()) {
		//tileManager->fullDraw();
		//tileManager->floorLayer.draw();
		//tileManager->floorLayer.draw(game->collisionMap, tileManager->floorMap);
		tileManager->updateWhiteRooms(playerStartBackup, player->p);
	}
			
	sanity();
	
	updateScreen();
	
}

// -----

bn::vector<Entity*, 4>::iterator EntityManager::killEntity(Entity* e) { profileFunction();
	
	BN_ASSERT(e != NULL, "killentity tried to kill a null??");
	
	// this func really needs to be rewritten to just work with killing players tbh, also its return value is scuffed af.
	
	if(e->entityType() == EntityType::Player || e->entityType() == EntityType::Shadow) {
		BN_ERROR("tried to kill either a player or a shadow or an interactable. thats not a thing you can do fool.");
	}
	
	if(e->entityType() == EntityType::Interactable) {
		// hopefully this doesnt (curse) anything up
		// YOU DUMBASS I THINK IT DID???
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
	
	//posTracker.insert(tempPos);
	
	// IS THIS CALL NEEDED/OK??? WHAT THE FUCK
	futureEntityMap[tempPos.x][tempPos.y].erase(e);
	
	bn::vector<Entity*, 4>::iterator res = entityMap[tempPos.x][tempPos.y].erase(e);
	entityList.erase(e);
	obstacleList.erase(e);
	enemyList.erase(e);		

	// this was legacy, but is now here just to make sure i dont leak memory in rlly stupid ways
	// and also for EUS STATUES, NOT MODIFYING THE (curse)IN BS IN THE MIDDLE OF A GAMEUPDATE
	deadList.insert(e);
	
	
	// this initially called on all entities, should it?
	// if two entities collide on a tile, the tile should collapse
	/*
	if(tileManager->hasFloor(tempPos) && e->entityType() == EntityType::TanStatue) {
		tileManager->stepOff(tempPos);
	}
	
	if(e->entityType() == EntityType::LevStatue) {
		// this happening here is (curse)ing dumb, and i would prefer it occur with the destructor, but idk
		// actually,,, we have the isdead func, which gets called during vblank. i could call it here? to some potential repercussions
		//i cannot spell
		// tan statue should also have that
		//LevStatue::totalLev--;
	}*/
	
	// this may be horrid

	if(!hasFloor(tempPos)) {
		effectsManager->entityFall(e);
	} else {
		game->playSound(&bn::sound_items::snd_enemy_explosion);
		e->sprite.setVisible(false);
		effectsManager->explosion(tempPos);
	}
	
	effectsManager->rotateTanStatues();
	
	e->isDead();
	
	
	// should a floorstep be put in here?
	
	
	return res;
}

void EntityManager::removeEntity(Entity* e) {
	
	Pos tempPos = e->p;
	
	futureEntityMap[tempPos.x][tempPos.y].erase(e);
	entityMap[tempPos.x][tempPos.y].erase(e);
	
	entityList.erase(e);
	obstacleList.erase(e);
	enemyList.erase(e);		

	deadList.insert(e);
	
	
}

void EntityManager::killAllAddStatues() {
	
	// SHOULD THIS CHECK BE HERE??? BC LIKE,,, UGHH GODS 
	// it was previously in vblank, but like, it seems like killing entities in vblank fucks shit up 
	// i also was probs not updating the entitymaps properly.
	
	// i,,,, i 
	// look you wouldnt believe the amount of issues this func has caused. 
	// i still think it might 
	// if any crashes without asserts occur, it was probs this
	// im fucking stupid and dont understand iterators no matter  
	// i,, ugh 
	// this is why i am trash
	
	
	
	//killAllAddStatues();
	for(auto it = obstacleList.begin(); it != obstacleList.end(); ) {
		BN_ASSERT(*it != NULL, "wtf in killalladdstatues");
		if((*it)->entityType() == EntityType::AddStatue) {
			
			// should this be here or in,,, killentity
			tileManager->stepOffs.insert((*it)->p);
			
			killEntity(*it);
			
			it = obstacleList.begin(); // trash code, killentity only returns the getpos index, this needs to be fixed, TODO
		} else {
			++it;
		}
	}
	
	tileManager->doFloorSteps();
	
	// is this call needed????? is this call extremely stupid??? will this call fix this stupid bug???
	//updateMap();

}

void EntityManager::manageShadows(bn::optional<Direction> playerDir) { profileFunction();
	
	// IMPORTANT. SHADOWS WILL ALWAYS SPRING UP FROM THE TILE THEY WERE STEPPED ON FROM
	// HOWEVER, IF THERE IS ANOTHER SHADOW(POSSIBLY ANY ENTITY?) ON SAID TILE 
	// IT WILL WAIT UNTIL CLEAR.
	// ADDITIONALLY, NEW SHADOWS WILL SPAWN AFTER OLD ONES MOVE

	BN_ASSERT(playerDir.has_value(), "manageshadows was called with a playerdir without a value??");
	
	Direction moveDir = playerDir.value();

	// i rlly should have a func for this bynow, screw it, now it kinda does
	// i should rlly impliment this in mimic
	// moving the,, shadow at the end to the players past pos would be much more efficent!!
	// YUP, and that finally what ima do here.
	
	// i REALLY hope that like, this kind of data movement doesnt involve shifting the 
	// shit. i should be using a dequeue. 
	// i should also be using a dequeue for the shadowqueue.
	// and probs for a ton of other shit.
	
	// this whole function is now O(1) (assuming const insertion times, which isnt the case). that is insane to me.
	
	if(shadowList.size() != 0) {
		
		Pos nextShadowPos = player->p;
		BN_ASSERT(nextShadowPos.moveInvert(moveDir, true, true), "somehow, in shadowmanager when reversing a playermove, the move failed??");
		
		Shadow* lastShadow = shadowList.back();
		shadowList.pop_back();
		Pos prevShadowPos = lastShadow->p;
		
		lastShadow->p = nextShadowPos;
		lastShadow->currentDir = moveDir;
		
		shadowList.push_front(lastShadow);
		
		bn::pair<EntityType, bn::pair<Pos, Pos>> tempFloorStep(EntityType::Shadow, bn::pair<Pos, Pos>(prevShadowPos, nextShadowPos));
		tileManager->floorSteps.push_back(tempFloorStep);
		
		futureEntityMap[prevShadowPos.x][prevShadowPos.y].erase(lastShadow);
		futureEntityMap[nextShadowPos.x][nextShadowPos.y].insert(lastShadow);
		
		posTracker.insert(prevShadowPos);
		posTracker.insert(nextShadowPos);
	}
	
	if(shadowQueue.size() != 0) {
		
		Pos queuePos = *shadowQueue.begin();
		
		// not using hasEntity here bc im going off of the future map in this case.
		//if(hasEntity(queuePos)) {
		if(futureEntityMap[queuePos.x][queuePos.y].size() == 0) {
			
			// this may be an assumption. 
			// but, 
			// if/assuming the queue is in order,,, 
			// i can just break here?
			// i really dont think this should work tbh 
			// but it somehow does.
			// im,, im inserting stuff to the end of the shadowqueue, 
			// BUT WAIT THE STUFF AT THE START IS THE OLDEST, MEANING SOONEST TO GET CREATED
			// im chilling.
			// and,,, since only one shadow can be created per move, i can further optimize this
			//break;
			
			shadowQueue.erase(shadowQueue.begin());
		
			// unsure if needed
			posTracker.insert(queuePos);
			
			effectsManager->shadowCreate(queuePos);
			Shadow* temp = new Shadow(queuePos);
			
			shadowList.push_back(temp);

			temp->animationIndex = shadowList.front()->animationIndex;

			
			entityList.insert(temp);
			
			// this doupdate call fixes buggy sprites why dying right as a shadow is created
			//temp->doUpdate();
		
			futureEntityMap[queuePos.x][queuePos.y].insert(temp);
			
		}		
	}
	
	
	
}

void EntityManager::updateMap() { profileFunction();

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

	// doing this type of copy is,,, expensive. ugh
	/*
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y] = futureEntityMap[x][y];
			//futureEntityMap[x][y].clear();
		}
	}
	*/
	
	// do floor updates.
	
	// this is shit code. im just doing this to like,,,,, see if this is even feasable.
	// tbh tho, i think ill have a seperate var for tracking it.
	/*
	SaneSet<Pos, MAXENTITYSPRITES> testPoses;
	
	// even with this horrid copying, the code is still efficient!
	for(auto it = tileManager->floorSteps.cbegin(); it != tileManager->floorSteps.cend(); ++it) {
		testPoses.insert((*it).second.first);
		testPoses.insert((*it).second.second);
	}
	*/
	
	// this copy needs to occur here so that doFloorSteps has the correct data to work on. 
	// if it messes up performance,,, 
	// gods i hope it doesnt
	// putting the copies into the actual loop made stuff faster
	// i hope this doesnt hurt me to much
	// ok, it doesnt seem to
	
	if(posTracker.size() != 0) {
		for(auto posit = posTracker.cbegin(); posit != posTracker.cend(); ++posit) {
	
			int x = (*posit).x;
			int y = (*posit).y;
	
			entityMap[x][y] = futureEntityMap[x][y];
		}
	}
	
	
	tileManager->doFloorSteps();
	// i could(and maybe should) do haskills here,,, but,, this causes issues with shadows
	// ill do haskills, and null in kills?
	if(hasKills()) {
		return;
	}
	
	// do the rest.
	
	//Shadow* doShadowDeath = NULL;

	// this could most likely be optimized if i have a,, SET of positions which need to be checked.
	// now that i think about that, isnt the thing that i have in floorsteps,,, exactly that?
	
	// instead of doing the whole map.
	// im starting to have frame drops again, and while ik its ok, having framedrops in vblank due to my shitty effects classes, is rlly rlly bad 
	// ugh 
	// if i could,, easily put the computation parts for effects inside 
	// of cpu cycles, and just have vblank do drawing, but ugh 
	// i should of had a more event based system for the timings of event ticks.



	Entity* temp = NULL;


	//BN_LOG("posTracker size is ", posTracker.size());
	
	//for(int x=0; x<14; x++) {
	//	for(int y=0; y<9; y++) {
	if(posTracker.size() != 0) {
		for(auto posit = posTracker.cbegin(); posit != posTracker.cend(); ++posit) {

			int x = (*posit).x;
			int y = (*posit).y;
		
			//entityMap[x][y] = futureEntityMap[x][y];
	
			// i tried haveing a iscollision check here, but it slowed it down??
			
			// should probs use a swtitch statement here, maybe?
			// yup, made it a lil faster
			switch(entityMap[x][y].size()) {
				case 0: [[likely]]
					break;
				case 1: [[likely]]
					
					temp = *entityMap[x][y].begin();
					
					if(!hasFloor(Pos(x, y)) && !hasCollision(Pos(x, y))) {
						
						switch(temp->entityType()) {
							case EntityType::Shadow:
								addKill(temp);
								break;
							case EntityType::Player:
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
								break;
							default:
								killEntity(temp);
								break;
						}
					}
					break;
				default: [[unlikely]]
					// remove all enemies, keep obstacles
					for(auto it = entityMap[x][y].begin(); it != entityMap[x][y].end(); ) {
						
						temp = *it;
						
						switch(temp->entityType()) {
							case EntityType::Player:
								entityMap[x][y].erase(temp);
								BN_LOG("intersect kill with a ", (*(entityMap[x][y]).begin())->entityType());
								addKill((*(entityMap[x][y]).begin()));
								++it;
								break;
							case EntityType::Shadow:
								addKill(temp);
								++it;
								break;
							default:
								if(temp->isObstacle()) {
									++it;
								} else if(temp->isEnemy()) {
									it = killEntity(temp);
									continue;
								} else {
									BN_ERROR("a entity was somehow not a player, obstacle, or enemy. wtf.");
								}
								break;
						}
					}
					break;
			}
		
			futureEntityMap[x][y] = entityMap[x][y];
		}
	}
	
	if(hasFloor(player->p)) {
		player->wingsUse = 0;
	}
	
	posTracker.clear();
	
	// all the following code could(and probs should) be called once at the end of the domove func,,,,, but 
	// ugh i am so scared to touch all this but i have to.
	// im keeping the wings check here tho
	
	/*
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			futureEntityMap[x][y] = entityMap[x][y];
		}
	}
	*/
	
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
	//doTicks();
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
		if((*it)->entityType() != EntityType::Player && (*it)->entityType() != EntityType::LevStatue && (*it)->entityType() != EntityType::MonStatue && (*it)->entityType() != EntityType::Shadow) {
			effectsManager->entityKill(*it);
		}
	}
	
}

bool EntityManager::enterRoom() {
	return true;
}

void EntityManager::doVBlank() { profileFunction();
	
	// is modulo expensive???
	//if(frame % 33 == 0) { // ticks should occur at roughly 110bpm
	if(frame % 32 == 0) { // ticks should occur at roughly 110bpm, nvm now its 32 bc power of 2
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
	
	// is this ok to occur here?
	if(needKillAllAddStatues) {
		needKillAllAddStatues = false;
		killAllAddStatues();
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

	// this causes a BUNCH of lag
	// it should skip empty chests, and stuff that doesnt have a special bump 
	// it doesnt
	// CREATE A FUCKING SPECIALBUMP LIST!
	
	// i despise this code. it also is DEF the thing contributing to slowdown in vblank!
	if(obstacleList.size() != 0) {
		for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
			//BN_LOG("SPECIALBUMP");
			BN_ASSERT(*it != NULL, "a specialbump was somehow null. how??? why???");
			// THIS FUNCTION CAN,, AS IN IF IT DELETES OBSTACLES INSIDE OF IT, I AM FUCKED?????
			// ITS PATHETIC I DIDNT SEE THIS EARLIER
			
			// pathetic half fix
			// this is REALLY bad.
			/*switch((*it)->entityType()) {
				//case EntityType::Interactable:
				//	continue;
				//case EntityType::Chest:
				//	break;
				default:
					break;
			}
			*/
			
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
		//BN_LOG("loop");
		// this assert shouldnt be needed, but my god have i been having some whack ass bugs
		//BN_ASSERT(*it != NULL, "in hasenemy, a NULL was found???? how????");
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
	//if(temp == 1 || temp == 12) {
	if(temp == 0 || temp == 1 || temp == 12) {
		return false;
	}
	
	return true;
}

bool EntityManager::hasNonInteractableObstacle(const Pos& p) const { //nice function name (curse)

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

bn::optional<Direction> EntityManager::canSeePlayer(const Pos& p) const { profileFunction();
	
	// ISSUE, DOES THIS GO OFF OF THE PLAYERS POS, OR START POS?? UGH
	// this totally means that just like diamonds, chesters and bulls are going to have weird shit
	// and need to have their moves queued at the,,, start with,,, mimics and diamonds?
	
	Pos playerPos = player->p;
	//Pos playerPos = playerStart;
	//BN_ASSERT(getMap(player->p).contains(player), "wtf");
	
	// first, do some inexpensive checks
	if(!(playerPos.x == p.x || playerPos.y == p.y)) {
		return bn::optional<Direction>();
	}
	
	bool checkY = playerPos.x == p.x;
	//bool checkX = playerPos.y == p.y;
	
	int sharedValue = checkY ? p.x : p.y;
	//int startValue = checkY ? MIN(playerPos.y, p.y) + 1 : MIN(playerPos.x, p.x) + 1;
	//int stopValue = checkY ? MAX(playerPos.y, p.y) - 1 : MAX(playerPos.x, p.x) - 1;
	
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
		
		//if(testPos == playerStartPos) {
		if(testPos == playerStart) {
			
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
	// do iterators like (curse) performance as well?
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



