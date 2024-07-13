

#include "EntityManager.h"

#include "Game.h"
#include "TileManager.h"
#include "EffectsManager.h"


//SaneSet<Entity*, 4> entityMap[14][9];
//SaneSet<Entity*, 4> futureEntityMap[14][9];


// should, all managers (and game) be namespaces? it seems so

void EntityManager::loadEntities(EntityHolder* entitiesPointer, int entitiesCount) {

	posTracker.clear();

	menuOpened = false;

	playerPush = NULL;

	shouldTickPlayer = true;

	LevStatue::rodUses = 0;
	LevStatue::totalLev = 0;

	levKill = false;

	// conspiricy time, NULL EVERYTHING. i rlly should have overloaded delete and made it check
	// bc on a gba deleting a null isnt going to,, give me the error i desire
	killedPlayer.clear();
	shadowQueue.clear();
	kickedList.clear();

	if(player != NULL) {
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
	tanStatueList.clear();
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

		BN_ASSERT(!posSet.contains(tempPos), "the entitymangager tried loading in 2 entities on the same position: ", tempPos.x, " ", tempPos.y);
		posSet.insert(tempPos);

		if(i == 0) {
			BN_ASSERT(temp.t == EntityType::Player, "first entity loaded into a room MUST be the player");
		}

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
				entityList.insert(getBoulder(tempPos));
				break;
			case EntityType::Chest:
				entityList.insert(new Chest(tempPos));
				if(tileManager->floorMap[tempPos.x][tempPos.y] == NULL) {
					tileManager->floorMap[tempPos.x][tempPos.y] = new FloorTile(tempPos);
				}
				break;
			case EntityType::EmptyChest:
				entityList.insert(getEmptyChest(tempPos));
				if(tileManager->floorMap[tempPos.x][tempPos.y] == NULL) {
					tileManager->floorMap[tempPos.x][tempPos.y] = new FloorTile(tempPos);
				}
				break;
			case EntityType::AddStatue:
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
			case EntityType::JukeBox:
				entityList.insert(new JukeBox(tempPos));
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

			if(temp->entityType() == EntityType::TanStatue) {
				tanStatueList.insert(temp);
			}
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

	// SHOULD THIS DATA[text, not mechanics at least] be mined from the game?

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
			Interactable* inter = static_cast<Interactable*>(obj);

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
	Sprite::spritePalette = pal;
}

void EntityManager::addEntity(Entity* e) {

	BN_ASSERT(entityMap[e->p.x][e->p.y].size() == 0, "tried adding entity to non zero position");
	BN_ASSERT(futureEntityMap[e->p.x][e->p.y].size() == 0, "tried adding entity to non zero position");

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
	tanStatueList.clear();
	shadowList.clear();
	deadList.clear();

	player = NULL;
}

// -----

bool EntityManager::moveEntity(Entity* e, bool dontSet) { profileFunction();

	switch(e->entityType()) {
		case EntityType::Interactable:
			return false;
		case EntityType::Eye: // rm_0140, gives a ~.3 perf improvement
			return false;
		default:
			break;
	}

	bn::optional<Direction> nextMove = e->getNextMove();

	if(!nextMove) {
		e->moveFailed();
		return false;
	}

	Direction move = nextMove.value();

	Pos startPos = e->p;

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

	bn::pair<EntityType, bn::pair<Pos, Pos>> tempFloorStep(e->entityType(), bn::pair<Pos, Pos>(e->p, testPos));

	tileManager->floorSteps.push_back(tempFloorStep);

	//if(!dontSet) { // prev was above the !nextmove check, is this ok?
	posTracker.insert(startPos);
	//}

	posTracker.insert(testPos);

	e->currentDir = move;
	e->p = testPos;

	futureEntityMap[e->p.x][e->p.y].insert(e);

	e->moveSucceded();

	// why is this taking so much?? func resolution?? figure it out
	// undoing this saves some perf, but do we need it? and also it may (seems to be) messes up a lot of death poses, that was an issue for a hilw
	//e->doUpdate();
	// actually, elay player update, do every other as it happens?? maybe???
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

	// handling ("most of") all entity code in here instead of in each class is horrid
	// but trying to handle the ordering of everything just,,, ugh

	// return an entity if we died an need a reset
	bn::optional<Entity*> res;

	player->hasWingsTile = player->inRod(tileManager->wingsTile);
	bool prevHasWingsTile = player->hasWingsTile;

	// insert player's move into its func
	// IF THE PLAYER CHANGES THE FLOOR, DO IT HERE.
	bn::pair<bool, bn::optional<Direction>> playerRes = player->doInput();

	if(prevHasWingsTile && !player->inRod(tileManager->wingsTile)) {
		player->hasWingsTile = false;
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
	}

	Pos playerStartBackup = player->p;

	// do player move.
	playerStart = player->p;
	bool playerMoved = moveEntity(player);

	// why is this here?? i guess i didnt want to do a full updatemap call??
	entityMap[playerStart.x][playerStart.y] = futureEntityMap[playerStart.x][playerStart.y];
	entityMap[player->p.x][player->p.y] = futureEntityMap[player->p.x][player->p.y];

	// a small check is needed here tho for if we walked backward into a shadow,, or just any entity?
	if(playerMoved && getMap(player->p).size() >= 2) {
		SaneSet<Entity*, 4> tempMap = getMap(player->p);
		tempMap.erase(player);
		BN_ASSERT(tempMap.begin() != tempMap.end(), "something has gone horribly wrong with the backwards walk into shadow detection");
		addKill(*tempMap.begin());
		return;
	}

	// IF WE ARE ONE TILE AWAY FROM EXIT, AND SHADOWS ARE ON BUTTONS, WE DO NOT LEAVE THE LEVE
	// still tho, calling doFloorSteps will update the shadows, which is needed
	// pushanims were created before the effect system, and rlly need to be made into one

	player->pushAnimation = player->p == playerStart;
	if(player->p != playerStart) {
		player->doUpdate(); // previously, the player would update their direction after falling, this fixes that
	} else {
		Pos temp = playerStart;
		if(playerRes.second.has_value() && temp.move(player->currentDir) && hasNonInteractableObstacle(temp)) {
			game->playSound(&bn::sound_items::snd_push_small);
		}
	}

	// the playermoved bool is basically useless
	// i think this might mean that you will never die if you,, ugh idek
	// i fucking hate this so much :)
	// i think i have 3 or 4 variables dedicated to just seeing if the player moved
	// and now, trying to have,,, allowing buttons to be held might REALLY fuck me here
	// i could just,, have gloval variables but what if i forget to use those in the future?

	if(!getInput(bn::keypad::key_type::A)) {
		playerMoveCount++;
	}

	tileManager->doFloorSteps();

	bn::optional<Direction> shadowMove = playerRes.second;
	if(player->p == playerStart) {
		shadowMove.reset();
	}

	updateMap();
	if(hasKills()) {
		return;
	}

	bn::optional<Direction> tempDir;


	for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
		switch((*it)->entityType()) {
			case EntityType::MonStatue:
				// mon statues werent (curse)ing like, working properly at the end of a tick, i think this fixes that
				tempDir = canSeePlayer((*it)->p);
				if(tempDir.has_value()) {
					// puttint this mon line after, the *it, is it a useafterfree?

					updateScreen();

					// TODO, HAVE MONS LIGHTNING TAKE UP THE MAIN THREAD!!!!!!
					// THIS IS HOW WE SOLVE THE SCREEN TRANSITION TIMING ISSUES
					effectsManager->monLightning((*it)->p, tempDir.value());
					delay(30);
					addKill(*it);
				}
				break;
			default:
				break;
		}
	}

	// should updatemap occur here?
	if(hasKills()) {
		return;
	}

	// THESE FUNCS/CODE NEED TO BE MOVED INTO THEIR ENTITIES
	// getnextmove was supposed to be the solution to this, but
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
				} else {
					tempDiamond->nextMove = canPathToPlayer(tempDiamond, player->p);
				}

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
		updateMap(); // this call may not be needed, but im not risking it
	}

	if(hasKills()) {
		return;
	}

	playerStart = player->p;

	if(playerPush != NULL) {
		moveEntity(playerPush);
		playerPush = NULL;
	}

	updateMap();
	if(hasKills()) {
		return;
	}

	Mimic* tempMimic = NULL;
	bn::optional<Direction> tempMimicNextMove;

	// obstacle collision is done via: if (not obstacle) and (not collision), do move
	for(auto it = enemyList.begin(); it != enemyList.end(); ++it) {
		switch((*it)->entityType()) {
			case EntityType::WhiteMimic:
			case EntityType::GrayMimic:
			case EntityType::BlackMimic:
				tempMimic = static_cast<Mimic*>(*it);
				tempMimicNextMove = tempMimic->nextMove;
				if(moveEntity(tempMimic, true)) {
					tempMimic->nextMove = tempMimicNextMove;
				}
				break;
			default:
				break;
		}
	}

	for(auto it = kickedList.begin(); it != kickedList.end(); ++it) { // unsure if this change is ok!
		if( (static_cast<Obstacle*>(*it))->bumpDirections.size() != 0 ) {
			moveEntity(*it);
		}
	}

	updateMap();
	if(hasKills()) {
		return;
	}

	// a chester, moving onto and destroying a section of bomb tiles with you on it, wasnt properly activiating wings?
	posTracker.insert(player->p);

	moveEnemies();

	updateMap();
	if(hasKills()) {
		return;
	}

	if(enemyList.size() == 0 && shadowList.size() == 0) {
		while(tanStatueList.begin() != tanStatueList.end()) {
			killEntity(*(tanStatueList.begin()));
		}
	}

	int levcount = 0;
	LevStatue* hasLevStatue = NULL;

	for(auto it = obstacleList.begin(); it != obstacleList.end(); ++it) {
		switch((*it)->entityType()) {
			case EntityType::MonStatue:
				// mon statues werent (curse)ing like, working properly at the end of a tick, i think this fixes that
				tempDir = canSeePlayer((*it)->p);
				if(tempDir.has_value()) {
					// puttint this mon line after, the *it, is it a useafterfree?

					updateScreen();

					// TODO, HAVE MONS LIGHTNING TAKE UP THE MAIN THREAD!!!!!!
					// THIS IS HOW WE SOLVE THE SCREEN TRANSITION TIMING ISSUES
					effectsManager->monLightning((*it)->p, tempDir.value());
					delay(30);
					addKill(*it);
				}
				break;
			case EntityType::LevStatue:
				levcount++;
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

	if(game->roomManager.isWhiteRooms()) {
		tileManager->updateWhiteRooms(playerStartBackup, player->p);
	}

	sanity();

	// insane i have had this here for so long, obvious performance improvement
	updateScreen();
}

// -----

bn::vector<Entity*, 4>::iterator EntityManager::killEntity(Entity* e) { profileFunction();

	BN_ASSERT(e != NULL, "killentity tried to kill a null??");

	if(e->entityType() == EntityType::Player || e->entityType() == EntityType::Shadow) {
		BN_ERROR("tried to kill either a player or a shadow or an interactable. thats not a thing you can do fool.");
	}

	if(e->entityType() == EntityType::Interactable) {
		return entityMap[e->p.x][e->p.y].end();
	}

	Pos tempPos = e->p;

	futureEntityMap[tempPos.x][tempPos.y].erase(e);
	bn::vector<Entity*, 4>::iterator res = entityMap[tempPos.x][tempPos.y].erase(e);

	entityList.erase(e);

	// should save some cycles?
	if(e->isObstacle()) {
		obstacleList.erase(e);
		if(e->entityType() == EntityType::TanStatue) {
			tanStatueList.erase(e);
		}
	}
	if(e->isEnemy()) {
		enemyList.erase(e);
	}

	deadList.insert(e);

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

void EntityManager::removeEntity(Entity* e) { // used for cutscenes, primarily the mimic talk in rm_0172
	Pos tempPos = e->p;

	futureEntityMap[tempPos.x][tempPos.y].erase(e);
	entityMap[tempPos.x][tempPos.y].erase(e);

	entityList.erase(e);
	obstacleList.erase(e);
	enemyList.erase(e);

	tanStatueList.erase(e);

	deadList.insert(e);
}

void EntityManager::killAllAddStatues() {

	// SHOULD THIS CHECK BE HERE??? BC LIKE,,, UGHH GODS
	// it was previously in vblank, but like, it seems like killing entities in vblank fucks shit up
	// i also was probs not updating the entitymaps properly.

	for(auto it = obstacleList.begin(); it != obstacleList.end(); ) {
		BN_ASSERT(*it != NULL, "wtf in killalladdstatues");
		if((*it)->entityType() == EntityType::AddStatue) {
			tileManager->stepOffs.insert((*it)->p);

			killEntity(*it);

			it = obstacleList.begin();
		} else {
			++it;
		}
	}

	tileManager->doFloorSteps();
}

void EntityManager::manageShadows(bn::optional<Direction> playerDir) { profileFunction();

	// IMPORTANT. SHADOWS WILL ALWAYS SPRING UP FROM THE TILE THEY WERE STEPPED ON FROM
	// HOWEVER, IF THERE IS ANOTHER SHADOW(POSSIBLY ANY ENTITY?) ON SAID TILE
	// IT WILL WAIT UNTIL CLEAR.
	// ADDITIONALLY, NEW SHADOWS WILL SPAWN AFTER OLD ONES MOVE

	BN_ASSERT(playerDir.has_value(), "manageshadows was called with a playerdir without a value??");

	Direction moveDir = playerDir.value();

	// this whole function is now O(1) (assuming const insertion times, which isnt the case[BUT WOULD BE IF I WASINT A IDIOT]). that is insane to me.

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

			shadowQueue.erase(shadowQueue.begin());

			// unsure if needed
			posTracker.insert(queuePos);

			effectsManager->shadowCreate(queuePos);
			Shadow* temp = new Shadow(queuePos);

			shadowList.push_back(temp);

			temp->animationIndex = shadowList.front()->animationIndex;

			entityList.insert(temp);

			futureEntityMap[queuePos.x][queuePos.y].insert(temp);
		}
	}
}

void EntityManager::updateMap() { profileFunction();

	// not constatly copying these two buffers between each other would be ideal

	if(posTracker.size() != 0) {
		for(auto posit = posTracker.cbegin(); posit != posTracker.cend(); ++posit) {

			int x = (*posit).x;
			int y = (*posit).y;

			entityMap[x][y] = futureEntityMap[x][y];
		}
	}

	tileManager->doFloorSteps();

	if(hasKills()) {
		return;
	}

	Entity* temp = NULL;

	if(posTracker.size() != 0) {
		for(auto posit = posTracker.cbegin(); posit != posTracker.cend(); ++posit) {

			int x = (*posit).x;
			int y = (*posit).y;

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

	sanity();
}


// -----

void EntityManager::doDeaths() {
	if(deadList.size() == 0) {
		return;
	}

	for(auto it = deadList.begin(); it != deadList.end(); ) {
		BN_ASSERT(*it != NULL, "a entity in deadlist was null, this should never happen!");
		Entity* temp = *it;
		delete temp;
		it = deadList.erase(it);
	}
}

void EntityManager::updateScreen() {
	// update all onscreen sprites
	// NOT EFFICIENT.
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
	updateScreen();
}

// -----

void EntityManager::hideForDialogueBox(bool vis, bool isCutscene) {

	int compareVal = isCutscene ? 0 : 6;

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

bool EntityManager::exitRoom() {
	// dying entities (including the player) should really all be converted to being effects (and they now have)
	BN_ASSERT(hasKills(), "entityManager exitroom called when there were no kills?");
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
		doTicks();
	}

	if(frame % 2 == 0) {
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
			BN_ASSERT(*it != NULL, "a specialbump was somehow null. how??? why???");
			static_cast<Obstacle*>((*it))->specialBumpFunction();
		}
	}
	doDeaths();
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

	if(temp == 0 || temp == 1 || temp == 12) {
		return false;
	}

	return true;
}

bool EntityManager::hasNonInteractableObstacle(const Pos& p) const {
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

	Pos playerPos = player->p;

	// first, do some inexpensive checks
	if(!(playerPos.x == p.x || playerPos.y == p.y)) {
		return bn::optional<Direction>();
	}

	bool checkY = playerPos.x == p.x;

	int sharedValue = checkY ? p.x : p.y;

	int startValue = checkY ? MIN(playerPos.y, p.y) + 1 : MIN(playerPos.x, p.x) + 1;
	int stopValue = checkY ? MAX(playerPos.y, p.y) - 1 : MAX(playerPos.x, p.x) - 1;

	for(int i=startValue; i<=stopValue; i++) {
		Pos testPos = checkY ? Pos(sharedValue, i) : Pos(i, sharedValue);
		if(hasCollision(testPos) || hasNonPlayerEntity(testPos)) {
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

	BN_ASSERT(playerPos != p, "why in tarnation do you have a diamond and player at the same pos????");

	static u8 visited[14][9];
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			visited[x][y] = 0;
		}
	}

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

			if(!hasNonPlayerEntity(test)) {
				queue.push_back(test);
				continue;
			}

		}
	}

	return bn::optional<Direction>();
}

bn::optional<Direction> EntityManager::canPathToPlayer(Diamond* e, Pos playerStartPos) {

	const Direction testDirections[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

	for(int i=0; i<4; i++) {
		Pos testPos = e->p;
		if(!testPos.move(testDirections[i])) {
			continue;
		}

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

	if(!foundLevStatue && temp != NULL) {
		levKill = true;
	}

	if(LevStatue::rodUses >= LevStatue::totalLev && LevStatue::rodUses != 0) {
		levKill = true;
	}
}

// -----

void EntityManager::sanity() const {

	// check that all data structures are holding up
	// kills performance, so is set to return by default

	return;

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
