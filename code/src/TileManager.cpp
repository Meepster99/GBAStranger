

#include "TileManager.h"
#include "EffectsManager.h"
#include "EntityManager.h"

#include "Game.h"

// -----

void Floor::setBigTile(int x, int y, int tile, bool flipX, bool flipY) {
	// flip the highest bit of the tile to get a details tile.
	// grabbing the size here each call may be expensive. i could set it to a int
	collisionPointer->setBigTile(x, y, tile + collisionTileCount, flipX, flipY);
}

void Floor::setTile(int x, int y, int tileIndex, bool flipX, bool flipY) {
	collisionPointer->rawMap.setTile(x, y, tileIndex + collisionTileCount, flipX, flipY);
}

void doFloorDraw() {

	auto& floorMap = globalGame->tileManager.floorMap;
	//auto& floorLayer = globalGame->tileManager.floorLayer;

	// is the floor map stored in iwram or ewram?
	// does, not doing this sequentially cause fuck ups with doing multiplication?

	for(int x=0; x<14; x++) {
		for(int y=0; y<8; y++) { // save a branch, also layer 8 isnt even needed dumbass bc thats the fuckin, UI layer, idiot
			if(floorMap[x][y] != NULL) {
				floorMap[x][y]->draw();
				if(floorMap[x][y+1] == NULL && floorMap[x][y]->drawDropOff()) {
					y++;
					FloorTile::drawDropOff(x, y);
				}
			}
		}
	}

	for(int x=0; x<14; x++) {
		if(floorMap[x][8] != NULL) {
			floorMap[x][8]->draw();
		}
	}

};

__attribute__((noinline, section(".ewram"))) void doWhiteRoomsFloorDraw() {

	// a reference requires lookup,, right? would using just a normal thing be better?
	const int playerX = globalGame->entityManager.player->p.x;
	const int playerY = globalGame->entityManager.player->p.y;

	const int startX = MAX(playerX - 2, 0);
	const int startY = MAX(playerY - 2, 0);

	const int endX = MIN(playerX + 2, 13);
	const int endY = MIN(playerY + 2, 8);

	auto& floorMap = globalGame->tileManager.floorMap;

	for(int x=startX; x<=endX; x++) {
		for(int y=startY; y<=endY; y++) {
			if(floorMap[x][y] != NULL) {

				floorMap[x][y]->draw();

				if( (y < 8) && (
				(y == endY) ||
				(floorMap[x][y]->drawDropOff() && floorMap[x][y+1] == NULL)
				)) {
					FloorTile::drawDropOff(x, y+1);
					y++;
				}
			}
		}
	}

	if(globalGame->tileManager.exitTile != NULL) {
		Pos exitPos = globalGame->tileManager.exitTile->tilePos;
		if(exitPos.move(Direction::Down)) {
			FloorTile::drawDropOff(exitPos.x, exitPos.y);
		}
	}
}

void Floor::draw(u8 (&collisionMap)[14][9], FloorTile* (&floorMap)[14][16]) {

	// THIS COULD, AND SHOULD BE OPTIMIZED INTO ONE LOOP OVER THE THING.
	// also the whole background doesnt need a redraw, only the stuff that changed

	// gods, is this like, if i didnt
	// does settings this here vs calling it in the func like
	// does putting it in the loop cause repeat calls?
	// the more i look at the compiler the more i realize im bad

	(void)collisionMap;
	(void)floorMap;


	//const auto& detailsMap = globalGame->detailsMap;
	//auto& tileManager = globalGame->tileManager;

	const bool isWhiteRooms = globalGame->roomManager.isWhiteRooms();

	//BN_LOG("whiterooms status ", isWhiteRooms);

	if(isWhiteRooms) {
		doWhiteRoomsFloorDraw();
	} else {
		doFloorDraw();
	}

	collisionPointer->rawMap.reloadCells();
}

void Floor::reloadCells() {
	collisionPointer->reloadCells();
}

// as for why these lambdas are here, putting them in loadtiles caused a compiler error
// not a "you did something wrong" error, a "please submit a bug report, something is wrong" error

auto memoryGetFunc = []() -> int {
	Player* player = globalGame->entityManager.player;

	BN_ASSERT(player != NULL, "in a spriteTileFunc, player was null");

	if(player->hasMemory) {
		return 51 + ( globalGame->mode == 2 ? 3 : 0) + 0;
	}

	return 57;
};

auto wingsGetFunc = []() -> int {
	Player* player = globalGame->entityManager.player;

	BN_ASSERT(player != NULL, "in a spriteTileFunc, player was null");

	if(player->hasWings) {
		return 51 + ( globalGame->mode == 2 ? 3 : 0) + 1;
	}

	return 57;
};

auto swordGetFunc = []() -> int {
	Player* player = globalGame->entityManager.player;

	BN_ASSERT(globalGame->entityManager.player != NULL, "in a spriteTileFunc, player was null");

	if(player->hasSword) {
		Pos tempPos = player->p;

		if(tempPos.move(player->currentDir) && globalGame->entityManager.hasEnemy(tempPos)) { // i need to add a check that this ISNT a mimic here, but im very very sleepy your honor.
			return 68 + (globalGame->mode == 2 ? 4 : 0) + ((frame % 16) / 4);
		}

		return 51 + ( globalGame->mode == 2 ? 3 : 0) + 2;
	}

	return 57;
};

// -----

void TileManager::loadTiles(u8* floorPointer, SecretHolder* secrets, int secretsCount, const char* exitDest) {

	bn::timer testTimer;
	bn::fixed tickCount;
	testTimer.restart();

	u8 uncompressedFloor[126];
	uncompressData(uncompressedFloor, floorPointer);

	stepOns.clear();
	stepOffs.clear();
	floorSteps.clear();

	if(entityManager->player != NULL) {
		Player* player = entityManager->player;

		for(int i=0; i<player->rod.size(); i++) {
			delete player->rod[i];
			player->rod[i] = NULL;
		}
		player->rod.clear();
	} else {
		BN_LOG("PLAYER NULL DURING LOADTILE\n\n\n\n");
	}

	exitTile = NULL;
	rodTile = NULL;
	locustTile = NULL;
	memoryTile = NULL;
	wingsTile  = NULL;
	swordTile  = NULL;
	floorTile1 = NULL;
	voidTile1 = NULL;

	int switchTracker = 0;

	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] != NULL) {
				delete floorMap[x][y];
			}

			floorMap[x][y] = NULL;

			Pos tempPos(x, y);

			switch(static_cast<TileType>(uncompressedFloor[x + 14 * y])) {
				case TileType::Pit:
					break;
				case TileType::Floor:
					floorMap[x][y] = new FloorTile(tempPos);
					break;
				case TileType::Glass:
					floorMap[x][y] = new Glass(tempPos);
					break;
				case TileType::Bomb:
					floorMap[x][y] = new Bomb(tempPos);
					break;
				case TileType::Death:
					floorMap[x][y] = new Death(tempPos);
					break;
				case TileType::Copy:
					floorMap[x][y] = new Copy(tempPos);
					break;
				case TileType::Exit:
					floorMap[x][y] = new Exit(tempPos);
					exitTile = static_cast<Exit*>(floorMap[x][y]);
					break;
				case TileType::Switch:
					floorMap[x][y] = new Switch(tempPos);
					switchTracker++;
					break;
				case TileType::HalfBomb:
					floorMap[x][y] = new Bomb(tempPos);
					static_cast<Bomb*>(floorMap[x][y])->charge = 1;
					break;
				default:
					BN_ERROR("unknown tile tried to get loaded in, wtf. id=", uncompressedFloor[x + 14 * y]);
					break;
			}
		}
	}

	Switch::pressedCount = 0; // is this,,, ok? TODO, SEE IF THIS CAUSES ISSUES
	BN_ASSERT(Switch::pressedCount == 0, "after loading in new tiles, the number of pressed tiles wasnt 0? Switch::pressedCount = ", Switch::pressedCount);
	BN_ASSERT(Switch::totalCount == switchTracker, "after loading in new tiles, the number of switches and number inside the class wasnt equal???");
	BN_LOG("whiteroomstatus is ", game->roomManager.isWhiteRooms());

	if(!game->roomManager.isWhiteRooms()) {
		// blank tile
		floorMap[0][8] = new WordTile(Pos(0, 8));

		if(globalGame->saveData.isVoided) { // void tiles
			floorMap[1][8] = voidTile1 = new WordTile(Pos(1, 8), 'V', 'O');
			floorMap[2][8] = new WordTile(Pos(2, 8), 'I', 'D');
		} else { // hp tiles
			floorMap[1][8] = voidTile1 = new WordTile(Pos(1, 8), 'H', 'P');
			WordTile* tempHPTile = new WordTile(Pos(2, 8), '0', '7');
			floorMap[2][8] = tempHPTile;
		}

		floorMap[3][8] = new WordTile(Pos(3, 8));
		floorMap[4][8] = locustTile = new LocustTile(Pos(4, 8));

		WordTile* tempLocustCounterTile = new WordTile(Pos(5, 8), ' ', ' ');
		floorMap[5][8] = tempLocustCounterTile;

		floorMap[6][8] = new RodTile(Pos(6, 8));
		rodTile = static_cast<RodTile*>(floorMap[6][8]);

		floorMap[7][8] = new WordTile(Pos(7, 8));

		memoryTile = new SpriteTile(Pos(8, 8), memoryGetFunc);
		floorMap[8][8] = memoryTile;

		wingsTile = new SpriteTile(Pos(9, 8), wingsGetFunc);
		floorMap[9][8] = wingsTile;

		swordTile = new SpriteTile(Pos(10, 8), swordGetFunc);
		floorMap[10][8] = swordTile;

		floorMap[11][8] = new WordTile(Pos(11, 8));

		// this should be changed. roommanager should just have a array with a 3 length char array for what floor number should be displayed(or ???)
		int roomIndex = game->roomManager.roomIndex;

		BN_ASSERT(roomIndex <= 999, "why in tarnation is the roommanager's roomindex greater than 999???");

		if(roomIndex <= 256) { // normal rooms
			floorMap[12][8] = new WordTile(Pos(12, 8), 'B', '0' + roomIndex / 100);
			floorMap[13][8] = new WordTile(Pos(13, 8), '0' + (roomIndex / 10) % 10, '0' + (roomIndex % 10));
		} else if(roomIndex <= 330) { // EX rooms. THIS IS INCORRECT, SOME EX ROOMS HAVE ??
			int tempRoomIndex = roomIndex - 257;
			floorMap[12][8] = new WordTile(Pos(12, 8), 'E', '0' + tempRoomIndex / 100);
			floorMap[13][8] = new WordTile(Pos(13, 8), '0' + (tempRoomIndex / 10) % 10, '0' + (tempRoomIndex % 10));
		} else { // misc other rooms
			floorMap[12][8] = new WordTile(Pos(12, 8), 'B', '?');
			floorMap[13][8] = new WordTile(Pos(13, 8), '?', '?');
		}

		floorTile1 = static_cast<WordTile*>(floorMap[12][8]);
	}

	exitDestination = exitDest;
	secretDestinations.clear();

	secretsCount--;
	secrets++;

	for(int i=0; i<secretsCount; i++) {

		Pos tempSecretPos(secrets->x, secrets->y);

		if(WTF(secrets->dest) == 2) {
			BN_LOG("numtile with val of ", secrets->dest);
			BN_ASSERT(floorMap[tempSecretPos.x][tempSecretPos.y] != NULL, "when loading a numtile, the floor under it was null. this should never occur!");
			delete floorMap[tempSecretPos.x][tempSecretPos.y];
			floorMap[tempSecretPos.x][tempSecretPos.y] = NULL;
			floorMap[tempSecretPos.x][tempSecretPos.y] = new WordTile(tempSecretPos, secrets->dest[0], secrets->dest[1]);
		} else {
			bn::pair<const char*, Pos> tempPair(secrets->dest, tempSecretPos);
			secretDestinations.push_back(tempPair);
		}

		secrets++;
	}

	if(game->roomManager.currentRoomHash() == hashString("rm_e_021\0")) {
		// this is the room which,, takes you back unless you have the right config
		// it will take you back to rm_e_020\0

		exitDestination = "rm_e_020\0";
	}

	// why in tarnation is this vblank needed for floor 136??
	// i dont like having this here and slowing down debug swapping, but ugh
	// if i want to do tas verification, i need to not have framedrops
	game->doButanoUpdate();

	tickCount = testTimer.elapsed_ticks();
	BN_LOG("tilemanager loadtiles ", tickCount.safe_division(FRAMETICKS), " frames");
}

int TileManager::checkBrandIndex(const unsigned (&testBrand)[6]) {

	int matchIndex = -1;

	unsigned matches[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	for(unsigned i=0; i<sizeof(allBrands) / sizeof(allBrands[0]); i++) {
		for(int j=0; j<6; j++) {
			if(testBrand[j] == allBrands[i][j]) {
				matches[i]++;
			}
		}
	}

	for(unsigned i=0; i<sizeof(matches) / sizeof(matches[0]); i++) {
		if(matches[i] == 6) {
			matchIndex = i;
			break;
		}
	}

	return matchIndex;
}

const char* TileManager::checkBrand() { profileFunction();

	static int prevMatchIndex = -1;

	switch(game->roomManager.roomIndex) {
		case 23:
		case 53:
		case 67:
		case 89:
		case 137:
		case 157:
		case 179:
		case 223:
		case 227:
			break;
		default:
			prevMatchIndex = -1;
			return NULL;
			break;
	}

	if(!entityManager->player->inRod(exitTile)) {
		// if the player doesnt have the exit tile, return
		cutsceneManager->cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);
		prevMatchIndex = -1;
		return NULL;
	}

	unsigned roomState[6] = {0, 0, 0, 0, 0, 0};

	for(int y=1; y<=6; y++) {
		unsigned temp = 0;
		for(int x=4; x<=9; x++) {
			temp = ((temp << 1) | !!hasFloor(x, y));
		}
		roomState[y-1] = temp;
	}

	int matchIndex = checkBrandIndex(roomState);

	const bn::regular_bg_item* lordBackgrounds[8] = {
	&bn::regular_bg_items::dw_spr_lordborders_index0,
	&bn::regular_bg_items::dw_spr_lordborders_index1,
	&bn::regular_bg_items::dw_spr_lordborders_index2,
	&bn::regular_bg_items::dw_spr_lordborders_index3,
	&bn::regular_bg_items::dw_spr_lordborders_index4,
	&bn::regular_bg_items::dw_spr_lordborders_index5,
	&bn::regular_bg_items::dw_spr_lordborders_index6,
	&bn::regular_bg_items::dw_spr_lordborders_index7
	};

	if(matchIndex != -1) {
		if(matchIndex != prevMatchIndex) {
			if(matchIndex < 8) {
				cutsceneManager->cutsceneLayer.rawMap.create(*lordBackgrounds[matchIndex], 1);
			} else {
				cutsceneManager->cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);
			}

			cutsceneManager->cutsceneLayer.rawMap.bgPointer.set_palette(game->pal->getBlackBGPalette());

			effectsManager->fadeBrand();
			prevMatchIndex = matchIndex;
		}

		return destinations[matchIndex];
	}
	cutsceneManager->cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);

	prevMatchIndex = matchIndex;
	return NULL;
}

TileManager::~TileManager() {

	if(entityManager->player != NULL) {
		Player* player = entityManager->player;

		for(int i=0; i<player->rod.size(); i++) {
			delete player->rod[i];
			player->rod[i] = NULL;
		}
		player->rod.clear();
	}

	exitTile = NULL;
	rodTile = NULL;
	locustTile = NULL;
	memoryTile = NULL;
	wingsTile  = NULL;
	swordTile  = NULL;

	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] != NULL) {
				delete floorMap[x][y];
			}
			floorMap[x][y] = NULL;
		}
	}
}

// -----

void TileManager::doFloorSteps() { profileFunction();

	// STEP UPDATES OCCUR AFTER DOING EXIT CHECKING,
	// SINCE IF EVERY BUTTON IS PRESSED, YOU HAVE ONE TICK TO EXIT
	// but,, what about shadows,, this is weird
	// THIS DELAYED PRESS THING ONLY WORKS IF IT WAS A NONPLAYER ENTITY PRESSING IT!!

	// CURRENTLY SINCE I WANTED TO STORE THE ENTITYTYPE, POSES ARE
	// NO LONGER UNIQUE

	bn::optional<Entity*> res;

	for(auto it = floorSteps.cbegin(); it != floorSteps.cend(); ++it) {

		BN_ASSERT((*it).second.first != (*it).second.second, "in doFloorSteps, calculating steps. why is a move here with the same start and end??");

		stepOffs.insert((*it).second.first);
		stepOns.insert((*it).second.second);
	}

	// stepoffs occur before stepons bc of shadows, and switches
	for(auto it = stepOffs.cbegin(); it != stepOffs.cend(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) { [[likely]]
			stepOff(tempPos);
		}
	}

	for(auto it = stepOns.cbegin(); it != stepOns.cend(); ++it) {
		Pos tempPos = *it;
		if(hasFloor(tempPos)) { [[likely]]
			stepOn(tempPos);
		}
	}

	stepOns.clear();
	stepOffs.clear();
	floorSteps.clear();

	if(hasFloor(entityManager->player->p) == TileType::Exit && Switch::pressedCount == Switch::totalCount) {
		entityManager->addKill(NULL);
	}

	// calling this here may be excessive!
	checkBrand();
}

void TileManager::updateTile(const Pos& p) {

	// i pray that like, i can optimize out the creation of the pos class in this

	const int x = p.x;
	const int y = p.y;

	if(floorMap[x][y] != NULL && !floorMap[x][y]->isAlive) {
		delete floorMap[x][y];
		floorMap[x][y] = NULL;
	}

	if(floorMap[x][y] == NULL) {
		// the collision check isnt needed, but im keeping it here just in case
		// i also could(and maybe should?) use the hascollison func.
		// idk abt speed but it would be good to standardize it, as im currently doing with the hasfloor func

		if(y > 0 && !hasCollision(Pos(x, y-1)) && hasFloor(x, y-1) && floorMap[x][y-1]->drawDropOff()) {
			FloorTile::drawDropOff(x, y);
		} else {
			FloorTile::drawPit(x, y);
		}

		// some issues could be caused here if this tile isnt freed yet.
		// rewrite all map calls to hasfloor
		if(y < 8 && !hasFloor(x, y+1) && !hasCollision(Pos(x, y+1))) {
			FloorTile::drawPit(x, y+1);
		}
	} else {
		floorMap[x][y]->draw();
		if(floorMap[x][y]->drawDropOff() && y < 8 && !hasFloor(x, y+1) && !hasCollision(Pos(x, y+1))) {
			FloorTile::drawDropOff(x, y+1);
		}
	}

	floorLayer.reloadCells();
}

void TileManager::updateExit() {
	if(exitTile == NULL) {
		return;
	}
	if(entityManager->player->inRod(exitTile)) {
		return;
	}
	updateTile(exitTile->tilePos);
}

void TileManager::updateRod() {
	// i dont like the way these funcs are being coded tbh
	if(rodTile == NULL) {
		return;
	}
	if(entityManager->player->inRod(rodTile)) {
		return;
	}

	updateTile(rodTile->tilePos);
}

void TileManager::updateLocust() {

	if(locustTile != NULL && !entityManager->player->inRod(locustTile)) {
		updateTile(locustTile->tilePos);
	}

	if(locustTile != NULL) {
		Pos tempTilePos = locustTile->tilePos;
		if(tempTilePos.move(Direction::Right) && hasFloor(tempTilePos) == TileType::WordTile) {
			// the pos to the right of this tile is valid
			// the pos to the right has a tile which is a word tile. ALL word tiles should be kept in numberTiles, except floortile1

			// i seriously NEVER made a gettile func??? fom pos?
			WordTile* locustCounterTile = static_cast<WordTile*>(floorMap[tempTilePos.x][tempTilePos.y]);

			locustCounterTile->first = '0' + ((entityManager->player->locustCount / 10) % 10);
			locustCounterTile->second = '0' + (entityManager->player->locustCount % 10);

			updateTile(tempTilePos);
		}
	}
}

void TileManager::updateVoidTiles() {

	if(entityManager->player == NULL) {
		// just run. i REALLY should have just read from the gamesave instead of player all the time
		return;
	}

	bool isVoided = entityManager->player->isVoided;

	if(voidTile1 != NULL && !entityManager->player->inRod(voidTile1)) {

		voidTile1->first = isVoided ? 'V' : 'H';
		voidTile1->second = isVoided ? 'O' : 'P';

		updateTile(voidTile1->tilePos);
	}

	Pos tempPos = Pos(2, 8);
	if(hasFloor(tempPos) == TileType::WordTile) {

		WordTile* voidTile2 = static_cast<WordTile*>(floorMap[tempPos.x][tempPos.y]);

		voidTile2->first = isVoided ? 'I' : '0';
		voidTile2->second = isVoided ? 'D' : '7';

		updateTile(voidTile2->tilePos);
	}
}

void TileManager::updateBurdenTiles() {

	if(memoryTile != NULL && !entityManager->player->inRod(memoryTile)) {
		updateTile(memoryTile->tilePos);
	}

	if(wingsTile != NULL && !entityManager->player->inRod(wingsTile)) {
		updateTile(wingsTile->tilePos);
	}

	if(swordTile != NULL && !entityManager->player->inRod(swordTile)) {
		updateTile(swordTile->tilePos);
	}

}

int TileManager::getLocustCount() {

	if(locustTile == NULL || entityManager->player->inRod(locustTile)) {
		return -1;
	}

	Pos locustPos = locustTile->tilePos;
	if(!locustPos.move(Direction::Right)) {
		return -1;
	}

	if(hasFloor(locustPos) != TileType::WordTile) {
		return -1;
	}

	WordTile* locustCounterTile = static_cast<WordTile*>(floorMap[locustPos.x][locustPos.y]);
	return locustCounterTile->getNumber();
}

int TileManager::getRoomIndex() {

	if(floorTile1 == NULL) {
		return -1;
	}

	char temp = floorTile1->second;
	if(temp == '?') {
		return -1;
	}

	if(hasFloor(Pos(13, 8)) != TileType::WordTile) {
		return -1;
	}

	WordTile* floorTile2 = static_cast<WordTile*>(floorMap[13][8]);

	int res = ((floorTile1->second - '0') * 100) + floorTile2->getNumber();

	if(floorTile1->first == 'E') {
		res += 257;
	}

	return res;
}

bool TileManager::hasCollision(const Pos& p) {
	// should this even be here?
	return entityManager->hasCollision(p);
}

void TileManager::updateWhiteRooms(const Pos& startPos, const Pos& currentPos) {

	/*

	this code is responsible for having tiles appear and dissapear in whiterooms
	lots of edge cases, should be cleaned up!

	*/

	if(startPos == currentPos) {
		return;
	}

	// assuming this dir, is,,, bad.
	// i should get it from the dir, but im fucking tired
	Direction dir = globalGame->entityManager.player->currentDir;
	Direction ortho;

	switch(dir) {
		case Direction::Up:
		case Direction::Down:
			ortho = Direction::Left;
			break;
		case Direction::Left:
		case Direction::Right:
			ortho = Direction::Up;
			break;
		default:
			BN_ERROR("no");
			break;
	}

	Pos drawPos = startPos;
	drawPos.move(dir);
	drawPos.move(dir);
	drawPos.move(dir);

	drawPos.move(ortho);
	drawPos.move(ortho);

	Pos erasePos = currentPos;
	erasePos.moveInvert(dir, true, true);
	erasePos.moveInvert(dir, true, true);
	erasePos.moveInvert(dir, true, true);

	erasePos.move(ortho);
	erasePos.move(ortho);
	//erasePos.move(ortho);

	auto doDraw = [this, &dir](const Pos& p) -> void {

		int x = p.x;
		int y = p.y;

		if(floorMap[x][y] != NULL && !floorMap[x][y]->isAlive) {
			delete floorMap[x][y];
			floorMap[x][y] = NULL;
		}

		if(floorMap[x][y] != NULL) {

			floorMap[x][y]->draw();

			if(y < 8) {
				switch(dir) {
					case Direction::Up:
						if(floorMap[x][y+1] == NULL) {
							FloorTile::drawDropOff(x, y+1);
						}
						break;
					case Direction::Down:
					case Direction::Left:
					case Direction::Right:
						FloorTile::drawDropOff(x, y+1);
						break;
					default:
						BN_ERROR("unreachable");
						break;
				}
			}
		}
	};

	auto doClear = [this, &ortho, &dir](const Pos& p) -> void {

		int x = p.x;
		int y = p.y;

		FloorTile::drawPit(x, y);

		switch(dir) {
			case Direction::Left:
			case Direction::Right:
				if(y < 8 && floorMap[x][y] != NULL) {
					FloorTile::drawPit(x, y+1);
				}
				break;
			case Direction::Up:
				if(y < 8) {
					FloorTile::drawPit(x, y + 1);
				}
				if(y > 0 && floorMap[x][y-1] != NULL) {
					FloorTile::drawDropOff(x, y);
				}
				break;
			case Direction::Down:
				if(y < 8 && floorMap[x][y+1] == NULL) {
					FloorTile::drawPit(x, y + 1);
				}
				break;
			default:
				BN_ERROR("unreachable");
				break;
		}

	};


	Pos iteratorPos = currentPos;
	iteratorPos.move(ortho);
	iteratorPos.move(ortho);

	Pos stopPos = currentPos;
	stopPos.moveInvert(ortho, true, true);
	stopPos.moveInvert(ortho, true, true);

	while(true) {

		doClear(erasePos);

		doDraw(drawPos);

		if(iteratorPos == stopPos) {
			break;
		}

		drawPos.moveInvert(ortho, true, true);
		erasePos.moveInvert(ortho, true, true);
		iteratorPos.moveInvert(ortho, true, true);
	}

	updateExit();

	// goofy ahh shit code
	if(globalGame->tileManager.exitTile != NULL) {
		Pos exitPos = globalGame->tileManager.exitTile->tilePos;
		if(exitPos.move(Direction::Down)) {

			// check if the dropoff is within 2 units of the player
			if(ABS(currentPos.x - exitPos.x) <= 2 && ABS(currentPos.y - exitPos.y) <= 2) {
				if(floorMap[exitPos.x][exitPos.y] != NULL) {
					floorMap[exitPos.x][exitPos.y]->draw();
				}
			} else {
				FloorTile::drawDropOff(exitPos.x, exitPos.y);
			}
		}
	}

	floorLayer.reloadCells();
}

void TileManager::fullDraw() {

	//BN_LOG("tileManager layer draw");
	floorLayer.draw(game->collisionMap, floorMap);

	//BN_LOG("tileManager exit update");
	updateExit();
	//BN_LOG("tileManager rod update");
	updateRod();
	//BN_LOG("tileManager locust update");
	updateLocust();
	//BN_LOG("tileManager void update");
	updateVoidTiles();
	//BN_LOG("tileManager burden update");
	updateBurdenTiles();
}

bool TileManager::exitRoom() {
	return true;
}

bool TileManager::enterRoom() {
	return true;
}

void TileManager::doVBlank() { profileFunction();

	// ideally, keeping track of each tile type in its own list would be better, but this is const time 126

	const bool doDeathTileAnim = (bruhRand() & 0x3FF) == 0;

	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			if(floorMap[x][y] == NULL) {
				continue;
			}

			BN_ASSERT(floorMap[x][y]->isAlive, "a floor tile wasnt alive during vblank?? how??");

			if(floorMap[x][y]->isSteppedOn) {
				floorMap[x][y]->isSteppedOnAnimation();
			}

			switch(floorMap[x][y]->tileType()) {
				case TileType::Death: [[unlikely]]
					if(doDeathTileAnim) {
						effectsManager->deathTileAnimate(Pos(x, y));
					}
					break;
				default: [[likely]]
					break;
			}
		}
	}

	if(exitTile != NULL) {
		exitTile->isFirstCall = false;
	}

	if(swordTile != NULL && !entityManager->player->inRod(swordTile)) {
		updateTile(swordTile->tilePos);
	}

	return;
}

// -----

bn::optional<TileType> TileManager::hasFloor(const int x, const int y) {
	const FloorTile* temp = floorMap[x][y];

	if(temp == NULL || !temp->isAlive) {
		return bn::optional<TileType>();
	}

	return bn::optional<TileType>(temp->tileType());
}

void TileManager::stepOff(Pos p) {
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepoff on a tile, it was null?");
	floorMap[p.x][p.y]->stepOff();
}

void TileManager::stepOn(Pos p) {
	BN_ASSERT(floorMap[p.x][p.y] != NULL, "when stepon on a tile, it was null?");
	floorMap[p.x][p.y]->stepOn();
}
