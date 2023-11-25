
#include "Game.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

Game* globalGame = NULL;

unsigned int frame = 0;
int playerIdleFrame = 0; // frame at which the player started idleing
bool isVblank = false;
unsigned boobaCount = 0;
unsigned playerMoveCount = 0;

// yep. its exactly what it looks like.

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
		globalGame->doButanoUpdate();
	}
}

void Game::doButanoUpdate() {
	
	bn::core::update();
	
	int temp = bn::core::last_missed_frames();
	
	bn::fixed vblankUsage = bn::core::last_vblank_usage();
	
	if(temp != 0) {
		BN_LOG("dropped frames: ", temp);
		BN_LOG("CPU:    ", bn::core::last_cpu_usage());
		BN_LOG("VBLANK: ", bn::core::last_vblank_usage());
	}
	
	if(vblankUsage > 1) {
		BN_LOG("VBLANK EXCEDED, THIS IS RLLY BAD: ");
		BN_LOG("CPU:    ", bn::core::last_cpu_usage());
		BN_LOG("VBLANK: ", bn::core::last_vblank_usage());
	}
	
}	

void Game::uncompressData(u8 res[126], u8* input) {
	
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
	
	BN_ASSERT(i == 126, "i wasnt equal to 126 after decomp.");
	
	
}

void Game::createExitEffects() {
	
	// this func was originally in entityManager->exit, but was moved.
	// tbh, it kinda shouldnt of been 
	// bc in the future, getting timings of what death animations to stay on screen 
	// before starting the transition will be, lets say spiritually taxing
	// tbh, that will require so much rewriting/jank i probs just wont
	
	// this function is horrid.
	
	// might not be needed
	//doButanoUpdate();
	
	Pos playerPos = entityManager.player->p;
	
	entityManager.createKillEffects();
	
	// determines if something(like a mon or lev statue) has killed the player
	// may cause issues if the player dies to multiple things at once??
	//bool customKill = entityManager.obstacleKill();
	
	// if a custom kill handler(like for mon or tan statues)
	// but tbh, instead of calling those effects inside the domoves func, they really should be called here.
	
	/*
	if(customKill) {
		return;
	}
	*/
	
	// tbh this if statement is legacy now that im having death effects get drawn
	// at sprite's screen poses, instead of their game coords, but im leaving it here bc im scaredas
	if(playerPos != entityManager.playerStart && entityManager.enemyKill()) {
		
		// this case means a enemy killed the player and the player moved, so we use the player start pos
		
		// this line should NOT be commented out
		// how do i discern a falldeath from running into an enemy vs, shadow walkoff?	
		Pos tempPos = playerPos;
		//if(tempPos.move(entityManager.player->currentDir)) {
		if(true) {
			if(entityManager.killAtPos(tempPos)) {
				playerPos = entityManager.playerStart;
			}
		}
		
		//entityManager.player->p = playerPos;
	}
	
	
	// i could use the enemykill/fallkill thigns, but tbh like 
	// in certain cases, where an enemey kills you above a floor like, yea 
	
	if(entityManager.playerWon()) {
		
	} else if(entityManager.monKill()) {
		
		BN_LOG("killing player via mon! special goofy case!");
		entityManager.addKill(entityManager.player);
		//entityManager.
		
		// some weird shit happens,,,, with mon statue kills 
		// the player fall anim is replaced with, the defaullt fall???	
	
	} else if(entityManager.hasFloor(playerPos)) {
		BN_LOG("killing player via enemy");
		// unsure if this is a good idea, but it will make other calls to fallKill work
		entityManager.addKill(entityManager.player);
		effectsManager.entityKill(entityManager.player);
	} else {
		BN_LOG("killing player via fall");
		
		
		entityManager.addKill(entityManager.player);
		effectsManager.entityFall(entityManager.player);
	}
	
	entityManager.player->p = playerPos;
	
}

void Game::findNextRoom() {
	

	if(entityManager.playerWon()) {
		
		entityManager.player->wingsUse = 0;
		
		entityManager.updateScreen(); // is this ok?


		int tileManagerRoomIndex = tileManager.getRoomIndex();
		int startRoomIndex = roomManager.roomIndex;
	
		// check for cif statue
		bool cifReset = false;
		Pos testPos = entityManager.player->p;
		if(testPos.move(Direction::Up)) {
			
			SaneSet<Entity*, 4> tempMap = entityManager.getMap(testPos);
			
			for(auto it = tempMap.begin(); it != tempMap.end(); ++it) {
				if((*it)->entityType() == EntityType::CifStatue) {
					cifReset = true;
					break;
				}	
			}
		}
		
		if(cifReset) {
			// todo, in the future, put a special anim here
			BN_ASSERT(entityManager.player != NULL, "player was null during cif reset");
			
			entityManager.player->locustCount = 0;
			entityManager.player->isVoided = false;

			//game->save(); // extranious call for sanity
			roomManager.cifReset();
		} else {
			if(tileManager.exitDestination == NULL) {
				roomManager.nextRoom();
			} else {
				roomManager.gotoRoom(tileManager.exitDestination);
			}
		}
		
		if(tileManagerRoomIndex != -1 && tileManagerRoomIndex != startRoomIndex) {
			if(tileManagerRoomIndex >= 256) {
				// goto the white rooms 
				roomManager.gotoRoom("rm_u_0001\0");
			} else {
				roomManager.gotoRoom(tileManagerRoomIndex);
			}
			return;
		}
		
		bn::sound_items::snd_stairs.play();
		return;
	} else {
		if(roomManager.isWhiteRooms()) {
			roomManager.roomIndex = 256; // this is bad 
			save();
			cutsceneManager.crashGame();
		}
	}
	
	// do a check for if we are doing a bee statue shortcut (or in the future, a shortcut shortcut)((or in the future future, brands???))
	// because of the goofy ahh way this is written, this is going to get called every time until the falling anim finishes?, i dont like that tbh, but i cannot do anything
	// i can at least like,,, do a check to not duplicate inc room via just setting locusts to 0
	if(entityManager.fallKill() && entityManager.player->locustCount != 0) {
		bool beeReset = false;
		Pos testPos = entityManager.player->p;
		if(testPos.move(Direction::Up)) {
			SaneSet<Entity*, 4> tempMap = entityManager.getMap(testPos);
			for(auto it = tempMap.begin(); it != tempMap.end(); ++it) {
				if((*it)->entityType() == EntityType::BeeStatue) {
						beeReset = true;
						break;
				}
			}
			
			if(beeReset) {
				BN_LOG("bee reset occured");
				roomManager.changeFloor(entityManager.player->locustCount);
				entityManager.player->locustCount = 0; // setting this to 0 should prevent,,, oofs when doing this (curse)
				tileManager.locustCounterTile->first = '0';
				tileManager.locustCounterTile->second = '0';
				
				// but does this update the save file?
			}
		}
	}
	
	if(entityManager.fallKill()) {
		bool foundGlass = false;
		const char* res = NULL;
		switch(roomManager.roomIndex) {
			case 131: // glass break room
				// check for the goofy ahh no glass shortcut
				
				for(int x=0; x<14; x++) {
					for(int y=0; y<9; y++) {
						if(tileManager.hasFloor(x, y) == TileType::Glass) {
							foundGlass = true;
							break;
						}
					}
				}
				
				if(!foundGlass) {
					roomManager.gotoRoom("rm_mon_shortcut_004");
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

				res = tileManager.checkBrand();
				
				if(res != NULL) {
					roomManager.gotoRoom(res);
				}
				
				break;
			default:
				break;
		}
	}

	if(tileManager.secretDestinations.size() != 0) {
		for(int i=0; i<tileManager.secretDestinations.size(); i++) {
			if(tileManager.secretDestinations[i].second == entityManager.player->p && !tileManager.hasFloor(entityManager.player->p)) {
				roomManager.gotoRoom(tileManager.secretDestinations[i].first);
				break;
			}
		}
	}
	
}

void Game::changeMusic() {
	
	int roomIndex = roomManager.roomIndex;

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
		
		static int prevMode = globalGame->mode;
		
		if(bn::music::playing() && bn::music::playing_item() == item && prevMode == globalGame->mode) {
			return;
		}
		
		prevMode = globalGame->mode;
		
		bn::fixed adjustVal = 1.0;
		
		if(globalGame->mode == 2 && item != bn::music_items::msc_voidsong) {
			adjustVal = 0.96;
		}
		
		item.play();
		
		bn::music::set_pitch(adjustVal);
		bn::music::set_tempo(adjustVal);
		
		
		
	};
	
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
	} else if(strstrCustom(roomManager.currentRoomName(), "_bee_\0") != NULL ||
			strstrCustom(roomManager.currentRoomName(), "_misc_\0") != NULL) {
		doPlay(bn::music_items::msc_beesong);
	} else if(strstrCustom(roomManager.currentRoomName(), "_e_\0") != NULL) {
		doPlay(bn::music_items::msc_endless);
	} else if(strstrCustom(roomManager.currentRoomName(), "_mon_0\0") != NULL ||
			strstrCustom(roomManager.currentRoomName(), "_test_\0") != NULL) {	
		doPlay(bn::music_items::msc_monstrail);
	} else {
		bn::music::stop();
	}
	
	// voided song is msc_voidsong
	// mon secret area bn::music_items::msc_monstrail
	// bee music??? (is it bn::music_items::msc_beesong)
	// dis bn::music_items::msc_endless
	
	
}

void Game::resetRoom(bool debug) {
	
	BN_LOG("entered reset room with debug=",debug);

	//player->wingsUse = 0; // this being here renders a bunch of my other patchwork bs irrelevent.
	// wait actually,,, i am not sure if i want it here. i only have one other patch case left so yea

	// summon all room exiting effects
	if(!debug && !entityManager.playerWon()) {
		createExitEffects();
	}

	// decide what room to goto next 
	if(!debug) {
		findNextRoom();
	}
	
	entityManager.player->locustCount = tileManager.getLocustCount();
	if(!debug) {
		if(!entityManager.player->isVoided && !entityManager.playerWon()) {
			if(entityManager.player->locustCount > 0) { 
				entityManager.player->locustCount--;
			} else {
				entityManager.player->isVoided = true;
			}
		}
	}
	
	
	BN_LOG("reseting to room ", roomManager.currentRoomName());
	
	if(!debug) {
		state = GameState::Exiting;
		
		// wait for animations to finish 
		while(state == GameState::Exiting) { // wait for gamestate to no longer be exiting
			doButanoUpdate();
		} 
		// this one extra update is here for
		// the mon lightning effect specifically, i think?
		doButanoUpdate();

	}

	state = GameState::Loading;
	
	cutsceneManager.resetRoom();
	queuedSounds.clear();
	removedSounds.clear();
	
	save();
	
	// this hopefully wont slow down debug moving much
	cutsceneManager.cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);
	
	changeMusic();
	
	loadLevel(debug);
	if(!debug) {
		doButanoUpdate();
	}
	
	fullDraw();
	
	state = GameState::Normal;
	
	if(!debug) {
		state = GameState::Entering;
		while(state == GameState::Entering) { // wait for gamestate to no longer be entering
			doButanoUpdate();
		}
	}
	
	BN_ASSERT(state == GameState::Normal, "after a entering gamestate, the next state should be normal");
	
	//bn::bg_tiles::log_status();
	
	BN_LOG("reset room done");
}

void Game::loadTiles() {
	
	
	Room idek = roomManager.loadRoom();
	
	const bn::regular_bg_tiles_item* collisionTiles = (const bn::regular_bg_tiles_item*)idek.collisionTiles;
	const bn::regular_bg_tiles_item* detailsTiles = (const bn::regular_bg_tiles_item*)idek.detailsTiles;
	
	int floorTileCount = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref().size();
	int collisionTileCount = collisionTiles->tiles_ref().size();
	int detailsTileCount = detailsTiles->tiles_ref().size();
	
	details.collisionTileCount = floorTileCount + collisionTileCount;
	
	bn::regular_bg_tiles_ptr backgroundTiles = collision.rawMap.bgPointer.tiles();
		
	bn::optional<bn::span<bn::tile>> tileRefOpt = backgroundTiles.vram();
	BN_ASSERT(tileRefOpt.has_value(), "wtf");
	bn::span<bn::tile> tileRef = tileRefOpt.value();
	
	// copying to vram(directly) will cause issues when like,,,, going bull(curse)ery relating to 
	// swapping via debug keys, but thats fine, its debug
	
	// i REALLLLY should use memcpy here
	// and only change areas of mem which actually need changes
	
	BN_ASSERT( floorTileCount + collisionTileCount + detailsTileCount < tileRef.size(), "didnt have enough alloc for tiles, need at least ", floorTileCount + collisionTileCount + detailsTileCount);
	
	for(int i=0; i<collisionTileCount; i++) {
		tileRef[i].data[0] = collisionTiles->tiles_ref()[i].data[0];
		tileRef[i].data[1] = collisionTiles->tiles_ref()[i].data[1];
		tileRef[i].data[2] = collisionTiles->tiles_ref()[i].data[2];
		tileRef[i].data[3] = collisionTiles->tiles_ref()[i].data[3];
		tileRef[i].data[4] = collisionTiles->tiles_ref()[i].data[4];
		tileRef[i].data[5] = collisionTiles->tiles_ref()[i].data[5];
		tileRef[i].data[6] = collisionTiles->tiles_ref()[i].data[6];
		tileRef[i].data[7] = collisionTiles->tiles_ref()[i].data[7];
	}
	
	
	for(int i=0; i<detailsTileCount; i++) {
		
		tileRef[i + collisionTileCount].data[0] = detailsTiles->tiles_ref()[i].data[0];
		tileRef[i + collisionTileCount].data[1] = detailsTiles->tiles_ref()[i].data[1];
		tileRef[i + collisionTileCount].data[2] = detailsTiles->tiles_ref()[i].data[2];
		tileRef[i + collisionTileCount].data[3] = detailsTiles->tiles_ref()[i].data[3];
		tileRef[i + collisionTileCount].data[4] = detailsTiles->tiles_ref()[i].data[4];
		tileRef[i + collisionTileCount].data[5] = detailsTiles->tiles_ref()[i].data[5];
		tileRef[i + collisionTileCount].data[6] = detailsTiles->tiles_ref()[i].data[6];
		tileRef[i + collisionTileCount].data[7] = detailsTiles->tiles_ref()[i].data[7];
	}
	
	if(roomManager.isWhiteRooms()) {
		
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
		for(int i=0; i<floorTileCount; i++) {
			tileRef[i + detailsTileCount + collisionTileCount].data[0] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[0];
			tileRef[i + detailsTileCount + collisionTileCount].data[1] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[1];
			tileRef[i + detailsTileCount + collisionTileCount].data[2] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[2];
			tileRef[i + detailsTileCount + collisionTileCount].data[3] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[3];
			tileRef[i + detailsTileCount + collisionTileCount].data[4] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[4];
			tileRef[i + detailsTileCount + collisionTileCount].data[5] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[5];
			tileRef[i + detailsTileCount + collisionTileCount].data[6] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[6];
			tileRef[i + detailsTileCount + collisionTileCount].data[7] = bn::regular_bg_tiles_items::dw_customfloortiles.tiles_ref()[i].data[7];
		}
	}
	
	details.collisionTileCount = collisionTileCount;
	tileManager.floorLayer.collisionTileCount = collisionTileCount + detailsTileCount;
}

void Game::loadLevel(bool debug) {
	
	BN_LOG("entered loadlevel with debug=", debug, " roomname=", roomManager.currentRoomName(), " roomindex = ", roomManager.roomIndex);
	
	//load();

	Room idek = roomManager.loadRoom();
	
	u8 uncompressedCollision[126];
	u8 uncompressedDetails[126];
	
	uncompressData(uncompressedCollision, (u8*)idek.collision);
	uncompressData(uncompressedDetails, (u8*)idek.details);
	
	//static const bn::regular_bg_tiles_item* collisionTiles = NULL;
	//static const bn::regular_bg_tiles_item* detailsTiles = NULL;
	
	/*
	const bn::regular_bg_tiles_item* collisionTiles = NULL;
	const bn::regular_bg_tiles_item* detailsTiles = NULL;
	
	const bn::regular_bg_tiles_item* newCollisionTiles = (const bn::regular_bg_tiles_item*)idek.collisionTiles;
	if(newCollisionTiles != collisionTiles) {
		needRedraw = true;
	}
	collisionTiles = newCollisionTiles;

	const bn::regular_bg_tiles_item* newDetailsTiles = (const bn::regular_bg_tiles_item*)idek.detailsTiles;
	if(newDetailsTiles != detailsTiles) {
		needRedraw = true;
	}
	detailsTiles = newDetailsTiles;
	
	int collisionTileCount = collisionTiles->tiles_ref().size();
	int detailsTileCount = detailsTiles->tiles_ref().size();
	*/
	
	//BN_ASSERT(collisionTileCount < 128 * 4, "collisionTileCount, wtf = ", collisionTileCount);
	//BN_ASSERT(detailsTileCount < 128 * 4, "detailsTileCount, wtf = ", detailsTileCount);
	
	// i could maybe avoid realloc each time,, but im not 100% sure 
	// a tile being 4 subtiles always (curse)s my math up
	// also, im no longer going to use a bitwise, im just going to add the number of collision tiles 
	//backgroundTiles = bn::regular_bg_tiles_ptr::allocate(collisionTileCount + detailsTileCount, bn::bpp_mode::BPP_4);
	

	static bool needRestore = false;
	
	if(needRestore) {
		//cutsceneManager.restoreAllButEffectsAndFloor();
		//tileManager.floorLayer.rawMap.bgPointer.set_priority(2);
		needRestore = false;
	}
	
	loadTiles();
	
	
	// just in case the destructor isnt automatically called like,, do this
	// if we dont set the new bg ptr, the mem doesnt get freed
	//backgroundTiles = bn::regular_bg_tiles_ptr::allocate(1, bn::bpp_mode::BPP_4);
	//collision.rawMap.bgPointer.set_tiles(backgroundTiles);
	
	// this call is here to update the ref manager, and properly free the memory
	// it ispossible to maybe avoid this by calling update in bn_bg_blocks_manager directly
	//doButanoUpdate(); // these excess frame updates will just slow (curse) down
	
	// i could(and maybe should) realloc this every time 
	// but im wasting bg tiles on, stuff that i dont even know abt?
	// im going to try just having it be a const 512 again
	//backgroundTiles = bn::regular_bg_tiles_ptr::allocate(collisionTileCount + detailsTileCount, bn::bpp_mode::BPP_4);
	//collision.rawMap.bgPointer.set_tiles(backgroundTiles);
	
	//doButanoUpdate();
	
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
			
			/*
			if(y == 8) {
				collisionMap[x][y] = 1;
				detailsMap[x][y] = 0;
				continue;
			}
			*/
			
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
	
	tileManager.loadTiles(floorPointer, secretsPointer, secretsCount, exitDest);
	
	if(!debug) {
		doButanoUpdate();
	}

	EntityHolder* entitiesPointer = (EntityHolder*)idek.entities;
	int entitiesCount = idek.entityCount;
	
	entityManager.loadEntities(entitiesPointer, entitiesCount);
	

	EffectHolder* effectsPointer = (EffectHolder*)idek.effects;
	int effectsCount = idek.effectsCount;
	
	effectsManager.loadEffects(effectsPointer, effectsCount);
	
	// should probs put the below into the cutscene file
	
	// this code actually rlly should of been in effects, omfg
	if(strcmp(roomManager.currentRoomName(), "rm_rm4\0") == 0 || strcmp(roomManager.currentRoomName(), "hard_rm_rm4\0") == 0) {
		needRestore = true;
		//cutsceneManager.backupAllButEffectsAndFloor();
		
		cutsceneManager.createPlayerBrandRoom();
		
		// this should of been programmed in as a bigsprite, but now that i know that i shouldnt use those, its going as a bg
		// the floor rlly should be combined onto the same layer as collision

	} else if(roomManager.currentRoomHash() == hashString("rm_u_end\0")) {
		//needRestore = true;
		//cutsceneManager.backupAllButEffectsAndFloor();
		
		
		cutsceneManager.createResetRoom();
	} else {
		if((strstrCustom(roomManager.currentRoomName(), "_u_00\0") == NULL) &&
			(strstrCustom(roomManager.currentRoomName(), "_u_en\0") == NULL)) {	
			cutsceneManager.backgroundLayer.rawMap.bgPointer.set_item(bn::regular_bg_items::dw_default_black_bg);
			cutsceneManager.backgroundLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		} else {
			cutsceneManager.backgroundLayer.rawMap.bgPointer.set_item(bn::regular_bg_items::dw_default_white_bg);
			cutsceneManager.backgroundLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		}
	}
	
	effectsManager.setBorderColor(!roomManager.isWhiteRooms());
	
	BN_LOG("loadlevel completed");
}

void Game::fullDraw() {
	
	BN_LOG("entering fulldraw");
	
	// THESE 3 CALLS SHOULD BE MERGED INTO ONE FUCKHEAD
	collision.draw(collisionMap);
	details.draw(detailsMap, collisionMap);
	tileManager.fullDraw();
	

	
	entityManager.fullUpdate();
	
	// i swear. why does the game crash without this print here?
	BN_LOG("fulldraw completed");
}

void Game::fullTileDraw() {
	collision.draw(collisionMap);
	details.draw(detailsMap, collisionMap);
	tileManager.fullDraw();
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
		cutsceneManager.inputCustomPalette();
	}
	*/
	
	entityManager.updatePalette(paletteList[paletteIndex]);
	effectsManager.updatePalette(paletteList[paletteIndex]);

	
	collision.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	//details.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	//tileManager.floorLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	effectsManager.effectsLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	
	cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	cutsceneManager.backgroundLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
	
	BackgroundMap::backgroundPalette = paletteList[paletteIndex];
	
	
	*col0 = pal->colorArray[0].data();
	*col1 = pal->colorArray[1].data();
	*col2 = pal->colorArray[2].data();
	*col3 = pal->colorArray[3].data();
	*col4 = pal->colorArray[4].data();
	
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

Game::~Game() {
	globalGame = NULL;
	bn::core::set_vblank_callback(doNothing);
	frame = 0;
}

void didVBlank() {
	
	//frame = (frame + 1) % 600000;
	frame++;
	/*if(frame > 600000) {
		frame = 0;
	}*/
	// i save a branch? (yes)
	frame = frame & 0b1111111111111111;
	
	BN_ASSERT(globalGame != NULL, "in vblank, globalgame was null");
	
	isVblank = true;
	globalGame->doVBlank();	
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
	
	static bool a, b, c = false;
	
	switch(state) {
		default:
		case GameState::Normal:
			//BN_LOG("entityManager.doVBlank();");
			entityManager.doVBlank();
			//BN_LOG("effectsManager.doVBlank();");
			effectsManager.doVBlank();
			//BN_LOG("tileManager.doVBlank();");
			tileManager.doVBlank();
			//BN_LOG("done");
			break;
		case GameState::Exiting:
			if(!a) { a = entityManager.exitRoom(); }
			if(!b) { b = effectsManager.exitRoom(); }
			if(!c) { c = tileManager.exitRoom(); }
			if(a && b && c) {
				state = GameState::Entering;
				a = b = c = false; // vine boom sound effect
			}
			break;
		case GameState::Entering:
			if(!a) { a = entityManager.enterRoom(); }
			if(!b) { b = effectsManager.enterRoom(); }
			if(!c) { c = tileManager.enterRoom(); }
			if(a && b && c) {
				state = GameState::Normal;
				a = b = c = false; // vine boom sound effect
			}
			// tick effects even while intro is occuring
			// this should of been here months ago.
			effectsManager.doVBlank(); 
			break;
		case GameState::Loading:
		case GameState::Paused:
			break;
		case GameState::Dialogue:
			effectsManager.doVBlank();
			break;
		case GameState::Cutscene:
			cutsceneManager.doVBlank();
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
	
	globalGame = this;
	
	roomManager.isCustomRooms();
	
	load();
	effectsManager.setBorderColor(!roomManager.isWhiteRooms());
	
	changePalette(0); // the paletteindex is already set by the load func, this just properly updates it
	
	bn::core::set_vblank_callback(didVBlank);
	
	bn::timer inputTimer;
	
	state = GameState::Loading;
	
	//cutsceneManager.titleScreen();
	
	bool brandBlank = true;
	for(int i=0; i<6; i++) {
		if(tileManager.playerBrand[i] != 0) {
			brandBlank = false;
			break;
		}
	}
	
	if(brandBlank && !roomManager.isCustom) {
		
		cutsceneManager.titleScreen();
		
		cutsceneManager.brandInput();
	}
	
	loadLevel();
	fullDraw();
	
	save();
	
	state = GameState::Normal;
	
	changeMusic();

	BN_LOG("starting main gameloop");
	while(true) {
		
		if(bn::keypad::any_held()) {
			if(bn::keypad::l_held() || bn::keypad::r_held()) {
				
				int debugIncrement = bn::keypad::select_held() ? 5 : 1;
				
				if(bn::keypad::l_held()) {
					for(int i=0; i<debugIncrement; i++) {
						roomManager.prevRoom();
					}
				} else {
					for(int i=0; i<debugIncrement; i++) {
						roomManager.nextRoom();
					}
				}
				resetRoom(true);
				
				miscTimer.restart();
				
				//while(miscTimer.elapsed_ticks() < FRAMETICKS * 5) { }
				//doButanoUpdate();

				int i=0;
				while(i<5) { i++; doButanoUpdate(); }
				
				continue;
			}
			
			// pokemon style reset 
			// changing this to have multiple options
			if(bn::keypad::a_held() && bn::keypad::b_held() && (bn::keypad::start_held() || bn::keypad::start_pressed())) {
				//THIS IS (curse), make it break out of this loop, and then put the actual Game* in a while func, and have it call its destructor and then reconstruct
				//bn::core::reset();
				BN_LOG("game reset called");
				break;
			}
			if(bn::keypad::a_held() && bn::keypad::b_held() && bn::keypad::select_held()) {
				//THIS IS (curse), make it break out of this loop, and then put the actual Game* in a while func, and have it call its destructor and then reconstruct
				//bn::core::reset();
				BN_LOG("save reset called");
				entityManager.player->locustCount = 0;
				entityManager.player->isVoided = false;
				save();
				break;
			}
		}
			
		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > FRAMETICKS * 3) {
			
			inputTimer.restart();
			
			if(bn::keypad::start_pressed()) {
				effectsManager.doMenu();	
				continue;
			}
	
			if(bn::keypad::select_pressed()) {
				continue;
			}
			
			if(bn::keypad::b_pressed()) {
				if(bn::keypad::select_held()) {
					Profiler::reset();
				} else {
					Profiler::show();
				}
				continue;
			}
			
			entityManager.doMoves();
		
			if(entityManager.hasKills()) {
				resetRoom();
				continue;
			}
			
			playerIdleFrame = frame;
			
			/*
			bn::fixed tickCount = inputTimer.elapsed_ticks();
			(void)tickCount; // supress warning if logging is disabled
			//BN_LOG("a move took ", tickCount / FRAMETICKS, " frames");
			if(tickCount > FRAMETICKS) {
				BN_LOG("a move took ", tickCount.safe_division(FRAMETICKS), " frames");
			}
			*/
		}
		
		doButanoUpdate();
	}
}

uint64_t Game::getSaveHash() {
	uint64_t hash = 0;
	
	#define rotateHash(n) do { hash++; hash = (hash << n) | (hash >> ((sizeof(hash) * 8) - n)); } while(false)
	
	// this is barely even a hash algorithm, but it will work ig
	
	hash ^= saveData.locustCount;
	rotateHash(sizeof(saveData.locustCount) * 8);
	
	hash ^= saveData.isVoided;
	rotateHash(1);
	
	hash ^= saveData.roomIndex;
	rotateHash(sizeof(saveData.roomIndex) * 8);

	hash ^= saveData.paletteIndex;
	rotateHash(sizeof(saveData.paletteIndex) * 8);
	
	hash ^= saveData.mode;
	rotateHash(sizeof(saveData.mode) * 8);
	
	hash ^= saveData.hasMemory;
	rotateHash(1);
	
	hash ^= saveData.hasWings;
	rotateHash(1);
	
	hash ^= saveData.hasSword;
	rotateHash(1);
	
	// i should hash eggcount here, but im worried abt (curse)ing ppls saves now
	// nvm, better than than corrruption 
	
	hash ^= saveData.eggCount;
	rotateHash(sizeof(saveData.eggCount) * 8);
	
	hash ^= saveData.col1Save;
	rotateHash(sizeof(saveData.col1Save) * 8);

	hash ^= saveData.col2Save;
	rotateHash(sizeof(saveData.col1Save) * 8);
	
	hash ^= saveData.col3Save;
	rotateHash(sizeof(saveData.col1Save) * 8);
	
	hash ^= saveData.col4Save;
	rotateHash(sizeof(saveData.col1Save) * 8);
	
	for(int i=0; i<6; i++) {
		hash ^= saveData.playerBrand[i];
		rotateHash(sizeof(saveData.playerBrand[i]) * 8);
	}
	
	hash ^= saveData.randomSeed;
	rotateHash(sizeof(saveData.randomSeed) * 8);
	
	return hash;
}

void Game::save() {
	//BN_LOG("saving save");
	
	if(roomManager.isCustom) {
		return;
	}
	
	BN_ASSERT(entityManager.player != NULL, "when saving save, the player was null!");
	
	saveData.locustCount = entityManager.player->locustCount;
	saveData.isVoided = entityManager.player->isVoided;
	
	saveData.hasMemory = entityManager.player->hasMemory;
	saveData.hasWings = entityManager.player->hasWings;
	saveData.hasSword = entityManager.player->hasSword;
	
	saveData.roomIndex = roomManager.roomIndex;
	saveData.paletteIndex = paletteIndex;
	saveData.mode = mode;
	
	saveData.hasRod = entityManager.player->hasRod;
	saveData.hasSuperRod = entityManager.player->hasSuperRod;
	
	for(int i=0; i<6; i++) {
		saveData.playerBrand[i] = tileManager.playerBrand[i];
	}
	
	saveData.randomSeed = bruhRand();
	
	saveData.hash = getSaveHash();
	bn::sram::write(saveData);
	
	//BN_LOG("locust: ", saveData.locustCount);
	//BN_LOG("void: ", saveData.isVoided);
	//BN_LOG("room: ", saveData.roomIndex);
}

void Game::load() {
	//BN_LOG("loading save");
	bn::sram::read(saveData);
	
	if(roomManager.isCustom) {
		return;
	}
	
	if(saveData.hash != getSaveHash()) {
		BN_LOG("either a save wasnt found, or it was corrupted. creating new save");
		saveData = GameSave();
	}
	
	//BN_LOG("locust: ", saveData.locustCount);
	//BN_LOG("void: ", saveData.isVoided);
	//BN_LOG("room: ", saveData.roomIndex);

	roomManager.roomIndex = saveData.roomIndex;
	paletteIndex = saveData.paletteIndex;
	mode = saveData.mode;
	roomManager.setMode(mode);
	
	CUSTOM.colorArray[1].set_data(saveData.col1Save);
	CUSTOM.colorArray[2].set_data(saveData.col2Save);
	CUSTOM.colorArray[3].set_data(saveData.col3Save);
	CUSTOM.colorArray[4].set_data(saveData.col4Save);
	
	
	for(int i=0; i<6; i++) {
		tileManager.playerBrand[i] = saveData.playerBrand[i];
	}
	
}

void Game::playSound(const bn::sound_item* sound) {
	
	// THIS FUNC SHOULD ONLY BE USED FOR SOUNDS WITH A POSSIBILITY OF BEING EXCLUDED
	
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
	
	if(playedSounds != 0) {
		BN_LOG("played ", playedSounds, " sound(s) (from the priority soundhandler)");
	}
	
	queuedSounds.clear();
	removedSounds.clear();
}



 