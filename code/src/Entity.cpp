
#include "Entity.h"

#include "EntityManager.h"

#include "Game.h"

// this is the only way i could get this to work. why is this syntax like this??
Palette* Sprite::spritePalette = &defaultPalette;

// default initilization, gets overwritten in game.h
EntityManager* Entity::entityManager = NULL;
EffectsManager* Entity::effectsManager = NULL;
TileManager* Entity::tileManager = NULL;
CutsceneManager* Entity::cutsceneManager = NULL;
Game* Entity::game = NULL;


int LevStatue::rodUses = 0;
int LevStatue::totalLev = 0;

void Entity::isDead() {
	
	// more needs to be researched here 
	
	// obstacle and enemy collide, no stepoff,,,
	// enemy and enemy collide, no stepoff. 
	// in updatemap, should i,,,, oh gods. 
	// handle doFloorSteps AFTER the actual updatemap occurs?
	// or maybe i should remove updates for,, tiles under enemies who die?
	// UGH
	
	
	/*
	if(tileManager->hasFloor(p)) {
		tileManager->stepOff(p);
	}
	*/
}

// Player

//EffectTypeArray questionMark[] = {EffectType(bn::sprite_tiles_items::dw_spr_question_black, 9)};

void Player::pushRod(Pos tilePos) { 
	
	// pick tile up
	
	FloorTile* tile = tileManager->floorMap[tilePos.x][tilePos.y];
	
	rod.push_back(tileManager->floorMap[tilePos.x][tilePos.y]);
	
	tileManager->floorMap[tilePos.x][tilePos.y] = NULL;
	
	entityManager->rodUse();
	tileManager->updateTile(tilePos);
	tileManager->updateRod();
	
	bn::sound_items::snd_voidrod_store.play();
	effectsManager->voidRod(tilePos, currentDir);
	
	cutsceneManager->disCrash(tile, true);
	
}

void Player::popRod(Pos tilePos) {
	
	// put tile down 
	
	FloorTile* tempTile = rod.back();
	rod.pop_back();
	
	tempTile->tilePos = tilePos;
	tileManager->floorMap[tilePos.x][tilePos.y] = tempTile;

	entityManager->rodUse();
	tileManager->updateTile(tilePos);
	tileManager->updateRod();

	bn::sound_items::snd_voidrod_place.play();
	effectsManager->voidRod(tilePos, currentDir);
	
	cutsceneManager->disCrash(tempTile, false);
}

bool Player::inRod(FloorTile* tile) {
	// trashCode
	
	if(tile == NULL) {
		return false;
	}
	
	BN_ASSERT(tile != NULL, "you should never be calling inRod with a null tile");
	
	for(int i=0; i<rod.size(); i++) {
		if(tile == rod[i]) {
			return true;
		}
	}
	
	return false;
}

bn::pair<bool, bn::optional<Direction>> Player::doInput() {
	
	// if a direction was pressed, return that (true, dir)
	// if A was pressed, meaning we pick up a tile,,,, then (true, NULL)
	// should the move be invalid(picking up a tile with ENTITY on it, return false, null)
	
	Direction currentDirBackup = currentDir;
	
	if(bn::keypad::a_pressed()) {
		
		Pos tilePos(p);
		
		bool moveRes = tilePos.move(currentDir);
		
		if(!moveRes) {
			//effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			effectsManager->questionMark();
			return {false, bn::optional<Direction>()};
		}

		if(entityManager->hasCollision(tilePos)) {
			//effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			effectsManager->questionMark();
			return {false, bn::optional<Direction>()};
		}
		
		if(entityManager->hasObstacle(tilePos)) {
			// do dialogue here
			// what abt npcs tho, (curse)
			// shadows not technically being enemies rlly (curse)s me
			
			Obstacle* temp = static_cast<Obstacle*>(*(entityManager->getMap(tilePos).begin()));
			
			temp->interact();
			
			return {false, bn::optional<Direction>()};
		}
		
		// if there is a entity in this tile, this is an invalid move(unless sword!).
		
		if(entityManager->hasEntity(tilePos)) {
			if(hasSword && !inRod(tileManager->swordTile)) {
				BN_ASSERT(entityManager->getMap(tilePos).size() == 1, "when killing an entity, there were multiple entitys in the tilepos??");
				
				Entity* tempEntity = *(entityManager->getMap(tilePos).begin());
				if(tempEntity->entityType() != EntityType::Shadow) {
					entityManager->killEntity(tempEntity);
					
					// ADD STEPOFF HERE?
					tileManager->stepOffs.insert(tilePos);
					
					effectsManager->sword(tilePos, currentDir);
					
					entityManager->futureEntityMap[tilePos.x][tilePos.y] = entityManager->entityMap[tilePos.x][tilePos.y];
					
					return {true, bn::optional<Direction>()};
				}
			}
			
			
			//effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			effectsManager->questionMark();
			return {false, bn::optional<Direction>()};
		}
		
		// do the tile swap.
		
		FloorTile* tile = tileManager->floorMap[tilePos.x][tilePos.y];
		
		//BN_LOG("fhdjlf ", hasSuperRod);
		
		if(hasRod || hasSuperRod) {
			if(tile == NULL && rod.size() != 0) {
				// put tile down 
				popRod(tilePos);
			} else if(tile != NULL && (rod.size() == 0 || hasSuperRod)) {
				// pick tile up
				pushRod(tilePos);
			} else if(tile == NULL && rod.size() == 0) { 
				//effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
				effectsManager->questionMark();
				return {false, bn::optional<Direction>()};
			} else if (tile != NULL && rod.size() != 0) {
				//effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
				effectsManager->questionMark();
				return {false, bn::optional<Direction>()};
			}
		} else {
			//effectsManager->createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			effectsManager->questionMark();
		}

		nextMove = bn::optional<Direction>();
		
		return {true, bn::optional<Direction>()};
	}
	
	nextMove = bn::optional<Direction>();
	
	if(bn::keypad::down_pressed()) {
		currentDir = Direction::Down;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::up_pressed()) {
		currentDir = Direction::Up;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::left_pressed()) {
		currentDir = Direction::Left;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	} else if(bn::keypad::right_pressed()) {
		currentDir = Direction::Right;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	}
	
	if(!nextMove.has_value()) {
		return {false, bn::optional<Direction>()};
	}

	
	// do sweat anim here.
	// we can do this without needing to vblank, since it is meant to hold up execution
	
	Pos tempPos = p;
	if(!tempPos.move(currentDir) || entityManager->hasCollision(tempPos)) {
		game->playSound(&bn::sound_items::snd_push_small);
		return {true, bn::optional<Direction>(currentDir)};
	}
	
	if( ((wingsUse == hasWings) || hasWingsTile) && !entityManager->hasFloor(tempPos) && !entityManager->hasCollision(tempPos) && !entityManager->hasEntity(tempPos)) {
		
		BN_LOG("doing sweat!");
		
		// spr_sweat
		
		entityManager->shouldTickPlayer = false;
		
		// i should rlly probs not be using this coord system
		bn::fixed xVal = sprite.screenx;
		bn::fixed yVal = sprite.screeny;
		
		Direction invertDirections[4] = {Direction::Down, Direction::Up, Direction::Right, Direction::Left};
		
		Direction stopDir = invertDirections[static_cast<int>(currentDir)];
		
		// nice function name, (curse)
		auto didPlayerPressStopDir = [stopDir]() -> bool {
			if(bn::keypad::down_pressed() && stopDir == Direction::Down) {
				return true;
			} else if(bn::keypad::up_pressed() && stopDir == Direction::Up) {
				return true;
			} else if(bn::keypad::left_pressed() && stopDir == Direction::Left) {
				return true;
			} else if(bn::keypad::right_pressed() && stopDir == Direction::Right) {
				return true;
			}
			return false;
		};
		
		// direction is casted to int as up, down, left, right
		int xDiffs[4] = {0, 0, -1, 1};
		int yDiffs[4] = {-1, 1, 0, 0};
		
		int xDif = xDiffs[static_cast<int>(currentDir)];
		int yDif = yDiffs[static_cast<int>(currentDir)];
		
		xVal += 8 * xDif;
		yVal += 8 * yDif;
		
		//bn::fixed factor = 0.66;
		bn::fixed factor = 1.0;
		int tickAmount = (8.0 / factor).ceil_integer();

		Effect* sweatEffect = effectsManager->generateSweatEffect();
		
		sprite.spritePointer.set_x(xVal);
		sprite.spritePointer.set_y(yVal);
		
		doTick();
		
		bool playerStop = false;
		
		for(int i=0; i<20; i++) {
			
			for(int waitFrames=0; waitFrames<7; waitFrames++) {
				playerStop = didPlayerPressStopDir();
				if(playerStop) {
					break;
				}
				if(frame % 6 == 0) {
					doTick();
				}
				game->doButanoUpdate();
			}
			if(playerStop) {
				break;
			}
			//doTick();
			
			if(i < tickAmount) {
				xVal += xDif * factor;
				yVal += yDif * factor;
			
				sprite.spritePointer.set_x(xVal);
				sprite.spritePointer.set_y(yVal);
			} else {
				break;
			}
		}
		
		effectsManager->removeEffect(sweatEffect);
		entityManager->shouldTickPlayer = true;
		
		if(playerStop) {
			currentDir = currentDirBackup;
			sprite.updatePosition(p);
			doTick();
			nextMove = bn::optional<Direction>();
			return {false, bn::optional<Direction>()};
		}
	}

	return {true, bn::optional<Direction>(currentDir)};
}

bn::optional<Direction> Player::getNextMove() {
	bn::optional<Direction> temp = nextMove;
	nextMove.reset();
	return temp; 
}

void Player::updateTileIndex() {
	
	// there should of been a way to do this by,,, setting the bool in hasmoved, but that didnt seem to work.
	// now the bool is updated by movefailed and movesucceded
	// nope, im just going to use the playermoved bool from the entityManager, (curse) it
	// nope, player->p == playerStart
	// gods this is so dumb
	
	tileIndex = static_cast<int>(currentDir) + (4 * !!pushAnimation);
	
	BN_ASSERT(tileIndex < spriteTilesArray.size(), "tried loading a tileIndex out of the sprite array bounds! ", __PRETTY_FUNCTION__);
	
}

Player::Player(Pos p_) : Entity(p_) {
	spriteTilesArray.clear(); 
	
	switch(game->mode) {
		default:
		case 0:
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_up);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_down);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_left);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_right);
			
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_attack_u);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_attack_d);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_attack_l);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_player_attack_r);
			break;
		case 1:
		
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_up);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_down);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_left);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_right);
			
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_attack_u);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_attack_d);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_attack_l);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lil_attack_r);
		
			break;
		case 2:
		
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_up);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_down);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_left);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_right);
			
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_u_attack);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_d_attack);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_l_attack);
			spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cif_r_attack);
			break;
	}
	
	sprite.spritePointer.set_z_order(-1);
}


// Enemy

bn::optional<Direction> Enemy::getNextMove() {
	return bn::optional<Direction>(currentDir);
}

bn::optional<Direction> Bull::getNextMove() {
	
	bn::optional<Direction> temp = nextMove;
	nextMove.reset();
	
	// this MIGHT BE DUMB
	Pos tempPos = p;
	if(temp.has_value() && tempPos.move(temp.value()) && entityManager->hasNonPlayerEntity(tempPos)) {
		idle = true;
		return bn::optional<Direction>();
	}

	return temp;
}

void Bull::moveFailed() {
	idle = true;
}

bn::optional<Direction> Chester::getNextMove() {
	// wow. it really is that simple.
	//return entityManager->canSeePlayer(p);
	
	bn::optional<Direction> temp = nextMove;
	nextMove.reset();
	
	// this MIGHT BE DUMB
	Pos tempPos = p;
	if(temp.has_value() && tempPos.move(temp.value()) && entityManager->hasNonPlayerEntity(tempPos)) {
		idle = true;
		return bn::optional<Direction>();
	}

	return temp;
}

void Chester::moveFailed() {
	idle = true;
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

bool Obstacle::kicked() {
	
	// this might be(definitely is) bad for add statues, but for rn, im just hacking this together
	
	Pos tempPos = p;
	if(tempPos.move(Direction::Up) && tempPos != entityManager->player->p) {
		return true;
	}
	
	if(ABS(playerIdleFrame - playerIdleStart) > 60) {
		specialBumpCount = 0;
	}
	playerIdleStart = playerIdleFrame;
	
	specialBumpCount++;
		
	return true;
}

Chest::Chest(Pos p_, bool isEmpty) : Obstacle(p_) {
	spriteTilesArray.clear();
	spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_regular);
	
	// this is horrid, but sometimes chests spawn without any floor under them. 
	// this fixes that.
	
	if(!tileManager->hasFloor(p_)) {
		tileManager->floorMap[p_.x][p_.y] = new FloorTile(p_);
	}
	
	if(isEmpty) {
		animationIndex = 1;
	}
	
	doUpdate();
}

void Chest::interact() {
	
	// this is actually the only place im even using game in here
	// and i could(actually should? probs just pass the roomManager into here, but idk 
	// to be 100% real is that i need to go convert all that (curse) into namespaces, but i 
	// (curse)ing hate namespaces, and if i have to rewrite a bunch of my h files i will freak
	
	// ALL OF THESE FUNCS SHOULD OF BEEN LAMBDAS OMFG
	
	if(game->roomManager.roomIndex > 3) {
		// this if statement is here on purpose for a very stupid easter egg, see the random boulder msgs
		// its 2 just in case, for future room ordering changes
		Pos playerPos = entityManager->player->p;
		playerPos.move(Direction::Up);
		if(playerPos != p) {
			return;
		}
	}
	
	interactCount++;
	
	if(animationIndex == 0) {
		animationIndex = 1;
	
		doUpdate();
		
		bn::sound_items::snd_open.play();
	
		if(gotBonus) {
			entityManager->player->locustCount+=3;
		} else {
			entityManager->player->locustCount++;
		}
		tileManager->updateLocust();
		// this isnt counted as a successful move, but we should still update locusts
		tileManager->floorLayer.reloadCells();
		
		entityManager->player->currentDir = Direction::Down;
		
	}
	
}

void Chest::specialBumpFunction() {
	if(!gotBonus && animationIndex == 0 && specialBumpCount == 3 && playerIdleFrame == playerIdleStart && (frame - playerIdleStart) >= 60 * 3) {
		specialBumpCount = 0;
		gotBonus = true;
		effectsManager->chestBonus(this);
	}
}; 

// why doesnt this work??
//const char* const randomBoulderMessages[] = {"jfdklsafs", "a", "123124", "VOID look heres a bunch of  text wow we even have scrolling\nbruh1\nbruh2"};

// i swear, why cant i do this without a struct? idek if the idek var is needed, but im not going to be changing it.
// messagestr struct used to be here, now in sharedtypes

// each string here has a limit of,, 64 non space chars in between lines(seperated with \n)
// i,, could do something to fix that though? but like ugh 
// idek 

#define MSGSTR(s) { s }
const MessageStr randomBoulderMessages[] = { 
	MSGSTR("i rlly hope this works\0"), 
	MSGSTR("Did you know every time you sigh, a little bit of happiness escapes?\0"), 
	MSGSTR("VOID look heres a bunch of text wow we even have scrolling\nbruh1\nbruh2\0"),
	MSGSTR("jesus christ i need a job\0"),
	MSGSTR("ugh\0"),
	MSGSTR("i firmly believe that cif is best lord\0"),
	MSGSTR("please if you know anyone who has a job in CS, give them\nmy contact info, i need a job\0"),
	MSGSTR("you might be able to still open the chest when you arent facing it\ndepending on if ive fixed that yet\0"), // it would be funny to program in an exception for this chest only
	MSGSTR("uwu\nuwuuwuwuwuwuwu\0"),
	MSGSTR("lev is so hot omgomgomgomgomgomg\0"),
	MSGSTR("i felt like there should be more messages here, but idk what to write\0"),
	MSGSTR("borerlining???\rboderlining???\rborderlining???\0"),
	MSGSTR("you should pay attention to the trees.\rtheres some eggs to be ate!\0"),
	MSGSTR("to this day, i swear that the compiler doesnt properly nullterm strings. or maybe its the string_view thingy in sprite_text_generator????? makes no sense\0"),
	MSGSTR("Play the milk games!\0"),
	MSGSTR("Play Celeste!\0"),
	MSGSTR("Watch the monogatari series!\0"),
	MSGSTR("Watch(and read, its getting super good, but ending soon) yofukashi no uta!\0"),
	MSGSTR("Listen to Bloody Mercury, i rlly like their music. Also Vafi, both are on spotify and youtube.\0"),
	MSGSTR("I really am sitting here typing messages no one will ever read.\nlol\0"),
	MSGSTR("ever think about how you are the human equivalent of a sponge?\ndoing nothing but sucking everything and everyone else up, giving nothing back.\nunless someone comes along and squeezes it out of you.\nparasite.\0")
};

void Boulder::interact() {
	
	static int lastIndex = -1;
	
	int index = randomGenerator.get_int(0, sizeof(randomBoulderMessages) / sizeof(randomBoulderMessages[0]));
	if(game->roomManager.roomIndex == 3 && lastIndex == -1) {
		index = 8;
		lastIndex = index;

		const char* temp = randomBoulderMessages[index].str;
			
		effectsManager->doDialogue(temp);
		return;
	}
	
	constexpr int prevMsgStackSize = 16;
	static bn::vector<int, prevMsgStackSize> prevMsgStack(prevMsgStackSize, -1);
	
	int i=0;

	retry:
	
	index = randomGenerator.get_int(0, sizeof(randomBoulderMessages) / sizeof(randomBoulderMessages[0]));
	
	i=0;
	for(; i<prevMsgStackSize; i++) {
		if(prevMsgStack[i] == index) {
			goto retry;
		}
	}
	
	prevMsgStack.erase(prevMsgStack.cbegin());
	prevMsgStack.push_back(index);
	
	lastIndex = index;

	const char* temp = randomBoulderMessages[index].str;
		
	effectsManager->doDialogue(temp);
}

void Obstacle::moveSucceded() {
	game->playSound(&bn::sound_items::snd_push);
	game->removeSound(&bn::sound_items::snd_push_small);
	effectsManager->smokeCloud(p, currentDir);
}

void Obstacle::moveFailed() {
	//game->playSound(&bn::sound_items::snd_push_small);
}

void EusStatue::isDead() {

	BN_ASSERT(tileManager->floorMap[p.x][p.y] == NULL, "with a eus statue, you tried pushing it onto an area that i hadnt nulled yet(a glass that just broke, or something). im to lazy rn to fix this, but if you see it msg me");

	tileManager->floorMap[p.x][p.y] = new FloorTile(p);

	// this is trash, please make tilemanager update based on tile
	// oh wow, i forgot that i wrote this method so long ago it was before i had the new tilesystem
	//tileManager->fullDraw();
	tileManager->updateTile(p);
	
	//Obstacle::startFall();
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
	
	bn::optional<Direction> res = entityManager->canSeePlayer(p);
	
	if(res.has_value()) {
		animationIndex = 1;
		doUpdate();
		entityManager->addKill(this);
		effectsManager->monLightning(p, res.value());
	}

	return Obstacle::getNextMove();
}

void TanStatue::isDead() {
	if(tileManager->hasFloor(p)) { // i still have 0 clue if like,,, idek
		tileManager->stepOff(p);
	}
}

LevStatue::~LevStatue() {	
	if(activeEffect != NULL) {
		effectsManager->removeEffect(activeEffect);
		activeEffect = NULL;
	}
}

void LevStatue::isDead() {
	totalLev--;
	if(isActive) {
		rodUses--;
		entityManager->rodUse();
	}
	
	if(activeEffect != NULL) {
		effectsManager->removeEffect(activeEffect);
		activeEffect = NULL;
	}
	

	if(rodUses != 0 && rodUses >= totalLev) {
		entityManager->levKill = true;
	}
	
}

void LevStatue::activate() {
	//animationIndex = 1;
	
	BN_ASSERT(activeEffect == NULL, "a lev statues active effect wasnt null why trying to activate it??");
	
	activeEffect = effectsManager->levStatueActive(this);
	
	isActive = true;
	rodUses++;
	doUpdate();
}

