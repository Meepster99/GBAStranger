
#include "Game.h"

#include "Palette.h"

Palette* BackgroundMap::backgroundPalette = &defaultPalette;

Game* globalGame = NULL;

unsigned int frame = 0;
bool isVblank = false;
unsigned boobaCount = 0;
unsigned playerMoveCount = 0;

bn::random randomGenerator = bn::random();

void Game::doButanoUpdate() {
	
	bn::core::update();
	
	int temp = bn::core::last_missed_frames();
	if(temp != 0) {
		BN_LOG("dropped frames: ", temp);
		BN_LOG("CPU:    ", bn::core::last_cpu_usage());
		BN_LOG("VBLANK: ", bn::core::last_vblank_usage());
	}
	
}	

void Game::uncompressData(u8 res[126], u8* input) {
	
	int i = 0;
	
	int val = -1;
	int count = -1;
	
	while(i <= 126) {
		
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
	
	
}

void Game::resetRoom(bool debug) {
	
	BN_LOG("entered reset room with debug=",debug);
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
	save();
	
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
	
	int collisionTileCount = collisionTiles->tiles_ref().size();
	int detailsTileCount = detailsTiles->tiles_ref().size();
	
	details.collisionTileCount = collisionTileCount;
	
	
	bn::regular_bg_tiles_ptr backgroundTiles = collision.rawMap.bgPointer.tiles();
		
	bn::optional<bn::span<bn::tile>> tileRefOpt = backgroundTiles.vram();
	BN_ASSERT(tileRefOpt.has_value(), "wtf");
	bn::span<bn::tile> tileRef = tileRefOpt.value();
	
	// copying to vram(directly) will cause issues when like,,,, going bullshitery relating to 
	// swapping via debug keys, but thats fine, its debug
	
	for(int i=0; i<collisionTileCount; i++) {
		
		BN_ASSERT(i < tileRef.size(), "out of bounds when copying tiles");
		
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
		
		
		BN_ASSERT((i + collisionTileCount) < tileRef.size(), "out of bounds when copying tiles");
		
		tileRef[i + collisionTileCount].data[0] = detailsTiles->tiles_ref()[i].data[0];
		tileRef[i + collisionTileCount].data[1] = detailsTiles->tiles_ref()[i].data[1];
		tileRef[i + collisionTileCount].data[2] = detailsTiles->tiles_ref()[i].data[2];
		tileRef[i + collisionTileCount].data[3] = detailsTiles->tiles_ref()[i].data[3];
		tileRef[i + collisionTileCount].data[4] = detailsTiles->tiles_ref()[i].data[4];
		tileRef[i + collisionTileCount].data[5] = detailsTiles->tiles_ref()[i].data[5];
		tileRef[i + collisionTileCount].data[6] = detailsTiles->tiles_ref()[i].data[6];
		tileRef[i + collisionTileCount].data[7] = detailsTiles->tiles_ref()[i].data[7];
	}
	
	details.collisionTileCount = collisionTileCount;
}

void Game::loadLevel(bool debug) {
	
	BN_LOG("entered loadlevel with debug=", debug);
	
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
	// a tile being 4 subtiles always fucks my math up
	// also, im no longer going to use a bitwise, im just going to add the number of collision tiles 
	//backgroundTiles = bn::regular_bg_tiles_ptr::allocate(collisionTileCount + detailsTileCount, bn::bpp_mode::BPP_4);
	

	
	loadTiles();
	
	
	// just in case the destructor isnt automatically called like,, do this
	// if we dont set the new bg ptr, the mem doesnt get freed
	//backgroundTiles = bn::regular_bg_tiles_ptr::allocate(1, bn::bpp_mode::BPP_4);
	//collision.rawMap.bgPointer.set_tiles(backgroundTiles);
	
	// this call is here to update the ref manager, and properly free the memory
	// it ispossible to maybe avoid this by calling update in bn_bg_blocks_manager directly
	//doButanoUpdate(); // these excess frame updates will just slow shit down
	
	// i could(and maybe should) realloc this every time 
	// but im wasting bg tiles on, stuff that i dont even know abt?
	// im going to try just having it be a const 512 again
	//backgroundTiles = bn::regular_bg_tiles_ptr::allocate(collisionTileCount + detailsTileCount, bn::bpp_mode::BPP_4);
	//collision.rawMap.bgPointer.set_tiles(backgroundTiles);
	
	//doButanoUpdate();
	
	if(needRedraw) {
		
		needRedraw = false;
		
		
	}
	//doButanoUpdate(); // these excess frame updates will just slow shit down
	
	

	for(int x=0; x<14; x++) { 
		for(int y=0; y<9; y++) {
			
			collisionMap[x][y] = uncompressedCollision[x + 14 * y];
			detailsMap[x][y] = uncompressedDetails[x + 14 * y];
			
			// when changing the color palettes, some weird shit happened where, areas 
			// would be transparent when they shouldnt be? this hopefully fixes that
		
			if(collisionMap[x][y] == 0) {
				collisionMap[x][y] = 1;
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
	
	
	BN_LOG("loadlevel completed");
}

void Game::fullDraw() {
	
	BN_LOG("entering fulldraw");
	
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
	
	static bool firstRun = true;
	
	const int paletteListSize = (int)(sizeof(paletteList) / sizeof(paletteList[0]));
	
	paletteIndex += offset;
	
	paletteIndex = ((paletteIndex % paletteListSize) + paletteListSize) % paletteListSize;
	
	pal = paletteList[paletteIndex];
	
	if(firstRun) {
		
		firstRun = false;
		
		entityManager.updatePalette(paletteList[paletteIndex]);
		effectsManager.updatePalette(paletteList[paletteIndex]);

		
		collision.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		//details.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		tileManager.floorLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		effectsManager.effectsLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		
		cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(paletteList[paletteIndex]->getBGPalette());
		
		BackgroundMap::backgroundPalette = paletteList[paletteIndex];
		
		// this is a problem, fuck it ima just have palette not cause a save.
		//save();
		
		
	} else {
	
		
		// oh boy, prepare for fun 
		// as far as i know, butano doesnt give me direct memory access, nor does it give me direct palette access.
		// this means that we are about to have a fun time with direct memory shit 
		// reminds me of the original gb 
		// going to have to probs go back and fix some of the random areas where i manually am setting a palette 
		// probs might actually make those funcs private
		// declare game as a friend class?
		
		// bg palette at 0x05000000 + 0x1E0
		// sprite palette at 0x05000000 + 0x3E0
		
		unsigned short* idek = reinterpret_cast<unsigned short*>(0x05000000 + 0x1E0);
		
		//BN_ERROR( pal->colorArray[0].red(), " ", pal->colorArray[0].green(), " ", pal->colorArray[0].blue(), " ", *idek);
		
		for(int i=0; i<5; i++) {
			unsigned short temp = 0;
			
			temp |= (pal->colorArray[i].blue() << 10);
			temp |= (pal->colorArray[i].green() << 5);
			temp |= (pal->colorArray[i].red() << 0);
		
			idek[i] = temp;
		}
		
		
		
	}
}

void didVBlank() {
	
	//frame = (frame + 1) % 600000;
	frame++;
	/*if(frame > 600000) {
		frame = 0;
	}*/
	// i save a branch? (yes)
	frame = frame & 0b1111111111111111;
	
	isVblank = true;
	globalGame->doVBlank();	
	randomGenerator.update();
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
			break;
		case GameState::Loading:
		case GameState::Paused:
			break;
		case GameState::Dialogue:
			effectsManager.doVBlank();
			break;
	}
	
	
}

void Game::run() {
	
	BN_LOG("look at u bein all fancy lookin in the logs");
	
	//u8* test = reinterpret_cast<u8*>(0x00001000);
	
	// why does simply instantiating the game class cause frame drops?
	// wont be much of an issue tho, i suppose
	//doButanoUpdate(); 
	
	globalGame = this;
	
	load();
	changePalette(0); // the paletteindex is already set by the load func, this just properly updates it
	

	//auto old effectsManager.effectsLayer.rawMap.bgPointer


	bn::core::set_vblank_callback(didVBlank);
	
	bn::timer inputTimer;
	
	state = GameState::Loading;
	
	//while(true) { bn::core::update(); }
	
	loadLevel();
	fullDraw();
	
	state = GameState::Normal;
	
	//effectsManager.doDialogue("Did you know every time you sigh, a little bit of happiness escapes?\0");
		
		
	doButanoUpdate();
	changePalette(1);
	cutsceneManager.introCutscene(); 
	
	//bn::core::update(); 

	
	//cutsceneManager.testCutscene(); 
	
	//while(true) { bn::core::update(); }
	
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
				
				while(miscTimer.elapsed_ticks() < FRAMETICKS * 5) { }
		
				doButanoUpdate();

				continue;
			}
			
			// pokemon style reset 
			if(bn::keypad::a_held() && bn::keypad::b_held() && bn::keypad::start_held() && bn::keypad::select_held()) {
				bn::core::reset();
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
			
			bn::fixed tickCount = inputTimer.elapsed_ticks();
			(void)tickCount; // supress warning if logging is disabled
			//BN_LOG("a move took ", tickCount / FRAMETICKS, " frames");
			BN_LOG("a move took ", tickCount.safe_division(FRAMETICKS), " frames");
		}
		
		
		doButanoUpdate();
	}
}

uint64_t Game::getSaveHash() {
	uint64_t hash = 0;
	
	#define rotateHash(n) hash = (hash << n) | (hash >> ((sizeof(hash) * 8) - n))
	
	// this is barely even a hash algorithm, but it will work ig
	
	hash ^= saveData.locustCount;
	rotateHash(sizeof(saveData.locustCount) * 8);
	
	hash ^= saveData.isVoided;
	rotateHash(sizeof(saveData.isVoided) * 8);
	
	hash ^= saveData.roomIndex;
	rotateHash(sizeof(saveData.roomIndex) * 8);

	hash ^= saveData.paletteIndex;
	rotateHash(sizeof(saveData.paletteIndex) * 8);
	
	hash ^= saveData.mode;
	rotateHash(sizeof(saveData.mode) * 8);
	
	hash ^= saveData.hasMemory;
	rotateHash(sizeof(saveData.mode) * 8);
	
	hash ^= saveData.hasWings;
	rotateHash(sizeof(saveData.mode) * 8);
	
	hash ^= saveData.hasSword;
	rotateHash(sizeof(saveData.mode) * 8);
	
	return hash;
}

void Game::save() {
	//BN_LOG("saving save");
	
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
	
	saveData.hash = getSaveHash();
	bn::sram::write(saveData);
	
	//BN_LOG("locust: ", saveData.locustCount);
	//BN_LOG("void: ", saveData.isVoided);
	//BN_LOG("room: ", saveData.roomIndex);
}

void Game::load() {
	BN_LOG("loading save");
	bn::sram::read(saveData);
	
	if(saveData.hash != getSaveHash()) {
		BN_LOG("either a save wasnt found, or it was corrupted. creating new save");
		saveData = GameSave();
	}
	
	BN_LOG("locust: ", saveData.locustCount);
	BN_LOG("void: ", saveData.isVoided);
	BN_LOG("room: ", saveData.roomIndex);

	roomManager.roomIndex = saveData.roomIndex;
	paletteIndex = saveData.paletteIndex;
	mode = saveData.mode;
	roomManager.setMode(mode);
}

void Game::playSound(const bn::sound_item* sound) {
	
	static unsigned prevFrame = -1;
	static unsigned soundsThisFrame = 0;
	
	// could maybe have a queue to store sounds that could be played on future frames?
	// but in my case, im going to have a SaneSet for sounds already played on this frame 
	// or maybe just a vector 
	// actually, no, SaneSet
	
	#define MAXSOUNDS 4
	
	static SaneSet<const bn::sound_item*, MAXSOUNDS> playedSounds;
	
	if(frame != prevFrame) {
		prevFrame = frame;
		soundsThisFrame = 0;
		playedSounds.clear();
	}
	
	if(state == GameState::Normal || state == GameState::Exiting || state == GameState::Paused || state == GameState::Dialogue) {
		if(soundsThisFrame < MAXSOUNDS && !playedSounds.contains(sound)) {
			sound->play();
			soundsThisFrame++;
			playedSounds.insert(sound);
		}	
	}	
}





