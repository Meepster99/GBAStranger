
#include "Game.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

unsigned int frame = 0;
int playerIdleFrame = 0; // frame at which the player started idleing
bool isVblank = false;
unsigned boobaCount = 0;
unsigned playerMoveCount = 0;

bool debugToggle = false;


namespace Game {
	Collision* collision = NULL;
	//Details details(&collision);
	//Details details(NULL);
	Details* details = NULL;
	u8 collisionMap[14][9];
	u8 detailsMap[14][9];
	GameSave saveData;
	bn::timer miscTimer;
	GameState state = GameState::Loading;
	bool needRedraw = false;
	int mode = 0;
	const char* strangerNames[3] = {"Gray\0", "Lillie\0", "Cif\0"};
	int paletteIndex = 0;
	Palette* pal = NULL;
	int fadePaletteIndex = -1;
	SaneSet<const bn::sound_item*, MAXSOUNDS> queuedSounds;
	SaneSet<const bn::sound_item*, MAXSOUNDS*2> removedSounds;
};

void Game::changeMode(int val) {
	mode += val;
	mode = ((mode % 3) + 3) % 3;
	RoomManager::setMode(mode);
}

const char* Game::getMode() {
	return strangerNames[mode];
}

void Game::Game() {

	TileManager::TileManager(collision);

	FloorTile::floorLayer = TileManager::floorLayer;

	bn::regular_bg_tiles_ptr backgroundTiles = bn::regular_bg_tiles_ptr::allocate(896, bn::bpp_mode::BPP_4);

	collision->rawMap.bgPointer.set_tiles(backgroundTiles);
}

// -----

/*

import math

res = [ round(math.sin(math.radians(i)), 5) for i in range(0, 360) ]

print("bn::fixed sinTable[360] = {{{:s}}};".format(",".join([str(v) for v in res])))

*/

// i hope this doesnt slow compilation?
// ^^ i wrote this a while ago, i cant believe i said "will this slow compilation" when the rest of this trash code is ~15k lines long
bn::fixed sinTable[360] = {0.0,0.01745,0.0349,0.05234,0.06976,0.08716,0.10453,0.12187,0.13917,0.15643,0.17365,0.19081,0.20791,0.22495,0.24192,0.25882,0.27564,0.29237,0.30902,0.32557,0.34202,0.35837,0.37461,0.39073,0.40674,0.42262,0.43837,0.45399,0.46947,0.48481,0.5,0.51504,0.52992,0.54464,0.55919,0.57358,0.58779,0.60182,0.61566,0.62932,0.64279,0.65606,0.66913,0.682,0.69466,0.70711,0.71934,0.73135,0.74314,0.75471,0.76604,0.77715,0.78801,0.79864,0.80902,0.81915,0.82904,0.83867,0.84805,0.85717,0.86603,0.87462,0.88295,0.89101,0.89879,0.90631,0.91355,0.9205,0.92718,0.93358,0.93969,0.94552,0.95106,0.9563,0.96126,0.96593,0.9703,0.97437,0.97815,0.98163,0.98481,0.98769,0.99027,0.99255,0.99452,0.99619,0.99756,0.99863,0.99939,0.99985,1.0,0.99985,0.99939,0.99863,0.99756,0.99619,0.99452,0.99255,0.99027,0.98769,0.98481,0.98163,0.97815,0.97437,0.9703,0.96593,0.96126,0.9563,0.95106,0.94552,0.93969,0.93358,0.92718,0.9205,0.91355,0.90631,0.89879,0.89101,0.88295,0.87462,0.86603,0.85717,0.84805,0.83867,0.82904,0.81915,0.80902,0.79864,0.78801,0.77715,0.76604,0.75471,0.74314,0.73135,0.71934,0.70711,0.69466,0.682,0.66913,0.65606,0.64279,0.62932,0.61566,0.60182,0.58779,0.57358,0.55919,0.54464,0.52992,0.51504,0.5,0.48481,0.46947,0.45399,0.43837,0.42262,0.40674,0.39073,0.37461,0.35837,0.34202,0.32557,0.30902,0.29237,0.27564,0.25882,0.24192,0.22495,0.20791,0.19081,0.17365,0.15643,0.13917,0.12187,0.10453,0.08716,0.06976,0.05234,0.0349,0.01745,0.0,-0.01745,-0.0349,-0.05234,-0.06976,-0.08716,-0.10453,-0.12187,-0.13917,-0.15643,-0.17365,-0.19081,-0.20791,-0.22495,-0.24192,-0.25882,-0.27564,-0.29237,-0.30902,-0.32557,-0.34202,-0.35837,-0.37461,-0.39073,-0.40674,-0.42262,-0.43837,-0.45399,-0.46947,-0.48481,-0.5,-0.51504,-0.52992,-0.54464,-0.55919,-0.57358,-0.58779,-0.60182,-0.61566,-0.62932,-0.64279,-0.65606,-0.66913,-0.682,-0.69466,-0.70711,-0.71934,-0.73135,-0.74314,-0.75471,-0.76604,-0.77715,-0.78801,-0.79864,-0.80902,-0.81915,-0.82904,-0.83867,-0.84805,-0.85717,-0.86603,-0.87462,-0.88295,-0.89101,-0.89879,-0.90631,-0.91355,-0.9205,-0.92718,-0.93358,-0.93969,-0.94552,-0.95106,-0.9563,-0.96126,-0.96593,-0.9703,-0.97437,-0.97815,-0.98163,-0.98481,-0.98769,-0.99027,-0.99255,-0.99452,-0.99619,-0.99756,-0.99863,-0.99939,-0.99985,-1.0,-0.99985,-0.99939,-0.99863,-0.99756,-0.99619,-0.99452,-0.99255,-0.99027,-0.98769,-0.98481,-0.98163,-0.97815,-0.97437,-0.9703,-0.96593,-0.96126,-0.9563,-0.95106,-0.94552,-0.93969,-0.93358,-0.92718,-0.9205,-0.91355,-0.90631,-0.89879,-0.89101,-0.88295,-0.87462,-0.86603,-0.85717,-0.84805,-0.83867,-0.82904,-0.81915,-0.80902,-0.79864,-0.78801,-0.77715,-0.76604,-0.75471,-0.74314,-0.73135,-0.71934,-0.70711,-0.69466,-0.682,-0.66913,-0.65606,-0.64279,-0.62932,-0.61566,-0.60182,-0.58779,-0.57358,-0.55919,-0.54464,-0.52992,-0.51504,-0.5,-0.48481,-0.46947,-0.45399,-0.43837,-0.42262,-0.40674,-0.39073,-0.37461,-0.35837,-0.34202,-0.32557,-0.30902,-0.29237,-0.27564,-0.25882,-0.24192,-0.22495,-0.20791,-0.19081,-0.17365,-0.15643,-0.13917,-0.12187,-0.10453,-0.08716,-0.06976,-0.05234,-0.0349,-0.01745};

bn::random randomGenerator = bn::random();

void delay(int delayFrameCount) {
	// this function should of been made WAYYYY earlier
	for(int i=0; i<delayFrameCount; i++) {
		Game::doButanoUpdate();
	}
}

void uncompressData(u8 res[126], u8* input) {

	int i = 0;

	int val = -1;
	int count = -1;

	while(i < 126) {
		if(*input & 0xC0) {
			count = (*input & 0xC0) >> 6;
			val = (*input & ~0xC0);
		} else {
			count = *input;
			input++;
			val = *input;
		}

		input++;

		for(int j=0; j<count; j++) {
			res[i] = val;
			i++;
		}
	}

	//BN_ASSERT(i == 126, "i wasnt equal to 126 after decomp.");
}

void Game::createExitEffects() {

	// this func was originally in EntityManager::exit, but was moved.
	// tbh, it kinda shouldnt have been
	// bc in the future, getting timings of what death animations to stay on screen
	// before starting the transition will be, lets say spiritually taxing
	// tbh, that will require so much rewriting/jank i probs just wont

	// this function is horrid.

	Pos playerPos = EntityManager::player->p;

	EntityManager::createKillEffects();

	// determines if something(like a mon or lev statue) has killed the player
	// may cause issues if the player dies to multiple things at once??
	//bool customKill = EntityManager::obstacleKill();

	// if a custom kill handler(like for mon or tan statues)
	// but tbh, instead of calling those effects inside the domoves func, they really should be called here.


	// tbh this if statement is legacy now that im having death effects get drawn
	// at sprite's screen poses, instead of their game coords, but im leaving it here bc im scaredas
	if(playerPos != EntityManager::playerStart && EntityManager::enemyKill()) {

		// this case means a enemy killed the player and the player moved, so we use the player start pos

		// this line should NOT be commented out
		// how do i discern a falldeath from running into an enemy vs, shadow walkoff?
		Pos tempPos = playerPos;
		if(EntityManager::killAtPos(tempPos)) {
			playerPos = EntityManager::playerStart;
		}
	}


	// i could use the enemykill/fallkill things, but tbh like
	// in certain cases, where an enemey kills you above a floor like, yea
	if(EntityManager::playerWon()) {

	} else if(EntityManager::monKill()) {

		BN_LOG("killing player via mon! special goofy case!");
		EntityManager::addKill(EntityManager::player);
		//EntityManager::

		// some weird shit happens,,,, with mon statue kills
		// the player fall anim is replaced with, the defaullt fall???

	} else if(EntityManager::hasFloor(playerPos)) {
		BN_LOG("killing player via enemy");
		// unsure if this is a good idea, but it will make other calls to fallKill work
		EntityManager::addKill(EntityManager::player);
		EffectsManager::entityKill(EntityManager::player);
	} else {
		BN_LOG("killing player via fall");

		EntityManager::addKill(EntityManager::player);
		EffectsManager::entityFall(EntityManager::player);
	}

	EntityManager::player->p = playerPos;
}

void Game::findNextRoom() {

	if(EntityManager::playerWon()) {

		EntityManager::player->wingsUse = 0;

		EntityManager::updateScreen(); // is this ok?

		int tileManagerRoomIndex = TileManager::getRoomIndex();
		int startRoomIndex = RoomManager::roomIndex;

		// crash Game::
		if(tileManagerRoomIndex == -1 && startRoomIndex <= 256) {
			BN_LOG("crashing game due to bad floor count");
			CutsceneManager::displayDisText("FATAL ERROR : BR NULL\0");
			delay(5);
			CutsceneManager::crashGame();
		}

		// check for cif statue
		bool cifReset = false;
		Pos testPos = EntityManager::player->p;
		if(testPos.move(Direction::Up)) {

			SaneSet<Entity*, 4> tempMap = EntityManager::getMap(testPos);

			for(auto it = tempMap.begin(); it != tempMap.end(); ++it) {
				if((*it)->entityType() == EntityType::CifStatue) {
					cifReset = true;
					break;
				}
			}
		}

		if(cifReset) {
			// todo, in the future, put a special anim here
			BN_ASSERT(EntityManager::player != NULL, "player was null during cif reset");

			EntityManager::player->locustCount = 0;
			//TileManager::locustCounterTile->first = '0';
			//TileManager::locustCounterTile->second = '0';
			EntityManager::player->isVoided = false;

			//Game::save(); // extranious call for sanityl
			RoomManager::cifReset();
		} else {
			if(TileManager::exitDestination == NULL) {
				RoomManager::nextRoom();
			} else {
				RoomManager::gotoRoom(TileManager::exitDestination);
			}
		}

		/*

		the below code has become a massive issue with the addition of EX rooms
		E026 and B026 both have the same tilemanagerindex, but different room indicies
		potential solutions just,, adding 256 when in E?


		*/

		if(tileManagerRoomIndex != -1 && tileManagerRoomIndex != startRoomIndex) {
			if(tileManagerRoomIndex >= 256 && startRoomIndex <= 256) {
				// goto the white rooms
				RoomManager::gotoRoom("rm_u_0001\0");
			} else {
				RoomManager::gotoRoom(tileManagerRoomIndex);
			}
			return;
		}

		bn::sound_items::snd_stairs.play();
		return;
	} else {
		if(RoomManager::isWhiteRooms()) {
			RoomManager::roomIndex = 256; // this is bad
			save();
			CutsceneManager::crashGame();
		}
	}

	// do a check for if we are doing a bee statue shortcut (or in the future, a shortcut shortcut)((or in the future future, brands???))
	// because of the goofy ahh way this is written, this is going to get called every time until the falling anim finishes?, i dont like that tbh, but i cannot do anything
	// i can at least like,,, do a check to not duplicate inc room via just setting locusts to 0
	if(EntityManager::fallKill() && EntityManager::player->locustCount != 0) {

		bool beeReset = false;
		Pos testPos = EntityManager::player->p;

		if(testPos.move(Direction::Up)) {
			SaneSet<Entity*, 4> tempMap = EntityManager::getMap(testPos);
			for(auto it = tempMap.begin(); it != tempMap.end(); ++it) {
				if((*it)->entityType() == EntityType::BeeStatue) {
						beeReset = true;
						break;
				}
			}

			if(beeReset) {
				BN_LOG("bee reset occured");
				RoomManager::changeFloor(EntityManager::player->locustCount);
				EntityManager::player->locustCount = 0; // setting this to 0 should prevent,,, oofs when doing this (curse)
				//TileManager::locustCounterTile->first = '0';
				//TileManager::locustCounterTile->second = '0';

				// but does this update the save file?
			}
		}
	}

	// isnt,,, every kill now a fall kill??? or some shit???
	if(EntityManager::fallKill() && !EntityManager::hasFloor(EntityManager::player->p)) {
		bool foundGlass = false;
		const char* res = NULL;
		switch(RoomManager::roomIndex) {
			case 131: // glass break room
				// check for the goofy ahh no glass shortcut

				for(int x=0; x<14; x++) {
					for(int y=0; y<9; y++) {
						if(TileManager::hasFloor(x, y) == TileType::Glass) {
							foundGlass = true;
							break;
						}
					}
				}

				if(!foundGlass) {
					RoomManager::gotoRoom("rm_mon_shortcut_004");
				}
				break;
			case 23:
			case 53:
			case 67:
			case 89:
			case 137:
			case 157:
			case 179:
			case 223:
			case 227:

				res = TileManager::checkBrand();

				if(res != NULL) {
					RoomManager::gotoRoom(res);
				}

				break;
			default:
				break;
		}
	}

	if(TileManager::secretDestinations.size() != 0) {
		for(int i=0; i<TileManager::secretDestinations.size(); i++) {
			if(TileManager::secretDestinations[i].second == EntityManager::player->p && !TileManager::hasFloor(EntityManager::player->p)) {
				RoomManager::gotoRoom(TileManager::secretDestinations[i].first);
				break;
			}
		}
	}
}

void Game::resetRoom(bool debug) {

	BN_LOG("entered reset room with debug=",debug);

	//player->wingsUse = 0; // this being here renders a bunch of my other patchwork bs irrelevent.
	// wait actually,,, i am not sure if i want it here. i only have one other patch case left so yea

	// summon all room exiting effects
	if(!debug && !EntityManager::playerWon()) {
		createExitEffects();
	}

	int prevRoomIndex = RoomManager::roomIndex;

	// decide what room to goto next
	if(!debug) {
		findNextRoom();
	}


	if(!debug) {

		if(!RoomManager::isWhiteRooms()) {
			EntityManager::player->locustCount = MAX(0, TileManager::getLocustCount());
		}

		//if(!EntityManager::player->isVoided && !EntityManager::playerWon()) {
		if(!EntityManager::player->isVoided && prevRoomIndex == RoomManager::roomIndex) {
			if(EntityManager::player->locustCount > 0) {
				EntityManager::player->locustCount--;
			} else {
				EntityManager::player->isVoided = true;
			}
		}
	}


	BN_LOG("reseting to room ", RoomManager::currentRoomName());

	if(!debug) {
		state = GameState::Exiting;

		// wait for animations to finish
		while(state == GameState::Exiting) { // wait for gamestate to no longer be exiting
			doButanoUpdate();
		}
		// this one extra update is here for
		// the mon lightning effect specifically, i think?
		doButanoUpdate();
	} else {
		//doButanoUpdate();
	}

	state = GameState::Loading;

	CutsceneManager::resetRoom();
	queuedSounds.clear();
	removedSounds.clear();

	save();

	// this hopefully wont slow down debug moving much
	CutsceneManager::cutsceneLayer->rawMap.create(bn::regular_bg_items::dw_default_bg);
	CutsceneManager::backgroundLayer->rawMap.create(bn::regular_bg_items::dw_default_bg);

	changeMusic();

	loadLevel(debug);
	//if(!debug) {
	if(true) {
		doButanoUpdate();
	}

	fullDraw();

	state = GameState::Normal;

	if(!debug) {
		state = GameState::Entering;
		while(state == GameState::Entering) { // wait for gamestate to no longer be entering
			doButanoUpdate();
		}
	} else {
		//doButanoUpdate();
	}

	BN_ASSERT(state == GameState::Normal, "after a entering gamestate, the next state should be normal");

	doButanoUpdate();

	//bn::bg_tiles::log_status();

	BN_LOG("reset room done");
}

void Game::loadTiles() {

	// avoid dropping frames on the first alloc, when the game starts up

	bn::timer loadTilesTimer;
	bn::fixed tickCount;
	(void)tickCount; // supress warning if logging is disabled
	loadTilesTimer.restart();

	Room idek = RoomManager::loadRoom();

	const bn::regular_bg_tiles_item* collisionTiles = (const bn::regular_bg_tiles_item*)idek.collisionTiles;
	const bn::regular_bg_tiles_item* detailsTiles = (const bn::regular_bg_tiles_item*)idek.detailsTiles;

	int floorTileCount = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref().size();
	int collisionTileCount = collisionTiles->tiles_ref().size();
	int detailsTileCount = detailsTiles->tiles_ref().size();

	details->collisionTileCount = floorTileCount + collisionTileCount;

	bn::regular_bg_tiles_ptr backgroundTiles = collision->rawMap.bgPointer.tiles();

	bn::optional<bn::span<bn::tile>> tileRefOpt = backgroundTiles.vram();
	BN_ASSERT(tileRefOpt.has_value(), "wtf");
	bn::span<bn::tile> tileRef = tileRefOpt.value();

	// copying to vram(directly) will cause issues when like,,,, going bull(curse)ery relating to
	// swapping via debug keys, but thats fine, its debug

	// i REALLLLY should use memcpy here
	// and only change areas of mem which actually need changes

	BN_ASSERT( floorTileCount + collisionTileCount + detailsTileCount < tileRef.size(), "didnt have enough alloc for tiles, need at least ", floorTileCount + collisionTileCount + detailsTileCount);

	memcpy(tileRef.data(), collisionTiles->tiles_ref().data(), 8 * sizeof(uint32_t) * collisionTileCount);

	memcpy(tileRef.data() + collisionTileCount, detailsTiles->tiles_ref().data(), 8 * sizeof(uint32_t) * detailsTileCount);

	if(RoomManager::isWhiteRooms()) {

		unsigned dataBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		for(int i=0; i<floorTileCount; i++) {

			if(i/4 == 1) {
				memcpy(dataBuffer, bn::sprite_tiles_items::dw_spr_floor_unknown.tiles_ref()[i-4].data, 8 * sizeof(unsigned));
			} else if(i/4 == 2) {
				memcpy(dataBuffer, bn::sprite_tiles_items::dw_spr_floor_unknown.tiles_ref()[i-4].data, 8 * sizeof(unsigned));
			} else if(i/4 == 19) {
				memcpy(dataBuffer, bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data, 8 * sizeof(unsigned));
			} else {
				memset(dataBuffer, 0x00000000, 8 * sizeof(unsigned));
			}

			tileRef[i + detailsTileCount + collisionTileCount].data[0] = dataBuffer[0];
			tileRef[i + detailsTileCount + collisionTileCount].data[1] = dataBuffer[1];
			tileRef[i + detailsTileCount + collisionTileCount].data[2] = dataBuffer[2];
			tileRef[i + detailsTileCount + collisionTileCount].data[3] = dataBuffer[3];
			tileRef[i + detailsTileCount + collisionTileCount].data[4] = dataBuffer[4];
			tileRef[i + detailsTileCount + collisionTileCount].data[5] = dataBuffer[5];
			tileRef[i + detailsTileCount + collisionTileCount].data[6] = dataBuffer[6];
			tileRef[i + detailsTileCount + collisionTileCount].data[7] = dataBuffer[7];
		}

	} else {
		memcpy(tileRef.data() + collisionTileCount + detailsTileCount, bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref().data(), 8 * sizeof(uint32_t) * floorTileCount);
	}

	details->collisionTileCount = collisionTileCount;
	TileManager::floorLayer->collisionTileCount = collisionTileCount + detailsTileCount;

	tickCount = loadTilesTimer.elapsed_ticks();
	BN_LOG("loadtiles took ", tickCount.safe_division(FRAMETICKS), " frames");
}

void Game::loadLevel(bool debug) {

	loadTiles();

	BN_LOG("entered loadlevel with debug=", debug, " roomname=", RoomManager::currentRoomName(), " roomindex = ", RoomManager::roomIndex);

	//load();

	bn::timer loadLevelTimer;
	bn::fixed tickCount;
	(void)tickCount; // supress warning if logging is disabled
	loadLevelTimer.restart();

	Room idek = RoomManager::loadRoom();


	u8 uncompressedCollision[126];
	u8 uncompressedDetails[126];

	uncompressData(uncompressedCollision, (u8*)idek.collision);
	uncompressData(uncompressedDetails, (u8*)idek.details);


	static bool needRestore = false;

	if(needRestore) {
		//CutsceneManager::restoreAllButEffectsAndFloor();
		//TileManager::floorLayer.rawMap.bgPointer.set_priority(2);
		needRestore = false;
	}

	if(needRedraw) {
		needRedraw = false;

	}

	//doButanoUpdate(); // these excess frame updates will just slow (curse) down

	/*

	bottom left corner of room 39 (rm_0040)
	has collision covering,,,,, the ui. but that area is supposed to be accessable.
	also of note, reverseing all my x y loops to go y x would maybe give a small speed boost bc of caching

	*/

	for(int x=0; x<14; x++) {
		//for(int y=0; y<9; y++) {
		for(int y=0; y<9; y++) {

			if(y == 8) { // room 39 fix
				collisionMap[x][y] = 0;
				continue;
			}

			collisionMap[x][y] = uncompressedCollision[x + 14 * y];
			detailsMap[x][y] = uncompressedDetails[x + 14 * y];

			// when changing the color palettes, some weird (curse) happened where, areas
			// would be transparent when they shouldnt be? this hopefully fixes that

			if(collisionMap[x][y] == 0) {
				//collisionMap[x][y] = 1;
			}
		}
	}

	u8* floorPointer = (u8*)idek.floor;

	SecretHolder* secretsPointer = (SecretHolder*)idek.secrets;
	int secretsCount = idek.secretsCount;
	const char* exitDest = (const char*)idek.exitDest;


	doButanoUpdate();


	//static bn::timer tempTimer;
	//tempTimer.restart();

	TileManager::loadTiles(floorPointer, secretsPointer, secretsCount, exitDest);

	//tickCount = tempTimer.elapsed_ticks();
	//BN_LOG("tileManager loadtiles took ", tickCount.safe_division(FRAMETICKS), " frames");

	doButanoUpdate();

	EntityHolder* entitiesPointer = (EntityHolder*)idek.entities;
	int entitiesCount = idek.entityCount;


	EntityManager::loadEntities(entitiesPointer, entitiesCount);

	//BN_LOG("loadentities was completed,,, sorta?");

	doButanoUpdate();

	EffectHolder* effectsPointer = (EffectHolder*)idek.effects;
	int effectsCount = idek.effectsCount;

	EffectsManager::loadEffects(effectsPointer, effectsCount);

	// should probs put the below into the cutscene file

	// this code actually rlly should of been in effects, omfg
	if(strcmp(RoomManager::currentRoomName(), "rm_rm4\0") == 0 || strcmp(RoomManager::currentRoomName(), "hard_rm_rm4\0") == 0) {
		needRestore = true;
		//CutsceneManager::backupAllButEffectsAndFloor();

		CutsceneManager::createPlayerBrandRoom();

		// this should of been programmed in as a bigsprite, but now that i know that i shouldnt use those, its going as a bg
		// the floor rlly should be combined onto the same layer as collision

	} else if(RoomManager::currentRoomHash() == hashString("rm_u_end\0")) {
		//needRestore = true;
		//CutsceneManager::backupAllButEffectsAndFloor();


		CutsceneManager::createResetRoom();
	} else {
		if((strstrCustom(RoomManager::currentRoomName(), "_u_00\0") == NULL) &&
			(strstrCustom(RoomManager::currentRoomName(), "_u_en\0") == NULL)) {
			bn::bg_palettes::set_transparent_color(pal->getColorArray()[1]);
			//CutsceneManager::backgroundLayer.rawMap.bgPointer.set_item(bn::regular_bg_items::dw_default_black_bg);
			//CutsceneManager::backgroundLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		} else {
			//CutsceneManager::backgroundLayer.rawMap.bgPointer.set_item(bn::regular_bg_items::dw_default_white_bg);
			//CutsceneManager::backgroundLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
			bn::bg_palettes::set_transparent_color(pal->getColorArray()[2]);
		}
	}

	EffectsManager::setBorderColor(!RoomManager::isWhiteRooms());


	tickCount = loadLevelTimer.elapsed_ticks();
	BN_LOG("loadlevel completed, took ", tickCount.safe_division(FRAMETICKS), " frames");
}

void drawCollisionAndDetails() {
	// PUTTING THIS IN ARM GIVES A 50% REDUCTION. FIGURE IT OUT DUMBASS

	// why the fuck. does putting this in arm and iwram cause everything in restrequest to fucking melt and die??
	*reinterpret_cast<unsigned short*>(REG_IME) = 0; // disable interrupts

	auto& collisionMap = Game::collisionMap;
	auto& detailsMap = Game::detailsMap;
	auto& floorMap = TileManager::floorMap;

	auto& collision = Game::collision;
	auto& details = Game::details;

	for(int x=0; x<14; x++) {
		for(int y=0; y<8; y++) {

			// ugh. this is needed, i just hate that it is needed
			if(floorMap[x][y] != NULL) {
				continue;
			}

			int tile = collisionMap[x][y];

			if(tile < 3) {
				tile = detailsMap[x][y];
				if(tile != 0) {
					details->setTile(x * 2 + 1, y * 2 + 1, 4 * tile);
					details->setTile(x * 2 + 2, y * 2 + 1, 4 * tile + 1);
					details->setTile(x * 2 + 1, y * 2 + 2, 4 * tile + 2);
					details->setTile(x * 2 + 2, y * 2 + 2, 4 * tile + 3);
				}
			} else {
				collision->setTile(x * 2 + 1, y * 2 + 1, 4 * tile);
				collision->setTile(x * 2 + 2, y * 2 + 1, 4 * tile + 1);
				collision->setTile(x * 2 + 1, y * 2 + 2, 4 * tile + 2);
				collision->setTile(x * 2 + 2, y * 2 + 2, 4 * tile + 3);
			}
		}
	}

	*reinterpret_cast<unsigned short*>(REG_IME) = 1; // enable interrupts
}

void clearGameMap() {

	// the goal of this is simple, to clear the game map
	// i could do it in dotiledraw, but it rlly fucked performance (made it const time)
	// which is to an extent, ideal, but still
	// does butano have any funcs for this already?
	// i rlly hope that this doesnt take to much iwram up
	// loop unrolling rlly is messing me up here
	// also gods pointer lookups vs refs
	// i rlly need to make everything namespaces, but make sure they are still in ewram
	// im also reminded at how much i despise this syntax

	// clearGameMap draw took 0.08715 frames
	// [WARN] GBA Debug:	clearGameMap draw took 0.10717 frames
	// going through butano funcs is pathetically slow, most likely due to mults/maybe they arent in iwram?
	// i am quite suspicious of the difference between only having section(".ewram") and having that and target("arm")
	// i swear that the arm being there is needed to have it actually be in arm

	auto& cells = Game::collision->rawMap.cells;
	auto& mapItem = Game::collision->rawMap.mapItem;

	for(int x=1; x<28+1; x++) {
		for(int y=1; y<18+1; y++) {
			bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
			bn::regular_bg_map_cell_info current_cell_info(current_cell);

			current_cell_info.set_tile_index(0);
			current_cell = current_cell_info.cell();
		}
	}
}

void Game::fullDraw() {

	BN_LOG("entering fulldraw");

	// THESE 3 CALLS SHOULD BE MERGED INTO ONE FUCKHEAD

	bn::timer tempTimer;
	bn::fixed tickCount;
	(void)tickCount; // supress warning if logging is disabled

	/*
	tempTimer.restart();
	//collision.draw(collisionMap);
	tickCount = tempTimer.elapsed_ticks();
	BN_LOG("collision draw took ", tickCount.safe_division(FRAMETICKS), " frames");

	tempTimer.restart();
	//details.draw(detailsMap, collisionMap);
	tickCount = tempTimer.elapsed_ticks();
	BN_LOG("details draw took ", tickCount.safe_division(FRAMETICKS), " frames");
	*/

	tempTimer.restart();
	clearGameMap();
	tickCount = tempTimer.elapsed_ticks();
	BN_LOG("clearGameMap draw took ", tickCount.safe_division(FRAMETICKS), " frames");

	tempTimer.restart();
	TileManager::fullDraw();
	tickCount = tempTimer.elapsed_ticks();
	BN_LOG("tile draw took ", tickCount.safe_division(FRAMETICKS), " frames");


	tempTimer.restart();
	drawCollisionAndDetails();
	tickCount = tempTimer.elapsed_ticks();
	BN_LOG("collision and details draw took ", tickCount.safe_division(FRAMETICKS), " frames");

	// bad move, wouldnt be needed if i wasnt drawing the background 3 FUCKING TIMES.
	//doButanoUpdate();

	tempTimer.restart();
	EntityManager::fullUpdate();
	tickCount = tempTimer.elapsed_ticks();
	BN_LOG("entityManager draw took ", tickCount.safe_division(FRAMETICKS), " frames");

	// i swear. why does the game crash without this print here?
	BN_LOG("fulldraw completed");
}

void Game::fullTileDraw() {

	// this func should be removeddddddd

	//collision.draw(collisionMap);
	//details.draw(detailsMap, collisionMap);
	//TileManager::fullDraw();
}

void Game::changePalette(int offset) {

	// https://stackoverflow.com/questions/3417183/modulo-of-negative-numbers
	// lol

	// this is a horrid way of doing it, i should be able to just like,,, access the actual palette table???

	// why the (curse) didnt i have a getpalete function in the game class. im going to have to redo so much bs


	const int paletteListSize = (int)(sizeof(paletteList) / sizeof(paletteList[0]));

	paletteIndex += offset;

	paletteIndex = ((paletteIndex % paletteListSize) + paletteListSize) % paletteListSize;

	pal = paletteList[paletteIndex];

	/*
	if(paletteIndex == paletteListSize - 1) {
		CutsceneManager::inputCustomPalette();
	}
	*/

	pal->update();

	bn::bg_palettes::set_transparent_color(pal->getColorArray()[1]);

	EntityManager::updatePalette(paletteList[paletteIndex]);
	EffectsManager::updatePalette(paletteList[paletteIndex]);


	collision->rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	//details.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	//TileManager::floorLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	EffectsManager::effectsLayer->rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());

	BN_ASSERT(CutsceneManager::cutsceneLayer != NULL, "CutsceneLayer Empty");
	BN_ASSERT(CutsceneManager::backgroundLayer != NULL, "BackgroundLayer Empty");
	CutsceneManager::cutsceneLayer->rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	CutsceneManager::backgroundLayer->rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());

	BackgroundMap::backgroundPalette = paletteList[paletteIndex];

	pal->update();

	*col0 = pal->getColorArray()[0].data();
	*col1 = pal->getColorArray()[1].data();
	*col2 = pal->getColorArray()[2].data();
	*col3 = pal->getColorArray()[3].data();
	*col4 = pal->getColorArray()[4].data();

	//BN_LOG(*col1);
	//BN_LOG(*col2);
	//BN_LOG(*col3);
	//BN_LOG(*col4);

	saveData.col1Save = *col1;
	saveData.col2Save = *col2;
	saveData.col3Save = *col3;
	saveData.col4Save = *col4;


	// this is a problem, (curse) it ima just have palette not cause a save.
	//save();


	// oh boy, prepare for fun
	// as far as i know, butano doesnt give me direct memory access, nor does it give me direct palette access.
	// this means that we are about to have a fun time with direct memory (curse)
	// reminds me of the original gb
	// going to have to probs go back and fix some of the random areas where i manually am setting a palette
	// probs might actually make those funcs private
	// declare game as a friend class?
	// ok, so,, issue
	// from menu text changing color
	//,, yea
	// maybe ill just update the effects thing and pray that like, actually no i know that butano will (curse) me on this
	// ugh
	// it seems like,,, this entire journey was pointless
	// esp since i got the stupid thing in palete.h working(WHY DOES A STACK ALLOCED ARRAY NOT WORK??)

	// bg palette at 0x05000000 + 0x1E0
	// sprite palette at 0x05000000 + 0x3E0

	/*
	unsigned short* bgPalette = reinterpret_cast<unsigned short*>(0x05000000 + 0x1E0);
	unsigned short* spritePalette = reinterpret_cast<unsigned short*>(0x05000000 + 0x3E0);

	//BN_ERROR( pal->colorArray[0].red(), " ", pal->colorArray[0].green(), " ", pal->colorArray[0].blue(), " ", *idek);

	for(int i=0; i<5; i++) {
		unsigned short temp = 0;

		temp |= (pal->colorArray[i].blue() << 10);
		temp |= (pal->colorArray[i].green() << 5);
		temp |= (pal->colorArray[i].red() << 0);

		bgPalette[i] = temp;
		spritePalette[i] = temp;
	}

	*/

	if(EntityManager::player != NULL) {
		// FUCKING DUMBASSsave();
		//save();
	}
}

void Game::fadePalette(const int index) {

	// if index is 0, backup, and wipe the table
	// if a pos integer, we ball.
	// this currently will only fade from black to white, as for white to blank, idk
	// i did the intro cutscenes fade much differently, doing the fade for cif is a massive pain
	// im also going to assume a limit on the number of palettes but,, gods ugh.
	// (curse) it, we alloc a whole kb of memory.
	// im going to basically have to do that inthe heap
	// is new auto allocated in the heap? or do i have to do some weird staticdata bs


	// first tick is all black, but WHITE=DARKGRAY
	// second tick, ALL(except black) =DARKGRAY
	// third tick, WHITE+LIGHTGRAY=LIGHTGRAY
	// fourth tick, all colors are set

	// this code is an affront to the gods, and i wonder if it will like,,, work at all
	// i should try my best to not use my own memcpy funcs
	// actually,,,, couldnt i just not be stupid and have a static var in the palete class.
	// omfg
	// but then i have to update each palette of like everything, every (curse)ing frame.

	//BN_ASSERT(isVblank, "palette fading should only happen in vblank, or at least i think");

	// THIS IS NEVER PROPERLY FREED (curse)
	static unsigned short* localPaletteTable = NULL;

	if(localPaletteTable == NULL) {
		localPaletteTable = new unsigned short[512]();
		BN_LOG(localPaletteTable, " ", sizeof(unsigned short));
	}

	unsigned short* palettePointer = reinterpret_cast<unsigned short*>(0x05000000);

	if(index == 0) {

		memcpy(localPaletteTable, palettePointer, sizeof(unsigned short) * 512);

		return;
	}

	// this code was written during a walter white fugue state. i dont get it either fam

	for(unsigned i=0; i<512; i++) {
		if(index == 1) {
			switch(i % 16) {
				case 2:
					palettePointer[i] = localPaletteTable[i + 2];
					break;
				case 3:
					palettePointer[i] = localPaletteTable[i - 2];
					break;
				case 4:
					palettePointer[i] = localPaletteTable[i - 3];
					break;
				default:
					palettePointer[i] = localPaletteTable[i];
					break;
			}
		} else if(index == 2) {
			switch(i % 16) {
				case 2:
					palettePointer[i] = localPaletteTable[i + 2];
					break;
				case 3:
					palettePointer[i] = localPaletteTable[i + 1];
					break;
				default:
					palettePointer[i] = localPaletteTable[i];
					break;
			}
		} else if(index == 3) {
			switch(i % 16) {
				case 2:
					palettePointer[i] = localPaletteTable[i + 1];
					break;
				default:
					palettePointer[i] = localPaletteTable[i];
					break;
			}
		} else if(index == 4) {
			palettePointer[i] = localPaletteTable[i];
		}
	}
}

void Game::doButanoUpdate() {

	//static bn::timer vblankTimer;
	//static bn::fixed tickCount;
	//(void)tickCount; // supress warning if logging is disabled
	//vblankTimer.restart();

	Game::doVBlank();

	//tickCount = vblankTimer.elapsed_ticks();
	//BN_LOG("vblank ops took ", tickCount.safe_division(FRAMETICKS), " frames");
	//vblankTimer.restart();

	bn::core::update();

	//tickCount = vblankTimer.elapsed_ticks();
	//BN_LOG("vblank core took ", tickCount.safe_division(FRAMETICKS), " frames");

	int temp = bn::core::last_missed_frames();

	bn::fixed vblankUsage = bn::core::last_vblank_usage();

	//bool fucked = false;

	if(temp != 0) {
		BN_LOG("\n\n\n\n\n");
		BN_LOG("dropped frames: ", temp);
		BN_LOG("CPU:    ", bn::core::last_cpu_usage());
		BN_LOG("VBLANK: ", bn::core::last_vblank_usage());
		//fucked = true;
	}

	if(vblankUsage > 1) {
		BN_LOG("\n\n\n\n\n");
		BN_LOG("VBLANK EXCEDED, THIS IS RLLY BAD: ");
		BN_LOG("CPU:    ", bn::core::last_cpu_usage());
		BN_LOG("VBLANK: ", bn::core::last_vblank_usage());
		//fucked = true;
	}

	/*
	if(fucked) {
		for(int i=0; i<20; i++) {
			BN_LOG("YOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED\nYOU ARE FUCKED");
		}
	}
	*/
}

void logRamStatus() {

	unsigned stackIWram = bn::memory::used_stack_iwram();
	unsigned staticIWram = bn::memory::used_static_iwram();
	unsigned totalIWram = stackIWram + staticIWram;

	BN_LOG("used_stack_iwram: ", ((bn::fixed)stackIWram).safe_division(32 * 1024));
	BN_LOG("used_static_iwram: ", ((bn::fixed)staticIWram).safe_division(32 * 1024));
	BN_LOG("total iwram: ", ((bn::fixed)totalIWram).safe_division(32 * 1024));

	bn::memory::log_alloc_ewram_status();
}

void didVBlank() {

	unsigned stackIWram = bn::memory::used_stack_iwram();
	unsigned staticIWram = bn::memory::used_static_iwram();
	unsigned totalIWram = stackIWram + staticIWram;

	//BN_LOG("used_stack_iwram: ", stackIWram.safe_division(32 * 1024));
	//BN_LOG("used_static_iwram: ", staticIWram.safe_division(32 * 1024));
	//BN_LOG("total iwram: ", totalIWram.safe_division(32 * 1024));

	BN_ASSERT(totalIWram < 32 * 1024, "iwram overflow!!! val=", totalIWram);
	BN_ASSERT(totalIWram < 31 * 1024, "iwram getting close to overflow!!! val=", totalIWram);
	//logRamStatus();

	frame++;

	//BN_ASSERT(globalGame != NULL, "in vblank, globalgame was null");

	isVblank = true;
	randomGenerator.update();
	bruhRand();
	isVblank = false;
}

void Game::doVBlank() { profileFunction();

	// can vblank occur during the gameloop, or does this only get called after we call a butano update?
	// if this can get called in the gameloop, then we will have problems once we integrate dialogue, bc during dialogue this needs to be disabled!

	// cutscenes should be queued to a vector, and executed in here.
	// or maybe not
	// also should cutscenes have a vblank call? maybe
	// i could have it execute a lambda

	// a fundamental flaw has been discovered.
	// i,, everything i do in vblank, doesnt need to be in vblank, and tbh maybe shouldnt be in vblank?
	// bc like,,, maybe i should leave that to butano?
	// does,,, oh gods
	// when i first started this, i was just using the vblank hook, but now that i have dobutanoupdate, i can like
	// use cputime for extra stuff, per frame
	// another issue is, when im executing code, do i only have a,, scanlines 0-160 worth? or do i have that and vblank??
	// should i maybe,,, have a non vblank func for each thing
	// also gods now that i have that extra, hella extra cpu time bc of the optimizations i made
	// holy fucking shit
	// i just moved this function call to instead of being called in didvblank
	// call it in dobutanoupdate.
	// this means that if i have any bn::core::update calls left anywhere, im boned, but like
	// holy shit this is insanely helpful
	// b213 full of shadows, 0.22 cpu usage per move
	// actually fucking insane how,, how did i not notice this earlier?
	//
	static bool a, b, c = false;

	switch(state) {
		case GameState::Normal:
			//BN_LOG("EntityManager::doVBlank();");
			//timer.restart();
			EntityManager::doVBlank();
			//BN_LOG("entityManager vblank took ", ((bn::fixed)timer.elapsed_ticks()).safe_division(VBLANKTICKS), " vblanks");

			//BN_LOG("EffectsManager::doVBlank();");
			//timer.restart();
			EffectsManager::doVBlank();
			//BN_LOG("effectsManager vblank took ", ((bn::fixed)timer.elapsed_ticks()).safe_division(VBLANKTICKS), " vblanks");

			//BN_LOG("TileManager::doVBlank();");
			//timer.restart();
			TileManager::doVBlank();
			//BN_LOG("tileManager vblank took ", ((bn::fixed)timer.elapsed_ticks()).safe_division(VBLANKTICKS), " vblanks");
			//BN_LOG("done");
			break;
		case GameState::Exiting:
			if(!a) { a = EntityManager::exitRoom(); }
			if(!b) { b = EffectsManager::exitRoom(); }
			if(!c) { c = TileManager::exitRoom(); }
			if(a && b && c) {
				state = GameState::Entering;
				a = b = c = false; // vine boom sound effect
			}
			break;
		case GameState::Entering:
			if(!a) { a = EntityManager::enterRoom(); }
			if(!b) { b = EffectsManager::enterRoom(); }
			if(!c) { c = TileManager::enterRoom(); }
			if(a && b && c) {
				state = GameState::Normal;
				a = b = c = false; // vine boom sound effect
			}
			// tick effects even while intro is occuring
			// this should of been here months ago.
			EffectsManager::doVBlank();
			break;
		case GameState::Loading:
		case GameState::Paused:
			break;
		case GameState::Dialogue:
			EffectsManager::doVBlank();
			break;
		case GameState::Cutscene:
			CutsceneManager::doVBlank();
			break;
		case GameState::Sleep:
			break;
		default: [[unlikely]]
			BN_ERROR("unknown state in game::doVBlank");
			break;
	}

	// WILL THIS LEAD TO A 1 FRAME AUDIO DELAY WITH SOUNDS?
	// depending on if the sound handler runs before or after ours.
	// this will require more investigation
	// also, this being below all other vblank managers is very important, since some vblank managers will add sounds to it!
	doSoundVBlank();
}

void Game::run() {

	BN_LOG("look at u bein all fancy lookin in the logs");

	RoomManager::isCustomRooms();

	if(!RoomManager::isCustom) {
		load();
	}

	EffectsManager::setBorderColor(!RoomManager::isWhiteRooms());

	changePalette(0); // the paletteindex is already set by the load func, this just properly updates it

	bn::core::set_vblank_callback(didVBlank);

	bn::timer inputTimer;
	bn::timer moveTimer;

	state = GameState::Loading;

	bool brandBlank = true;
	for(int i=0; i<6; i++) {
		if(TileManager::playerBrand[i] != 0) {
			brandBlank = false;
			break;
		}
	}

	if(brandBlank && !RoomManager::isCustom) {

		bool shouldDoBrandInput = CutsceneManager::titleScreen();

		if(shouldDoBrandInput) {
			CutsceneManager::brandInput();
		} else {

			int brandState[6][6];
			// i have trust issues
			for(int i=0; i<6; i++) {
				for(int j=0; j<6; j++) {
					brandState[i][j] = 0;
				}
			}

			for(int i=0; i<6; i++) {
				int j = 6 - i - 1;
				brandState[i][i] = 1;
				brandState[i][j] = 1;
			}

			unsigned tempBrand[6] = {0, 0, 0, 0, 0, 0};

			for (int j=0; j<6; j++) {
				unsigned temp = 0;
				for(int i=0; i<6; i++) {
					temp = ((temp << 1) | (brandState[i][j]));
				}
				tempBrand[j] = temp;
			}

			for(int i=0; i<6; i++) {
				TileManager::playerBrand[i] = tempBrand[i];
			}

			// savedata setup
			debugToggle = true;
			saveData.debug = true;

			saveData.hasMemory = true;
			saveData.hasWings = true;
			saveData.hasSword = true;

			saveData.hasSuperRod = true;

			saveData.mode = 2;
			mode = 2;
			RoomManager::setMode(2);

			paletteIndex = 1;
			saveData.paletteIndex = 1;
			changePalette(0);
		}
	} else {
		if(!debugToggle) {
			CutsceneManager::titleScreen();
		}
	}

	loadLevel();
	doButanoUpdate();
	fullDraw();

	save();

	state = GameState::Normal;

	changeMusic();

	BN_LOG("starting main gameloop");
	while(true) {

		if(debugToggle && bn::keypad::any_held()) {
			if(bn::keypad::l_held() || bn::keypad::r_held()) {

				int debugIncrement = bn::keypad::start_held() ? 5 : 1;

				// could maybe cause some issues if the frame counter,, resets? during this?
				// does keeping the frame thing low save division cycles? i shouldnt even be doing modulo at all tho tbh

				unsigned startFrame = frame;

				if(bn::keypad::l_held()) {
					for(int i=0; i<debugIncrement; i++) {
						RoomManager::prevRoom();
					}
				} else {
					for(int i=0; i<debugIncrement; i++) {
						RoomManager::nextRoom();
					}
				}

				resetRoom(true);

				unsigned waitFrames = frame - startFrame;

				if(frame <= startFrame) {
					waitFrames = 5;
				}

				if(waitFrames > 5) {
					waitFrames = 0;
				} else {
					waitFrames = 5 - waitFrames;
				}

				unsigned i=0;
				while(i < waitFrames) { i++; doButanoUpdate(); }

				continue;
			}

			// pokemon style reset
			// changing this to have multiple options
			if(bn::keypad::a_held() && bn::keypad::b_held() && bn::keypad::start_pressed()) {
				//THIS IS (curse), make it break out of this loop, and then put the actual Game* in a while func, and have it call its destructor and then reconstruct
				//bn::core::reset();
				BN_LOG("full game reset called");

				Game::saveData.hash = 0;
				bn::sram::write(Game::saveData);

				delay(1);

				// previously this would just break out of the loop. it would be nice though if this completely restarted the gba. i can do that right?
				_fullReset();

				break;
			}

			if(bn::keypad::a_held() && bn::keypad::b_held() && bn::keypad::select_held()) {
				//THIS IS (curse), make it break out of this loop, and then put the actual Game* in a while func, and have it call its destructor and then reconstruct
				//bn::core::reset();
				BN_LOG("save reset called");
				EntityManager::player->locustCount = 0;
				EntityManager::player->isVoided = false;
				save();
				break;
			}
		}

		if(bn::keypad::start_pressed()) {
			EffectsManager::doMenu();

			inputTimer.restart();
			doButanoUpdate();
			continue;
		}

		if(bn::keypad::select_pressed()) {
			debugToggle = !debugToggle;
			EffectsManager::setDebugDisplay(!RoomManager::isWhiteRooms());

			inputTimer.restart();
			//save(); // calling debug here has a chance to maybe fuck shit up,, but
			// tbh its still a bummer that like,,i should be saving after each move tbh
			doButanoUpdate();
			continue;
		}

		if(bn::keypad::b_pressed()) {
			if(bn::keypad::select_held()) {
				Profiler::reset();
			} else {
				Profiler::show();
			}

			inputTimer.restart();
			doButanoUpdate();
			continue;
		}

		// i rlly wish butano gave me bitwise access to the controls. but this wont be that expensive(hopefully)
		// i could use an array, not sure if i rlly want to, but eh idk

		constexpr bn::keypad::key_type validKeys[] = {
			bn::keypad::key_type::UP,
			bn::keypad::key_type::DOWN,
			bn::keypad::key_type::LEFT,
			bn::keypad::key_type::RIGHT,
			bn::keypad::key_type::A
		};

		bool doMove = false;

		// on any press, ill always do the input
		if(bn::keypad::any_pressed()) {
			for(const auto key : validKeys) {
				if(bn::keypad::pressed(key)) {
					doMove = true;
					break;
				}
			}
		}

		// this is where ill check for held buttons
		if(!doMove && saveData.delay != -1 && inputTimer.elapsed_ticks() > saveData.delay && bn::keypad::any_held()) {
			for(const auto key : validKeys) {
				if(bn::keypad::held(key)) {
					doMove = true;
					break;
				}
			}
		}

		if(doMove) {
			doMove = false;

			moveTimer.restart();

			EntityManager::doMoves();

			// doing this after,,, so repeated mistake presses are less likely
			inputTimer.restart();

			if(EntityManager::hasKills()) {
				resetRoom();
				continue;
			}

			playerIdleFrame = frame;

			bn::fixed tickCount = moveTimer.elapsed_ticks();
			(void)tickCount; // supress warning if logging is disabled

			BN_LOG("a move took ", tickCount.safe_division(FRAMETICKS), " frames");

			// wait for certain animations to be done until the next move can occur
			while(true) {
				bool noWait = true;
				for(int i = 0; i < EffectsManager::effectList.size(); i++) {
					if(EffectsManager::effectList[i]->waitFlag) {
						noWait = false;
						break;
					}
				}

				if(noWait) {
					break;
				}

				doButanoUpdate();
			}
		}

		doButanoUpdate();
	}
}

// -----

void Game::playSound(const bn::sound_item* sound) {

	// THIS FUNC SHOULD ONLY BE USED FOR SOUNDS WITH A POSSIBILITY OF BEING EXCLUDED OR ALTERED

	if(boobaCount >= 8 && sound == &bn::sound_items::snd_push_small) {
		sound = &bn::sound_items::snd_bounce;
	}

	if(state == GameState::Loading || state == GameState::Entering) {
		return;
	}

	if(!removedSounds.contains(sound)) {
		queuedSounds.insert(sound);
	}
}

void Game::removeSound(const bn::sound_item* sound) {

	if(state == GameState::Loading || state == GameState::Entering) {
		return;
	}

	queuedSounds.erase(sound);
	removedSounds.insert(sound);
}

void Game::doSoundVBlank() {

	// should this if statement not be here and,,, just be like in the switch case? yes.
	// but im tired

	if(state == GameState::Loading || state == GameState::Entering) {
		return;
	}

	unsigned playedSounds = 0;

	for(auto it = queuedSounds.begin(); it != queuedSounds.end(); ++it) {
		if(!removedSounds.contains(*it)) {
			(*it)->play();
			playedSounds++;
		}
	}

	/*
	if(playedSounds != 0) {
		BN_LOG("played ", playedSounds, " sound(s) (from the priority soundhandler)");
	}
	*/

	queuedSounds.clear();
	removedSounds.clear();
}

void Game::changeMusic() {

	auto doPlay = [](const bn::music_item& item) -> void {

		/*

		an idea.
		build a whole different rom for each different song.
		custom hardware(ill do something like writing to a certain rom address)
		(maybe theres a way to directly,,, get butano log calls?)
		would switch the banks??
		at which point, i can do whatever i want.
		the issue? i hated the previous pcb software i used

		https://github.com/HDR/NintendoPCBs/tree/master/AGB-E02-20

		or,,, arduino? well for testing yes, but past that no

		fpga? i could do this with inline circuitry tho
		maybe,,, depending on how this works i could clock(write) to a buffer
		which just has lines constantly going to the rom chip?

		lets be greedy and say i want every song.
		thats 76. getting that down to 64 would be nice
		but,,,, things arent always nice. 128 it is, and might as well just go up to 256 so i can
		say i need to track 8 bits

		this is going to need to be fpga, if i end up doing it
		and, considering that im basically done with the main game
		and dont want this project to end, i probs will

		*/

		static int prevMode = Game::mode;

		if(bn::music::playing() && bn::music::playing_item() == item && prevMode == Game::mode) {
			return;
		}

		prevMode = Game::mode;

		bn::fixed adjustVal = 1.0;

		if((Game::mode == 2 || EntityManager::player->isVoided) &&
			(item == bn::music_items::msc_dungeon_wings ||
			item == bn::music_items::msc_beecircle ||
			item == bn::music_items::msc_dungeongroove ||
			item == bn::music_items::msc_013 ||
			item == bn::music_items::msc_gorcircle_lo ||
			item == bn::music_items::msc_levcircle ||
			item == bn::music_items::msc_cifcircle
			)) {

			adjustVal = 0.965;
		}

		item.play();

		// tbh,,,, no.
		//bn::music::set_pitch(adjustVal);
		//bn::music::set_tempo(adjustVal);

	};

	int roomIndex = RoomManager::roomIndex;

	if(roomIndex == 254 && mode == 2) {
		doPlay(bn::music_items::msc_voidsong);
		return;
	}

	int index = 0;
	int temp = 0;

	while(temp < 255) {
		if(roomIndex == temp || roomIndex == temp + 1) {
			bn::music::stop();
			return;
		}
		index++;
		temp = 28 * index;
	}

	if(roomIndex == 30) {
		// tail!
		doPlay(bn::music_items::msc_007);
		return;
	}

	/*
	1-28 add
	29-56 eus
	57-84 bee
	84-112 mon
	113-140 tan
	141-168 gor
	169-196 lev
	196-224 cif
	*/

	if(roomIndex <= 28) {
		if(mode == 2) {
			doPlay(bn::music_items::msc_themeofcif);
		} else {
			doPlay(bn::music_items::msc_001);
		}
	} else if(roomIndex <= 56) {
		doPlay(bn::music_items::msc_dungeon_wings);
	} else if(roomIndex <= 84) {
		doPlay(bn::music_items::msc_beecircle);
	} else if(roomIndex <= 112) {
		doPlay(bn::music_items::msc_dungeongroove);
	} else if(roomIndex <= 140) {
		doPlay(bn::music_items::msc_013);
	} else if(roomIndex <= 168) {
		doPlay(bn::music_items::msc_gorcircle_lo);
	} else if(roomIndex <= 196) {
		doPlay(bn::music_items::msc_levcircle);
	} else if(roomIndex <= 224) {
		doPlay(bn::music_items::msc_cifcircle);
	} else if(strstrCustom(RoomManager::currentRoomName(), "_bee_\0") != NULL ||
			strstrCustom(RoomManager::currentRoomName(), "_misc_\0") != NULL) {
		doPlay(bn::music_items::msc_beesong);
	} else if(strstrCustom(RoomManager::currentRoomName(), "_e_\0") != NULL) {
		doPlay(bn::music_items::msc_endless);
	} else if(strstrCustom(RoomManager::currentRoomName(), "_mon_0\0") != NULL ||
			strstrCustom(RoomManager::currentRoomName(), "_test_\0") != NULL) {
		doPlay(bn::music_items::msc_monstrail);
	} else {
		bn::music::stop();
	}

	// voided song is msc_voidsong
	// mon secret area bn::music_items::msc_monstrail
	// bee music??? (is it bn::music_items::msc_beesong)
	// dis bn::music_items::msc_endless
}

// -----

uint64_t GameSave::getSaveHash() {
	uint64_t res = 0;

	#define rotateHash(n) do { res++; res = (res << n) | (res >> ((sizeof(res) * 8) - n)); } while(false)

	// this is barely even a hash algorithm, but it will work ig

	res ^= locustCount;
	rotateHash(sizeof(locustCount) * 8);

	res ^= isVoided;
	rotateHash(1);

	res ^= roomIndex;
	rotateHash(sizeof(roomIndex) * 8);

	res ^= paletteIndex;
	rotateHash(sizeof(paletteIndex) * 8);

	res ^= mode;
	rotateHash(sizeof(mode) * 8);

	res ^= hasMemory;
	rotateHash(1);

	res ^= hasWings;
	rotateHash(1);

	res ^= hasSword;
	rotateHash(1);

	// why werent these hashed earlier???
	res ^= hasRod;
	rotateHash(1);

	res ^= hasSuperRod;
	rotateHash(1);

	// i should res eggcount here, but im worried abt (curse)ing ppls saves now
	// nvm, better than than corrruption

	res ^= eggCount;
	rotateHash(sizeof(eggCount) * 8);

	res ^= col1Save;
	rotateHash(sizeof(col1Save) * 8);

	res ^= col2Save;
	rotateHash(sizeof(col1Save) * 8);

	res ^= col3Save;
	rotateHash(sizeof(col1Save) * 8);

	res ^= col4Save;
	rotateHash(sizeof(col1Save) * 8);

	for(int i=0; i<6; i++) {
		res ^= playerBrand[i];
		rotateHash(sizeof(playerBrand[i]) * 8);
	}

	res ^= randomSeed;
	rotateHash(sizeof(randomSeed) * 8);

	res ^= delay;
	rotateHash(sizeof(delay) * 8);

	res ^= debug;
	rotateHash(sizeof(debug) * 8);

	return res;
}

void Game::save() {
	BN_LOG("saving save");

	if(RoomManager::isCustom) {
		return;
	}

	BN_ASSERT(EntityManager::player != NULL, "when saving save, the player was null!");

	saveData.locustCount = EntityManager::player->locustCount;
	saveData.isVoided = EntityManager::player->isVoided;

	saveData.hasMemory = EntityManager::player->hasMemory;
	saveData.hasWings = EntityManager::player->hasWings;
	saveData.hasSword = EntityManager::player->hasSword;

	saveData.roomIndex = RoomManager::roomIndex;
	saveData.paletteIndex = paletteIndex;
	saveData.mode = mode;

	BN_ASSERT(EntityManager::player->hasRod == 0 || EntityManager::player->hasRod == 1, "why was player hasrod ", EntityManager::player->hasRod);
	BN_ASSERT(EntityManager::player->hasSuperRod == 0 || EntityManager::player->hasSuperRod == 1, "why was player hasSuperRod ", EntityManager::player->hasSuperRod);

	saveData.hasRod = EntityManager::player->hasRod;
	saveData.hasSuperRod = EntityManager::player->hasSuperRod;

	if(saveData.hasRod) {
		saveData.hasRod = 1;
	} else {
		saveData.hasRod = 0;
	}

	if(saveData.hasSuperRod) {
		saveData.hasSuperRod = 1;
	} else {
		saveData.hasSuperRod = 0;
	}

	for(int i=0; i<6; i++) {
		saveData.playerBrand[i] = TileManager::playerBrand[i];
	}

	saveData.randomSeed = bruhRand();

	saveData.debug = debugToggle;

	saveData.hash = saveData.getSaveHash();
	bn::sram::write(saveData);
}

void Game::load() {
	BN_LOG("loading save");

	bn::sram::read(saveData);

	if(RoomManager::isCustom) {
		return;
	}

	if(saveData.hash != saveData.getSaveHash()) {
		BN_LOG("either a save wasnt found, or it was corrupted. creating new save");
		saveData = GameSave();
	}

	RoomManager::roomIndex = saveData.roomIndex;
	paletteIndex = saveData.paletteIndex;
	mode = saveData.mode;
	RoomManager::setMode(mode);

	if(saveData.col1Save == -1) {
		BN_LOG("SAVE DATA COLOR DATA WAS CORRUPTED??? HOW???");
		saveData.col1Save = 0;
		saveData.col2Save = 32767;
		saveData.col3Save = 25368;
		saveData.col4Save = 16912;
	}

	CUSTOM.b.set_data(saveData.col1Save);
	CUSTOM.c.set_data(saveData.col2Save);
	CUSTOM.d.set_data(saveData.col3Save);
	CUSTOM.e.set_data(saveData.col4Save);

	debugToggle = saveData.debug;

	for(int i=0; i<6; i++) {
		TileManager::playerBrand[i] = saveData.playerBrand[i];
	}
}

void Game::saveRNG() {

	GameSave saveDataStaging;

	bn::sram::read(saveDataStaging);

	if(saveDataStaging.hash != saveDataStaging.getSaveHash()) {
		return;
	}

	if(RoomManager::isCustom) {
		return;
	}

	saveData = saveDataStaging;

	saveData.randomSeed = bruhRand();

	saveData.hash = saveData.getSaveHash();
	bn::sram::write(saveData);
}
