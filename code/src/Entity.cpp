
#include "Entity.h"

#include "entityManager.h"

#include "Game.h"

// this is the only way i could get this to work. why is this syntax like this??
Palette* Sprite::spritePalette = &defaultPalette;


int LevStatue::rodUses = 0;
int LevStatue::totalLev = 0;

void Entity::isDead() {

}

// Player

void Player::pushRod(Pos tilePos) {

	// pick tile up

	FloorTile* tile = globalGame->tileManager.floorMap[tilePos.x][tilePos.y];

	rod.push_back(globalGame->tileManager.floorMap[tilePos.x][tilePos.y]);

	globalGame->tileManager.floorMap[tilePos.x][tilePos.y] = NULL;

	globalGame->entityManager.rodUse();
	globalGame->tileManager.updateTile(tilePos);
	globalGame->tileManager.updateRod();

	bn::sound_items::snd_voidrod_store.play();
	globalGame->effectsManager.voidRod(tilePos, currentDir);

	globalGame->cutsceneManager.disCrash(tile, true);
}

void Player::popRod(Pos tilePos) {

	// put tile down

	FloorTile* tempTile = rod.back();
	rod.pop_back();

	tempTile->tilePos = tilePos;
	globalGame->tileManager.floorMap[tilePos.x][tilePos.y] = tempTile;

	globalGame->entityManager.rodUse();
	globalGame->tileManager.updateTile(tilePos);
	globalGame->tileManager.updateRod();

	bn::sound_items::snd_voidrod_place.play();
	globalGame->effectsManager.voidRod(tilePos, currentDir);

	globalGame->cutsceneManager.disCrash(tempTile, false);
}

bool Player::inRod(FloorTile* tile) {
	// trashCode

	if(tile == NULL) {
		return false;
	}

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


	// i rlly should of just had a global.
	// or a func that checks this for me
	// ACTUALLY,,,,,,,
	// if domove is only called on press couldnt i just do a if pressed or held?
	// no, bc what if,, they are seperate buttons
	// fuck it, ill make a func in game
	// actually no, im making it global


	//if(bn::keypad::a_pressed()) {
	if(getInput(bn::keypad::key_type::A)) {

		Pos tilePos(p);

		bool moveRes = tilePos.move(currentDir);

		if(!moveRes) {
			//globalGame->effectsManager.createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			globalGame->effectsManager.questionMark();
			return {false, bn::optional<Direction>()};
		}

		if(globalGame->entityManager.hasCollision(tilePos)) {
			//globalGame->effectsManager.createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			globalGame->effectsManager.questionMark();
			return {false, bn::optional<Direction>()};
		}

		if(globalGame->entityManager.hasObstacle(tilePos)) {
			// do dialogue here
			// what abt npcs tho, (curse)
			// shadows not technically being enemies rlly (curse)s me

			Obstacle* temp = static_cast<Obstacle*>(*(globalGame->entityManager.getMap(tilePos).begin()));

			if(temp->entityType() == EntityType::Interactable || temp->entityType() == EntityType::Chest ||
				(hasMemory && !inRod(globalGame->tileManager.memoryTile))) {
				temp->interact();
			} else {
				globalGame->effectsManager.questionMark();
			}

			return {false, bn::optional<Direction>()};
		}

		// if there is a entity in this tile, this is an invalid move(unless sword!).

		if(globalGame->entityManager.hasEntity(tilePos)) {
			if( (hasSword && !inRod(globalGame->tileManager.swordTile)) ||
				hashString("rm_0172\0") == globalGame->roomManager.currentRoomHash()
				) {
				BN_ASSERT(globalGame->entityManager.getMap(tilePos).size() == 1, "when killing an entity, there were multiple entitys in the tilepos??");

				Entity* tempEntity = *(globalGame->entityManager.getMap(tilePos).begin());
				if(tempEntity->entityType() != EntityType::Shadow) {

					if(hashString("rm_0172\0") == globalGame->roomManager.currentRoomHash()) {
						// i rlly rlly hope that doing,,, this call so deep inside of the move code doesnt
						// mess anything up during vblank
						globalGame->cutsceneManager.mimicTalk();

						globalGame->entityManager.removeEntity(tempEntity);
						globalGame->effectsManager.sword(tilePos, currentDir);

						return {true, bn::optional<Direction>()};
					}

					tempEntity->deathReason = EntityType::Player;

					globalGame->entityManager.killEntity(tempEntity);

					// ADD STEPOFF HERE?
					globalGame->tileManager.stepOffs.insert(tilePos);

					globalGame->effectsManager.sword(tilePos, currentDir);

					globalGame->entityManager.futureEntityMap[tilePos.x][tilePos.y] = globalGame->entityManager.entityMap[tilePos.x][tilePos.y];
					//futureEntityMap[tilePos.x][tilePos.y] = entityMap[tilePos.x][tilePos.y];

					return {true, bn::optional<Direction>()};
				}
			}

			//globalGame->effectsManager.createEffect(p-Pos(0, 1), EffectTypeCast(questionMark));
			globalGame->effectsManager.questionMark();
			return {false, bn::optional<Direction>()};
		}

		// do the tile swap.

		FloorTile* tile = globalGame->tileManager.floorMap[tilePos.x][tilePos.y];

		if(hasRod || hasSuperRod) {
			if(tile == NULL && rod.size() != 0) {
				// put tile down
				popRod(tilePos);
			} else if(tile != NULL && (rod.size() == 0 || hasSuperRod)) {
				// pick tile up
				pushRod(tilePos);
			} else if(tile == NULL && rod.size() == 0) {
				globalGame->effectsManager.questionMark();
				return {false, bn::optional<Direction>()};
			} else if (tile != NULL && rod.size() != 0) {
				globalGame->effectsManager.questionMark();
				return {false, bn::optional<Direction>()};
			}
		} else {
			globalGame->effectsManager.questionMark();
		}

		nextMove = bn::optional<Direction>();

		return {true, bn::optional<Direction>()};
	}

	nextMove = bn::optional<Direction>();

	//if(bn::keypad::down_pressed()) {
	if(getInput(bn::keypad::key_type::DOWN)) {
		currentDir = Direction::Down;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	} else if(getInput(bn::keypad::key_type::UP)) {
		currentDir = Direction::Up;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	} else if(getInput(bn::keypad::key_type::LEFT)) {
		currentDir = Direction::Left;
		nextMove = bn::optional<Direction>(currentDir);
		//return {true, bn::optional<Direction>(currentDir)};
	} else if(getInput(bn::keypad::key_type::RIGHT)) {
		currentDir = Direction::Right;
		nextMove = bn::optional<Direction>(currentDir);
	}

	if(!nextMove.has_value()) {
		return {false, bn::optional<Direction>()};
	}

	// do sweat anim here.
	// we can do this without needing to vblank, since it is meant to hold up execution

	Pos tempPos = p;
	if(!tempPos.move(currentDir) || globalGame->entityManager.hasCollision(tempPos)) {
		globalGame->playSound(&bn::sound_items::snd_push_small);
		return {true, bn::optional<Direction>(currentDir)};
	}

	if( ((wingsUse == hasWings) || hasWingsTile) && !globalGame->entityManager.hasFloor(tempPos) && !globalGame->entityManager.hasCollision(tempPos) && !globalGame->entityManager.hasEntity(tempPos)) {

	   BN_LOG("doing sweat!");

		// spr_sweat

		globalGame->entityManager.shouldTickPlayer = false;

		// i should rlly probs not be using this coord system
		bn::fixed xVal = sprite.screenx;
		bn::fixed yVal = sprite.screeny;

		Direction invertDirections[4] = {Direction::Down, Direction::Up, Direction::Right, Direction::Left};

		Direction stopDir = invertDirections[static_cast<int>(currentDir)];

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

		bn::fixed factor = 1.0;
		int tickAmount = (8.0 / factor).ceil_integer();

		Effect* sweatEffect = globalGame->effectsManager.generateSweatEffect();

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
				globalGame->doButanoUpdate();
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

		globalGame->effectsManager.removeEffect(sweatEffect);
		globalGame->entityManager.shouldTickPlayer = true;

		if(playerStop) {
			currentDir = currentDirBackup;
			sprite.updatePosition(p);
			doTick();
			nextMove = bn::optional<Direction>();
			return {false, bn::optional<Direction>()};
		}
	}


	// check if we are bumping into an obstacle, and if so, set the playerPush var
	tempPos = p;
	if(tempPos.move(currentDir) && globalGame->entityManager.hasObstacle(tempPos)) {
		BN_ASSERT(globalGame->entityManager.getMap(tempPos).size() == 1, "when deciding what the player pushed, something was wrong");
		globalGame->entityManager.playerPush = *(globalGame->entityManager.getMap(tempPos).begin());
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
	// nope, im just going to use the playermoved bool from the globalGame->entityManager, (curse) it
	// nope, player->p == playerStart
	// gods this is so dumb

	tileIndex = static_cast<int>(currentDir) + (4 * !!pushAnimation);

	BN_ASSERT(tileIndex < spriteTilesArray.size(), "tried loading a tileIndex out of the sprite array bounds! ", __PRETTY_FUNCTION__);
}

Player::Player(Pos p_) : Entity(p_) {
	spriteTilesArray.clear();

	switch(globalGame->mode) {
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

	// having duplicates causes so many problems, why did i not just read from the savedata var
	// i do not like that we set these vars here!
	locustCount = globalGame->saveData.locustCount;
	isVoided = globalGame->saveData.isVoided;
	if(globalGame->mode == 2) { // cif is always voided!
		isVoided = true;
	}

	hasMemory = globalGame->saveData.hasMemory;
	hasWings  = globalGame->saveData.hasWings;
	hasSword  = globalGame->saveData.hasSword;

	hasRod = globalGame->saveData.hasRod;
	hasSuperRod = globalGame->saveData.hasSuperRod;
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
	if(temp.has_value() && tempPos.move(temp.value()) && globalGame->entityManager.hasNonPlayerEntity(tempPos)) {
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
	//return globalGame->entityManager.canSeePlayer(p);

	bn::optional<Direction> temp = nextMove;
	nextMove.reset();

	// this MIGHT BE DUMB
	Pos tempPos = p;
	if(temp.has_value() && tempPos.move(temp.value()) && globalGame->entityManager.hasNonPlayerEntity(tempPos)) {
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
	if(tempPos.move(Direction::Up) && tempPos != globalGame->entityManager.player->p) {
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

	spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_chest_regular);

	// this is horrid, but sometimes chests spawn without any floor under them.
	// this fixes that.

	if(!globalGame->tileManager.hasFloor(p_)) {
		globalGame->tileManager.floorMap[p_.x][p_.y] = new FloorTile(p_);
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

	if(globalGame->roomManager.roomIndex > 3) {
		// this if statement is here on purpose for a very stupid easter egg, see the random boulder msgs
		// its 2 just in case, for future room ordering changes
		Pos playerPos = globalGame->entityManager.player->p;
		playerPos.move(Direction::Up);
		if(playerPos != p) {
			return;
		}
	}

	interactCount++;

	if(animationIndex == 0) {
		animationIndex = 1;

		doUpdate();

		bool isFirstLocust = globalGame->entityManager.player->locustCount == 0;

		bn::sound_items::snd_open.play();

		if(gotBonus) {
			globalGame->entityManager.player->locustCount+=3;
		} else {
			globalGame->entityManager.player->locustCount++;
		}

		globalGame->tileManager.updateLocust();
		// this isnt counted as a successful move, but we should still update locusts
		globalGame->tileManager.floorLayer.reloadCells();

		globalGame->entityManager.player->currentDir = Direction::Down;
		globalGame->entityManager.player->doUpdate(); // ensure the direction change is updated
		globalGame->effectsManager.locustGet(isFirstLocust);
	}
}

void Chest::specialBumpFunction() {
	if(!gotBonus && animationIndex == 0 && specialBumpCount == 3 && playerIdleFrame == playerIdleStart && (frame - playerIdleStart) >= 60 * 3) {
		specialBumpCount = 0;
		gotBonus = true;
		globalGame->effectsManager.chestBonus(this);
	}
};

// all strings must be double nulltermed!
constexpr const char* randomBoulderMessages[] = {
	"lmao if you are seeing this, i did something wrong\0",
	"i rlly hope this works\0",
	"Did you know every time you sigh, a little bit of happiness escapes?\0",
	"VOID look heres a bunch of text wow we even have scrolling\ntest1\ntest2\0",
	"please if you know anyone who has a job in CS, give them\nmy contact info, i need a job\0",
	"you might be able to still open the chest when you arent facing it\ndepending on if ive fixed that yet\0", // it would be funny to program in an exception for this chest only
};

constexpr bool checkAllBoulderMessages() {

	for(unsigned i=0; i<sizeof(randomBoulderMessages)/sizeof(randomBoulderMessages[0]); i++) {
		const char* str = randomBoulderMessages[i];

		while(*str) {
			str++;
		}

		str++;
		if(*str != '\0') {
			return false;
		}
	}

	return true;
}

static_assert(checkAllBoulderMessages(), "a random boulder message was not properly double nulltermed!");

void generateFunny(char* res) {

	bn::string<512> string;
	bn::ostringstream string_stream(string);

	string_stream << "oh gods please let this work\n";
	string_stream << "ok we chillin now\n";
	string_stream << "have you ever wanted to know the ram usage of you gba at this point in time?\rno?\rwell, luckily, i dont care\n";

	string_stream << "stack iwram: " << ((bn::fixed)bn::memory::used_stack_iwram()).safe_division(1024) << " KB\r";
	string_stream << "static iwram: " << ((bn::fixed)bn::memory::used_static_iwram()).safe_division(1024) << " KB\r";
	string_stream << "static ewram: " << ((bn::fixed)bn::memory::used_static_ewram()).safe_division(1024) << " KB\r";

	string_stream << "damnnnn im not a math doctor, but those numbers seem pretty high\rif only SOMEONE could program this better!\n";
	string_stream << "with that much ewram left over, you should just start shoving random things in it, fool";

	// i do not for the life of me understand why i cant get a properly nulltermed string out of the bn::string class
	int i=0;
	for(; i<string.size(); i++) {
		res[i] = string[i];
	}
	// i fucking knew it, it is my code which needs 2 nullterms for some reason?
	res[i] = '\0';
	res[i+1] = '\0';
}

void Boulder::interact() {

	if(specialDialogue != NULL) {
		globalGame->effectsManager.doDialogue(specialDialogue);
		return;
	}

	static int lastIndex = -1;

	int index = randomGenerator.get_int(0, sizeof(randomBoulderMessages) / sizeof(randomBoulderMessages[0]));
	if(globalGame->roomManager.roomIndex == 3 && lastIndex == -1) {
		index = 8;
		lastIndex = index;

		const char* temp = randomBoulderMessages[index];

		globalGame->effectsManager.doDialogue(temp);
		return;
	}

	constexpr int prevMsgStackSize = 16;
	BN_DATA_EWRAM static bn::vector<int, prevMsgStackSize> prevMsgStack(prevMsgStackSize, -1);

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

	const char* temp = randomBoulderMessages[index];

	if(index == 0) {
		char buffer[512];
		generateFunny(buffer);
		globalGame->effectsManager.doDialogue(buffer);
	} else {
		globalGame->effectsManager.doDialogue(temp);
	}
}

void Obstacle::moveSucceded() {
	globalGame->playSound(&bn::sound_items::snd_push);
	globalGame->removeSound(&bn::sound_items::snd_push_small);
	globalGame->effectsManager.smokeCloud(p, currentDir);
}

void Obstacle::moveFailed() {
	//game->playSound(&bn::sound_items::snd_push_small);
}

void EusStatue::isDead() {

	BN_ASSERT(globalGame->tileManager.floorMap[p.x][p.y] == NULL, "with a eus statue, you tried pushing it onto an area that i hadnt nulled yet(a glass that just broke, or something). im to lazy rn to fix this, but if you see it msg me");

	globalGame->tileManager.floorMap[p.x][p.y] = new FloorTile(p);

	globalGame->tileManager.updateTile(p);
}

bn::optional<Direction> GorStatue::getNextMove() {

	if(startPos == p) {
		return Obstacle::getNextMove();
	}

	bumpDirections.clear();
	return bn::optional<Direction>();
}

void GorStatue::moveSucceded() {
	tileIndex = 1;
}

bn::optional<Direction> MonStatue::getNextMove() {

	bn::optional<Direction> res = globalGame->entityManager.canSeePlayer(p);

	if(res.has_value()) {
		animationIndex = 1;
		doUpdate();
		globalGame->entityManager.addKill(this);
		globalGame->effectsManager.monLightning(p, res.value());
	}

	return Obstacle::getNextMove();
}

void TanStatue::isDead() {
	if(globalGame->tileManager.hasFloor(p)) { // i still have 0 clue if like,,, idek
		globalGame->tileManager.stepOff(p);
	}
}

LevStatue::~LevStatue() {
	if(activeEffect != NULL) {
		globalGame->effectsManager.removeEffect(activeEffect);
		activeEffect = NULL;
	}
}

void LevStatue::isDead() {
	totalLev--;
	if(isActive) {
		rodUses--;
		globalGame->entityManager.rodUse();
	}

	if(activeEffect != NULL) {
		globalGame->effectsManager.removeEffect(activeEffect);
		activeEffect = NULL;
	}

	if(rodUses != 0 && rodUses >= totalLev) {
		globalGame->entityManager.levKill = true;
	}
}

void LevStatue::activate() {

	BN_ASSERT(activeEffect == NULL, "a lev statues active effect wasnt null why trying to activate it??");

	activeEffect = globalGame->effectsManager.levStatueActive(this);

	isActive = true;
	rodUses++;
	doUpdate();
}

// -----

void AddStatue::interact() {
	globalGame->effectsManager.doDialogue("...\0\0");
}

void EusStatue::interact() {
	globalGame->effectsManager.doDialogue("My love...\0\0");
}

void BeeStatue::interact() {
	globalGame->effectsManager.doDialogue("Got any locusts?\0\0");
}

void TanStatue::interact() {
	globalGame->effectsManager.doDialogue("MAKE THEM GO AWAY\0\0");
}

void GorStatue::interact() {
	globalGame->effectsManager.doDialogue("How bothersome.\0\0");
}

void LevStatue::interact() {
    if(bruhRand() & 0xFF) {
        globalGame->effectsManager.doDialogue("One wrong move and I'll destroy you...\0\0");
    } else {
        bn::sound_items::egg.play();
        globalGame->effectsManager.doDialogue("One wrong move and I'll absolutely FUCK you up...\0\0");
    }
}

void CifStatue::interact() {
	globalGame->effectsManager.doDialogue("...\0\0");
}

void JukeBox::interact() {
	globalGame->effectsManager.doDialogue(" G R O O V Y !!\0\0");
}
