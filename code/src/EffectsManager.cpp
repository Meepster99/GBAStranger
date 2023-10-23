

#include "EffectsManager.h"

#include "Game.h"

Palette* EffectsManager::spritePalette = &defaultPalette;
Game* BigSprite::game = NULL;
EffectsManager* MenuOption::effectsManager = NULL;
int MenuOption::yIndex = -1;
EntityManager* BigSprite::entityManager = NULL;
TileManager* BigSprite::tileManager = NULL;
EffectsManager* BigSprite::effectsManager = NULL;


// ----- 

BigSprite::BigSprite(const bn::sprite_tiles_item* tiles_, int x_, int y_, int width_, int height_, bool collide_, int priority_, bool autoAnimate_) :
	width(width_), height(height_), tiles(tiles_), xPos(x_), yPos(y_), collide(collide_), priority(priority_), autoAnimate(autoAnimate_) {
	
	BN_ASSERT(tiles->tiles_ref().size() % (4 * width * height) == 0, "a bigsprite had a weird amount of tiles");
	
	optionCount = tiles->tiles_ref().size() / (4 * width * height);
	
	xPos -= (8 * width);
	yPos -= (8 * height);

	//draw(0);
	//firstDraw();
	
	// i dont like this code, but tbh im not sure of a better way around this.
	// i also really wish i could use a switch statement here
	// the decision of if to put the vars in the effectholder struct, or hard coded in here is difficult
	
	if(tiles == &bn::sprite_tiles_items::dw_spr_tail_boobytrap) {
		
		BN_LOG("booba detected");
		
		
		auto func1 = [](void* obj) -> void {
			
			BigSprite* bigSprite = static_cast<BigSprite*>(obj);
			
			static unsigned boobaBackup = 0;
			
			if(boobaCount != boobaBackup) {
				boobaBackup = boobaCount;
				
				bigSprite->effectsManager->doDialogue(
				"please dont touch me\rin that manner.\nyou'll regret it."
				);
				
				return;
			}
			
			// wtf. irlly need to switch over to namespaces
			if(bigSprite->effectsManager->entityManager->hasObstacle(Pos(12, 5))) {
				bigSprite->effectsManager->doDialogue(
				"i,,, why did you even do that?\rprobably wanted to see if\ri had programed it in\n"
				"well, i did\rlet me move that out of the way\n"
				"it might hurt a little bit though"
				);
				bigSprite->effectsManager->entityManager->addKill(*(bigSprite->effectsManager->entityManager->getMap(Pos(12, 5)).begin()));
				return;
			}
		
			
			static unsigned msgIndex = 0;
			
			constexpr MessageStr msgs[] = {
				{"you wouldnt believe how much the\ridiot who made this remake spent on animating\n"
			"my boobs, head, and tail.\n"
			"I would say i appreciate it, but\rto be honest, its just borerlining\ron creepy now.\n"
			"like, jesus christ. you dont even\rhave music, or shortcuts working\r"
			"but now you have boobs????\rwhy????????"},
			{"anyway, you know the drill\rhead down the stairs, good luck"}
			};
			
			// i rlly need to rewrite the dialogue system to automatically cut words.
			if(msgIndex < sizeof(msgs)) {
				
				bigSprite->effectsManager->doDialogue(msgs[msgIndex].str);
				
				if(msgIndex != sizeof(msgs)/sizeof(msgs[0]) - 1) {
					msgIndex++;
				}
			}
			
			
		
			return;
		};
		auto func2 = [](void* obj) -> bool {
			if(frame % 2 != 0) {
				return false;
			}
			
			if(boobaCount > 8 && randomGenerator.get_int(0, 256 - boobaCount) == 0) {
				BN_ERROR("excessive, overflow, booba\nto much booba\ntouch grass. or maybe take some estrogen and\nget your own.\nBooba Error Code: ", boobaCount);
			}
			
			static int timesCalled = 0;
			static_cast<BigSprite*>(obj)->animate(); 
			timesCalled++;
			if(timesCalled == 4) {
				timesCalled = 0;
				boobaCount++;
				return true;
			} 
			return false;
		};
			
		Interactable* temp1 = new Interactable(Pos(9, 3),
			func1,
			func2,
			NULL,
			(void*)this
		);
		
		// yes, these are the same object but with a slightly different pos, and yes, i am to scared to fucking copy them
		Interactable* temp2 = new Interactable(Pos(10, 3),
			func1,
			func2,
			NULL,
			(void*)this
		);
		
		entityManager->addEntity(temp1);
		entityManager->addEntity(temp2);
		
		for(int i=3 ; i<=8; i++) {
			
			Interactable* temp = new Interactable(Pos(i, 3),
				[](void* obj) -> void { (void)obj; return; },
				[](void* obj) -> bool { (void)obj; return true; },
				NULL,
				NULL
			);
			entityManager->addEntity(temp);
		}
		
		BN_ASSERT(tileManager->floorMap[9][4] == NULL, "bigsprite tried adding a tile when there was one already there??");
		BN_ASSERT(tileManager->floorMap[10][4] == NULL, "bigsprite tried adding a tile when there was one already there??");
		
		tileManager->floorMap[9][4] = new FloorTile(Pos(9,4));
		tileManager->floorMap[10][4] = new FloorTile(Pos(10,4));
		
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_tail_upperbody) {
		autoAnimateFrames = 3;
		autoAnimate = true;
		animationIndex = 2;
		//draw(2);
		
		// impliment the head movement here
		
		customAnimate = []() -> int {
			
			// this was a switch statement until i became a conspiracy theorist.
			BN_ASSERT(globalGame->entityManager.player != NULL, "WHAT THE FUCK");
			int playerX = globalGame->entityManager.player->p.x;
			if(playerX <= 6) {
				return 2;
			} else if(playerX <= 8) {
				return 1;
			} else if(playerX <= 10) {
				return 0;
			} else if(playerX <= 13) {
				return 3;
			} else {
				
			}
			
			return -1;
		};
		
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_tail_tail) { 
		autoAnimate = true;
		autoAnimateFrames = 24;
	} else{
		
	}
	
	// firstdraw occurs down here to ensure that all animation flags are properly set up.
	firstDraw();
	
}

void BigSprite::draw(int index) { profileFunction();
	
	// im going to do my best to avoid reallocing this array
	//sprites.clear();
	// alrighty kids its time to use 100% of my brain. 
	// could i, instead of doing all this sprite redrawing, just replace the tileset for each sprite?
	// issue is that bigsprites are all in one tileset. ill try to maybe just,, ugh change the offsets?
	// WHY ISNT THERE JUST A CHANGE GRAPHICS INDEX FUNC??? WHY DO I HAVE TO PASS THE TILES IN EACH TIME
	
	// im reimplimenting customanimate to just return a animindex
	
	// one issue will/would be, collision consistency. this function should/will only be called on animating things 
	//BN_ASSERT(autoAnimate, "tryed calling draw on a non animated bigsprite!");
	
	int spriteIndex = 0;
	int offset = index * width * height;
	
	for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {
						
			int spriteXPos = xPos + x * 16;
			int spriteYPos = yPos + y * 16;
			
			if(spriteXPos < -16 || spriteXPos > 240+16 || spriteYPos < -16 || spriteYPos > 160+16) {
				continue;
			}
			
			Sprite* tempSprite = &sprites[spriteIndex];
		
			tempSprite->spritePointer.set_tiles(
				*tiles,
				x + (y * width) + offset
			);
			
			spriteIndex++;
		}
	}
}

void BigSprite::firstDraw() {
	
	
	
	int indexOffset = animationIndex * 4 * width * height;
	
	for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {
						
			int spriteXPos = xPos + x * 16;
			int spriteYPos = yPos + y * 16;
			
			// check if this sprite will even be drawn onscreen.
			// check occurs first as it is inexpensive
			// a bit generious, just in case 
			if(spriteXPos < -16 || spriteXPos > 240+16 || spriteYPos < -16 || spriteYPos > 160+16) {
				continue;
			}
			
			
			if(!autoAnimate) {
				// dont do the sprite if this tile is blank. should really help with not hitting the sprite limit
				// is this 4, or ithis 8???
				// omfg dumbass, its 4 for the number of subtiles, idiot
				for(int j=0; j<4; j++) {
					// quite goofy, but basically the set_tiles func like, does the 16x16 tile math, but we need to do it manually here.
					int offset = 4 * (x + (y * width)) + j + indexOffset;
					
					BN_ASSERT(offset < tiles->tiles_ref().size(), "bigsprite tried to load a invalid tile. max tileindex = ", tiles->tiles_ref().size(), " tried loading at ", offset);
					
					// is this pointer like,,, what tf
					const uint32_t* tileRef = (tiles->tiles_ref())[offset].data;
					
					for(int i=0; i<8; i++) {
						if(tileRef[i] != 0) {
							goto doBigTile; // shit code
						}
					}
				}
				
				// is continueing here ideal? the collision map wont update for blank tiles. is that ok?
				continue;
			
			}
			doBigTile:

			Sprite tempSprite = Sprite(*tiles);
			
			tempSprite.updateRawPosition(spriteXPos, spriteYPos);
			
			int tempX = xPos/16 + x;
			int tempY = yPos/16 + y;
			
			// this array doesnt need to be updated each time we do an update draw omg
			if(collide) {
				game->collisionMap[tempX][tempY] = 2;
			}
			
			tempSprite.spritePointer.set_bg_priority(priority);
			
			tempSprite.spritePointer.set_tiles(
				*tiles,
				x + (y * width) + (animationIndex * width * height)
			);
			
			BN_LOG("attempting push");
			sprites.push_back(tempSprite);
			BN_LOG("push success");
		}
	}
}

void BigSprite::updatePalette(Palette* pal) {
	for(int i=0; i<sprites.size(); i++) {
		sprites[i].spritePointer.set_palette(pal->getSpritePalette());
	}
}

void BigSprite::animate() { profileFunction();
	
	// icould maybe make this branchless by having animate be a default, but im tired ok
	if(customAnimate == NULL) {
		animationIndex = (animationIndex + 1) % optionCount;
	} else {
		animationIndex = customAnimate();
	}
	
	draw(animationIndex);
}

// -----
	
EffectsManager::EffectsManager(Game* game_) : game(game_), textGenerator(dw_fnt_text_12_sprite_font), verTextGenerator(common::variable_8x8_sprite_font) {
		
	// may not be the best idea?
	//textGenerator.set_one_sprite_per_character(true);
	
	// copy over effectstiles
	bn::optional<bn::span<bn::tile>> tileRefOpt = tilesPointer.vram();
	BN_ASSERT(tileRefOpt.has_value(), "wtf");
	bn::span<bn::tile> tileRef = tileRefOpt.value();
	
	// now that these tiles can be made in memory, we could/should also change the,,, like, whats in them, but thats for later
	for(int i=0; i<bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref().size(); i++) {
		tileRef[i].data[0] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[0];
		tileRef[i].data[1] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[1];
		tileRef[i].data[2] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[2];
		tileRef[i].data[3] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[3];
		tileRef[i].data[4] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[4];
		tileRef[i].data[5] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[5];
		tileRef[i].data[6] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[6];
		tileRef[i].data[7] = bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref()[i].data[7];
	}

	/*
	for(int i=0; i<8; i+=2) {
		tileRef[126].data[i+0] = 0x00000002;
		tileRef[126].data[i+1] = 0x00000003;
	}	
	tileRef[127].data[0] = 0x23232323;
	*/
	
	for(int i=0; i<8; i++) {
		tileRef[126].data[i] = 0x11111111;
		tileRef[127].data[i] = 0x11111111;
	}
		
	for(int i=0+1; i<30-1; i++) {
		effectsLayer.rawMap.setTile(i, 0, 127, false, true);
		effectsLayer.rawMap.setTile(i, 19, 127, true, false);
	}

	for(int i=0+1; i<20-1; i++) {
		effectsLayer.rawMap.setTile(0, i, 126, true, false);
		effectsLayer.rawMap.setTile(29, i, 126, false, true);
	}

	
	effectsLayer.reloadCells();
	
}


void EffectsManager::updatePalette(Palette* pal) {
	
	for(int i=0; i<effectList.size(); i++) {
		effectList[i]->sprite.spritePointer.set_palette(pal->getSpritePalette());
	}
	
	for(int i=0; i<textSprites.size(); i++) {
		textSprites[i].set_palette(pal->getSpritePalette());
	}
	
	EffectsManager::spritePalette = pal;
	
	
	for(int i=0; i<bigSprites.size(); i++) {
		bigSprites[i]->updatePalette(pal);
	}
	
	for(int i=0; i<menuOptions.size(); i++) {
		menuOptions[i].draw();
	}
	
	for(int i=0; i<verTextSprites.size(); i++) {
		verTextSprites[i].set_palette(pal->getFontSpritePalette());
	}
	
	
}

// -----

bool EffectsManager::zoomEffect(bool inward) {
	
	static bool firstRun = true;
	static int layer = 0;
	if(firstRun) {
		firstRun = false;
		layer = inward ? 13 : 0;
	}
	
	if(frame % 5 != 0) {
		return false;
	}
	
	Pos p = entityManager->player->p;
	
	int increment = inward ? -1 : 1;
	int stopIndex = inward ? -1 : 14;
	
	if(layer != stopIndex) {
		
		// loop until we willdraw at least one thing 
		while(p.x-layer < 0 && p.y-layer < 0 && p.x+layer >= 14 && p.y+layer >= 9) {
			layer += increment;
			if(layer == stopIndex) {
				// breaking my static vars at bottom rule.
				firstRun = true;
				return true;
			}
		}
		

		Pos topLeft = safePos(p.x-layer, p.y-layer);
		Pos bottomRight = safePos(p.x+layer, p.y+layer);
		
		for(int x = topLeft.x; x<=bottomRight.x; x++) {
			if(p.y-layer >= 0) {
				effectsLayer.setBigTile(x, topLeft.y, inward);
			}
			if(p.y+layer < 9) {
				effectsLayer.setBigTile(x, bottomRight.y, inward);
			}
		}
		
	
		for(int y = topLeft.y; y<=bottomRight.y; y++) {
			if(p.x-layer >= 0) {
				effectsLayer.setBigTile(topLeft.x, y, inward);
			}
			if(p.x+layer < 14) {
				effectsLayer.setBigTile(bottomRight.x, y, inward);
			}
		}
	
			
		layer += increment;
		
		effectsLayer.update();
		return false;
	}
		
	
	
	firstRun = true;
	return true;
}

bool EffectsManager::topDownEffect(bool downward) {
	
	static int nextLine = 0;
	static int fillLevel = 0;
	
	if(nextLine < 9) {

		for(int x=0; x<14; x++) {
			//effectsLayer.setBigTile(x, nextLine, 1+16-fillLevel);
			effectsLayer.setBigTile(x, nextLine,
				downward ? 1+16-fillLevel : 1+fillLevel, false, !downward);
		}
		fillLevel+=4;
		
		if(fillLevel > 16) {
			fillLevel = 0;
			nextLine++;
		}
		
		effectsLayer.update();
		return false;
	}
	
	// reset static var
	nextLine = 0;
	fillLevel = 0;
	return true;
}

bool EffectsManager::exitRoom() {
	
	// if we have a locusts death, flash to white 
	// if voided death, do a black from top down 
	// if we exit successfully, do the box zoom on the exit.
	
	// wait a lil before actually starting our anims here
	// i rlly wish all these static vars were more elegant, but im basically 
	// trying to be safe by being sure that the only place where true returns 
	// is the end of the func, right after we reset those vars, or something close to that
	
	static bool firstRun = true;
	static unsigned int firstFrame = 0;
	if(firstRun) {
		firstRun = false;
		firstFrame = frame;
		return false;
	}
	
	if(frame - firstFrame < 60 * 1) {
		return false;
	}
	
	bool res = entityManager->playerWon() ? zoomEffect(true) : topDownEffect(true);

	if(res) {
		firstRun = true;
		playerWonLastExit = entityManager->playerWon(); // wtf am i on
		return true;
	}
	
	return false;
}

bool EffectsManager::enterRoom() {
	
	static bool firstRun = true;
	static unsigned int firstFrame = 0;
	if(firstRun) {
		firstRun = false;
		firstFrame = frame;
		return false;
	}
	
	if(frame - firstFrame < 60 * 1) {
		return false;
	}
	
	bool res = playerWonLastExit ?  zoomEffect(false) : topDownEffect(false);

	if(res) {
		firstRun = true;
		return true;
	}
	
	return false;
}

void EffectsManager::doVBlank() { profileFunction();
	
	// eventually, we should call ticks on random misc effects here
	// what if i only do bigsprite updates on non,,, everything else frames?
	
	for(int i=0; i<bigSprites.size(); i++) {
		if(bigSprites[i]->autoAnimate) {
			if(frame % bigSprites[i]->autoAnimateFrames == 0) {
				bigSprites[i]->animate();
			}
		}
	}
	
	for(auto it = effectList.begin(); it != effectList.end(); ) {
		
		bool res = (*it)->animate();
		
		if(res) {
			delete (*it);
			(*it) = NULL;
			it = effectList.erase(it);
		} else {
			++it;
		}
	}

	if(frame % 4 != 0) {
		return;
	}
	
	return;

	static Pos spiralPos(3, 3);
	static int layerIndex = 0;
	static int layer = 1;
	static int layerCount = 0;
	static int dirIndex = 0;
	const Direction dirArray[4] = {Direction::Down, Direction::Right, Direction::Up, Direction::Left};

	// should this be static? should this be a class var honestly?
	static bn::span<bn::tile> tileRef = tilesPointer.vram().value();
	
	if(layerIndex == layer) {
		dirIndex = (dirIndex + 1) % 4;
		layerIndex = 0;
			
		layerCount++;
		if(layerCount == 2) {
			layerCount = 0;
			layer++;
		}
		if(layer == 8 && layerCount == 1) {
			layer = 1;
			spiralPos = Pos(3, 3);
			dirIndex = 0;
			layerCount = 0;
		}
	}
	

	if(layerIndex < layer) {
		//BN_ASSERT(spiralPos.y >=0 && spiralPos.y < 8, "spiralPos y out of range at ", spiralPos.y);
		//BN_ASSERT(spiralPos.x >=0 && spiralPos.x < 8, "spiralPos x out of range at ", spiralPos.x);
		
		//uint32_t val = tileRef[126].data[spiralPos.y] ^ (((uint32_t)2) << 4*spiralPos.x);
		
		uint32_t val = tileRef[126].data[spiralPos.y];
		uint32_t temp = (val & (((uint32_t)0xF) << 4*spiralPos.x)) >> 4*spiralPos.x;
		
		temp = (temp % 4) + 1;
		val = (val & ~(((uint32_t)0xF) << 4*spiralPos.x)) | (temp << 4*spiralPos.x) ;
		
		tileRef[126].data[spiralPos.y] = val;
		tileRef[127].data[spiralPos.y] = val;
		
		spiralPos.move(dirArray[dirIndex]);
		
		layerIndex++;
	}
	
	
	
	
	
	
	
}

void EffectsManager::loadEffects(EffectHolder* effects, int effectsCount) {

	// eventually, cleanup leftover effects here

	for(int i=0; i<effectList.size(); i++) {
		if(effectList[i] != NULL) {
			delete effectList[i];
		}
		effectList[i] = NULL;
	}
	
	effectList.clear();
	
	for(int i=0; i<bigSprites.size(); i++) {
		if(bigSprites[i] != NULL) {
			delete bigSprites[i];
		}
		bigSprites[i] = NULL;
	}
	
	bigSprites.clear();
	
	if(effectsCount == 1) { // the first effect is just a dummy to avoid 0 length arrays
		return;
	}
	
	effectsCount--;
	effects++;
	
	for(int i=0; i<effectsCount; i++) {
		if(effects->width == 1 && effects->height == 1) { // a smallsprite should be summoned as an effect
			if(effects->tiles == &bn::sprite_tiles_items::dw_spr_stinklines) {
				
				BN_LOG("kys", effects->x, " ", effects->y);
				
				// minus 1 to play infinitely?
				// nvm, gods idek what im doing 
				//EffectTypeArray questionMark[] = {EffectType(bn::sprite_tiles_items::dw_spr_stinklines, 9999999)};
				//createEffect(Pos(effects->x / 16, effects->y / 16), EffectTypeCast(questionMark));
				
			} else {
				BN_ERROR("you are a idiot(not you, but me)");
			}
		} else {
			bigSprites.push_back(new BigSprite(effects->tiles, effects->x, effects->y, effects->width, effects->height, effects->collide, effects->priority, effects->autoAnimate) );
		}
		effects++;
	}
	
	
}

// -----

void EffectsManager::hideForDialogueBox(bool vis) {
	
	entityManager->hideForDialogueBox(vis);
	
	for(auto it = effectList.cbegin(); it != effectList.cend(); ++it) {
		if((*it) == NULL) {
			continue;
		}
		if((*it)->getPos().y >= 6) {
			(*it)->sprite.spritePointer.set_visible(vis);
		}
    }
}

void EffectsManager::doDialogue(const char* data) {
	
	// this function has got to be one of the worst things i have ever written in my life.

	// some notes 
	// in order for the text to properly scroll, we are going to need to temporarily overwrite the floor.
	// also, the onesprite per like, ugh 
	// ideally, i would be using background tiles to hide the sprites so i could disable the one char per sprite thing.
	// althox2, i suppose that like,,,, ugh idek anymore .
	// im just going to pray i dont hit that limit(but i def will)
	// will probs need to do some redrawing, with like, a max of 4 lines of sprites actually drawn?
	// ugh
	

	GameState restoreState = game->state;
	game->state = GameState::Paused;
	
	for(int x=0; x<14; x++) {
		tileManager->floorLayer.setBigTile(x, 6, 0);
		tileManager->floorLayer.setBigTile(x, 7, 0);
		tileManager->floorLayer.setBigTile(x, 8, 0);
		
		game->details.setBigTile(x, 6, 0);
		game->details.setBigTile(x, 7, 0);
		game->details.setBigTile(x, 8, 0);
		
		game->collision.setBigTile(x, 6, 0);
		game->collision.setBigTile(x, 7, 0);
		game->collision.setBigTile(x, 8, 0);
		
	}
	
	tileManager->floorLayer.rawMap.reloadCells();
	game->collision.reloadCells();
	game->details.reloadCells();
	
	effectsLayer.setBigTile(0, 6, 19);
	effectsLayer.setBigTile(13, 6, 20);
	effectsLayer.setBigTile(0, 8, 21);
	effectsLayer.setBigTile(13, 8, 22);
	effectsLayer.setBigTile(0, 7, 25);
	effectsLayer.setBigTile(13, 7, 26);
	
	for(int i=1; i<=12; i++) {
		effectsLayer.setBigTile(i, 6, 23);
		effectsLayer.setBigTile(i, 8, 24);
		effectsLayer.setBigTile(i, 7, 18);
	}
	
	effectsLayer.update();
	
	hideForDialogueBox(false);
	
	// idk how long to make this
	static char buffer[128];
	for(int i=0; i<128; i++) {
		buffer[i] = '\0';
	}
	
	// ok heres the plan. 
	// nullterm is the end of the text 
	// newline indicates the player should have to press z 
	// carriage return indicates the that like, to continue this text but on the next line
	
	game->doButanoUpdate();
	bool pressQueued = true;
	
	bool newText = false;
	bool currentlyScrolling = false;
	int textShowIndex = 0;
	while(true) {
		
		if(bn::keypad::a_pressed() || pressQueued) {
			if(!newText) { // get a whole new thing of text
				pressQueued = false;
				game->doButanoUpdate();
				
				// this a press will carry over from when the interaction was initiated, meaning that like,,, 
				// i dont need to have a function which gets the line both inside and outside of this loop
				
				textSprites.clear();
				
				if(*data == '\n') {
					data++;
				} else if(*data == '\0') {
					break;
				}
				
				int textOffset = 0; // where to draw this text in terms of verticality
				
				drawNextLine:
				
				char* bufferPointer = buffer;
				
				while(*data != '\n' && *data != '\r' && *data != '\0') {
					*bufferPointer++ = *data++;
				}
				*bufferPointer = '\0';
				
				BN_ASSERT(bufferPointer - buffer < 128, "overflowed the dialogue buffer! ", buffer);				
				
				textGenerator.generate((bn::fixed)-120+8+4+4, (bn::fixed)40 + textOffset*16, bn::string_view(buffer), textSprites);


				// this section of code should MAYBE be below the below check
				// nope, moving this finally fixed the rare framedrops	
				for(int i=0; i<textSprites.size(); i++) {
					textSprites[i].set_bg_priority(3);
					textSprites[i].set_visible(false);
					textSprites[i].set_palette(spritePalette->getSpritePalette());
				}
				game->doButanoUpdate();

				if(*data == '\r') {
					// bad code 
					data++;
					textOffset++;
					goto drawNextLine;
				}

				

				newText = true;
				textShowIndex = 0;			
			} else { // scroll the existing text
				if(currentlyScrolling) { // we never finished displaying the current text, so do that.
					game->doButanoUpdate();
					for(int i=0; i<textSprites.size(); i++) {
						if(textSprites[i].y() == 40 || textSprites[i].y() == 40+16) {
							textSprites[i].set_visible(true);
							textShowIndex = i + 1;
						}
					}
				} else { // we finished the current text, so scroll up.
					for(int unused=0; unused<16; unused++) {
						for(int i=0; i<textSprites.size(); i++) {
							textSprites[i].set_y(textSprites[i].y() - 1);
						}
						game->doButanoUpdate();
					}
				}
			}
		}
	
		// oh my gods. i dont need to rewrite the text sprite generator! i can just use sprite visibility/prio, i am so dumb
		if(frame % 2 == 0) {
			if(newText && textShowIndex == textSprites.size()) {
				newText = false;
			} else if(newText && textShowIndex < textSprites.size()) {
				if(textSprites[textShowIndex].y() == 40 || textSprites[textShowIndex].y() == 40+16) {
					// sprite is in one of the two valid lines, display it.
					textSprites[textShowIndex].set_visible(true);
					textShowIndex++;
					currentlyScrolling = true;
				} else {
					currentlyScrolling = false;
				}
			}
		}
		
		game->doButanoUpdate();
	}
	
	game->doButanoUpdate();
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 0);
		}
	}
	effectsLayer.update();
	
	// this is trash, and will cause frame drops 
	game->fullTileDraw();

	hideForDialogueBox(true);	
	game->doButanoUpdate();
	
	
	textSprites.clear();
	game->state = restoreState;
	

}

// -----

// jesus fuck
MenuOption::MenuOption(const char* optionName_, const char* (*getOption_)(), void (*changeOption_)(int), int xVal) :
	optionName(optionName_),
	getOption(getOption_),
	changeOption(changeOption_),
	textGenerator(dw_fnt_text_12_sprite_font)
	{
		yDraw = yIndex;
		if(xVal == -1) {
			yIndex += 16;
		} else if(xVal == -2) {
			yIndex += 32;
			yDraw += 16;
		} else {
			xDraw = xVal;
		}
	}


void MenuOption::fullDraw(bool isActive) { // use white color for active, use darkest ver for non active
	isActiveState = isActive;
	textSprites.clear();

	strcpy(buffer, optionName);
	strcpy(buffer + WTF(optionName), getOption());
	
	// why is this so pathetically slow?
	textGenerator.generate((bn::fixed)-104 + xDraw, (bn::fixed)yDraw, bn::string_view(buffer), textSprites);
	
	auto spritePalettePalette = effectsManager->spritePalette->getAlternateSpritePalette();
	if(isActive) {
		spritePalettePalette = effectsManager->spritePalette->getSpritePalette();
	}
	
	
	for(int i=0; i<textSprites.size(); i++) {
		textSprites[i].set_palette(spritePalettePalette);
		textSprites[i].set_bg_priority(0);
		textSprites[i].set_visible(true);
	}
	
}

void MenuOption::draw(bool isActive) {
	isActiveState = isActive;
	for(int i=0; i<textSprites.size(); i++) {
	
		if(isActive) {
			textSprites[i].set_palette(effectsManager->spritePalette->getSpritePalette());
		} else {
			textSprites[i].set_palette(effectsManager->spritePalette->getAlternateSpritePalette());
		}
	}
}

void MenuOption::draw() {
	draw(isActiveState);
}

void EffectsManager::doMenu() {
	
	// room switching, profiler reseting, palete switching, and such
	// also show compile time, date, and butano version 
	// BN_VERSION_STRING   __DATE__   __TIME__
	
	GameState restoreState = game->state;
	game->state = GameState::Paused;
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 1);
		}
	}
	effectsLayer.reloadCells();
	game->doButanoUpdate();
	


	#define VERMSG1 "made with love with:" 
	const char* vermsgString1 = VERMSG1;
	
	#if defined(ENABLEPROFILER)
		#define VERMSG2 "butano version " BN_VERSION_STRING " with log=1 prof=1"
	#elif defined(ENABLELOGGING)
		#define VERMSG2 "butano version " BN_VERSION_STRING " with log=1"
	#else 
		#define VERMSG2 "butano version " BN_VERSION_STRING
	#endif
	

	const char* vermsgString2 = VERMSG2;
	
	#define VERMSG3 "on " __DATE__ " at " __TIME__
	const char* vermsgString3 = VERMSG3;
	
	#define VERMSG4 "actual game by System Erasure"
	const char* vermsgString4 = VERMSG4;
	
	#define VERMSG5 "BlueberryJam palette by gooeyphantasm"
	const char* vermsgString5 = VERMSG5;
	
	#define VERMSG6 "pls msg meepster99 on disc 2 report bugs"
	const char* vermsgString6 = VERMSG6;
	
	verTextSprites.clear();
	
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)24, bn::string_view(vermsgString1), verTextSprites);
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)32, bn::string_view(vermsgString2), verTextSprites);
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)40, bn::string_view(vermsgString3), verTextSprites);
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)48, bn::string_view(vermsgString4), verTextSprites);
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)56, bn::string_view(vermsgString5), verTextSprites);
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)64, bn::string_view(vermsgString6), verTextSprites);

	
	
	
	
	for(int i=0; i<verTextSprites.size(); i++) {
		verTextSprites[i].set_palette(spritePalette->getFontSpritePalette());
		verTextSprites[i].set_bg_priority(0);
		verTextSprites[i].set_visible(true);
	}
	
	//MenuOption::yIndex = -60;
	MenuOption::yIndex = -68;
	//MenuOption::yIndex = -70;
	
	// oh god im getting goofy again
	menuOptions.push_back(
		MenuOption("Stranger: ", 
		[]() -> const char* { return globalGame->getMode(); },
		[](int val) { globalGame->changeMode(val); }
		)
	);
	
	menuOptions.push_back(
		MenuOption("Room: ", 
		[]() -> const char* { return globalGame->roomManager.currentRoomName(); },
		[](int val) { return globalGame->roomManager.changeFloor(val); }
		)
	);
	
	menuOptions.push_back(
		MenuOption("Palette: ", 
		[]() -> const char* { return paletteNameList[globalGame->paletteIndex]; },
		[](int val) { return globalGame->changePalette(val); }
		)
	);
	
	menuOptions.push_back(
		MenuOption("Memory: ", 
		[]() -> const char* { 
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			return player->hasMemory ? "yay" : "nay";
		},
		[](int val) { 	
			(void)val;
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			player->hasMemory = !player->hasMemory;
			globalGame->tileManager.updateBurdenTiles();
		},
		80 * 0
		)
	);
	
	menuOptions.push_back(
		MenuOption("Wings: ", 
		[]() -> const char* { 
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			return player->hasWings ? "yay" : "nay";
		},
		[](int val) { 
			(void)val;
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			player->hasWings = !player->hasWings;
			globalGame->tileManager.updateBurdenTiles();
		},
		80 * 1
		)
	);

	
	menuOptions.push_back(
		MenuOption("Sword: ", 
		[]() -> const char* { 
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			return player->hasSword ? "yay" : "nay";
		},
		[](int val) { 
			(void)val;
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			player->hasSword = !player->hasSword;
			globalGame->tileManager.updateBurdenTiles();
		},
		80 * 2 - 12
		)
	);
	
	menuOptions.push_back(
		MenuOption("Rod: ", 
		[]() -> const char* { 
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			return player->hasSuperRod ? "super" : "normal";
		},
		[](int val) { 
			(void)val;
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
			player->hasSuperRod	= !player->hasSuperRod;
			globalGame->tileManager.updateRod();
		},
		-2
		)
	);
	
	menuOptions.push_back(
		MenuOption("Back", 
		[]() -> const char* { return "\0"; },
		[](int val) { (void)val; return; }
		)
	);
	
	game->doButanoUpdate();

	bn::timer tempTimer;
	
	for(int i=0; i<menuOptions.size(); i++) {
		tempTimer.restart();
		menuOptions[i].fullDraw(i == 0);
		// this many butanoupdates SHOULD NOT BE NECCESSARY!
		game->doButanoUpdate();
	}
	
	
	// everything in here could(and should) of been done with just palette table manip omfg
	
	int selectedOption = 0;
	bool isActive = false;
	bool flashing = false;
	
	int startRoomIndex = game->roomManager.roomIndex;
	int startRoomMode = game->mode;
	
	while(true) {
		
		if(bn::keypad::any_pressed()) {
			// i rlly with they gave me direct access to bn::keypad::data
			
			if(!isActive && bn::keypad::up_pressed()) {
				menuOptions[selectedOption].draw(false);
				
				selectedOption--;
				selectedOption = ((selectedOption % menuOptions.size()) + menuOptions.size()) % menuOptions.size();
				
				menuOptions[selectedOption].draw(true);
			} else if(!isActive && bn::keypad::down_pressed()) {
				menuOptions[selectedOption].draw(false);
				
				selectedOption++;
				selectedOption = ((selectedOption % menuOptions.size()) + menuOptions.size()) % menuOptions.size();
				
				menuOptions[selectedOption].draw(true);
			} else if(isActive && bn::keypad::left_pressed()) {
				menuOptions[selectedOption].changeOption(-1);
				menuOptions[selectedOption].fullDraw(flashing);
			} else if(isActive && bn::keypad::right_pressed()) {
				menuOptions[selectedOption].changeOption(1);
				menuOptions[selectedOption].fullDraw(flashing);
			} else if(bn::keypad::a_pressed()) {
				isActive = !isActive;
				menuOptions[selectedOption].draw(true);
				
				if(selectedOption == menuOptions.size() - 1) {
					break;
				}
				
			} else {
				
			}
		}
		
		if(isActive && frame % 8 == 0) {
			menuOptions[selectedOption].draw(flashing);
			flashing = !flashing;
		}

		
		game->doButanoUpdate();
	}
	

	verTextSprites.clear();
	// this causes frame drops, and isnt ideal, but will work for now
	if(startRoomIndex != game->roomManager.roomIndex ||
		startRoomMode != game->mode) {
		game->resetRoom(true);
	}
	menuOptions.clear();
	tileManager->fullDraw(); // literally only here since for some reason, butden tiles didnt update until a move?
	game->doButanoUpdate();
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 0);
		}
	}
	effectsLayer.reloadCells();
	game->doButanoUpdate();
	
	game->state = restoreState;

	
}

// -----

void EffectsManager::glassBreak(Pos p) {
	
	// we cannot use static vars in this joint, so static "vars" will have to be,,, alloced in real time. i think?
	// or i can just use the canerious std::functional
	// gods this is absolutely horrid.
	// i really should of just made fucking,, i should of just made graphicsIndex a static var 
	// i am assuming i will need more static vars in the future when im not even rlly sure, ugh idk
	

	createEffect(
	[p](Effect* obj) mutable -> void {
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_glassfloor,
			0
		);
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		obj->sprite.updateRawPosition(obj->x, obj->y);
	},
	[](Effect* obj) mutable -> bool {
		obj->graphicsIndex++;
		if(obj->graphicsIndex == 6) {
			return true;
		}
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_glassfloor,
			obj->graphicsIndex
		);
		return false;
	}
	);
}

void EffectsManager::voidRod(Pos p, Direction dir) {

	/* 
	swipe is:
	to the left when facing up
	to the right when facing down
	and to the down when facing left and right
	
	*/
	
	// swipe
	createEffect(
	[p, dir](Effect* obj) mutable -> void {
		
		obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_player_swipe, bn::sprite_tiles_items::dw_spr_player_swipe_shape_size);
		
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_player_swipe,
			0
		);
		
		p.moveInvert(dir, true, true);
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		obj->y += 8;
		obj->x -= 8;
		
		// to tired to cast to int
		if(dir == Direction::Up) {
			obj->sprite.spritePointer.set_rotation_angle(180);			
			obj->y -= 16;
		} else if(dir == Direction::Down) {
			obj->x += 16;
		} else { // left or right
			obj->sprite.spritePointer.set_rotation_angle(270);			
			if(dir == Direction::Right) {
				obj->sprite.spritePointer.set_vertical_flip(true);
				obj->x += 16;
			}
		}
		
		obj->sprite.updateRawPosition(obj->x, obj->y);
		obj->sprite.spritePointer.set_z_order(-2);
	},
	[](Effect* obj) mutable -> bool {
		obj->graphicsIndex++;
		if(obj->graphicsIndex == 10) {
			return true;
		}
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_player_swipe,
			obj->graphicsIndex
		);
		return false;
	}
	);
		
	// gods i really should of just had a class var wtf is wrong with me
	// im going to change it now, but tbh like, im going to still keep std::functional in, since its helpful for stuff like p and dir
	
	// sparkle
	createEffect(
	[p, dir](Effect* obj) mutable -> void {
		
		obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_sparkle);
		
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_sparkle,
			0
		);
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		
		obj->sprite.updateRawPosition(obj->x, obj->y);
		obj->sprite.spritePointer.set_z_order(-3);
	},
	[](Effect* obj) mutable -> bool {
		obj->graphicsIndex++;
		if(obj->graphicsIndex == 8) {
			return true;
		}
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_sparkle,
			obj->graphicsIndex
		);
		return false;
	}
	);
	
	
	// rod
	createEffect(
	[p, dir](Effect* obj) mutable -> void {
		
		obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_void_rod);
		
		// U D L R
		// R U L D
		
		constexpr int dirConverter[4] = {1, 3, 2, 0};
		
		obj->graphicsIndex = dirConverter[static_cast<int>(dir)];
		
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_void_rod,
			obj->graphicsIndex
		);
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		
		obj->sprite.updateRawPosition(obj->x, obj->y);
		//obj->sprite.spritePointer.set_z_order(-2);
	},
	[](Effect* obj) mutable -> bool {
		obj->tempCounter++;
		if(obj->tempCounter == 12) {
			return true;
		}
		return false;
	}
	);
		
		
}

void EffectsManager::wings(Pos p, Direction dir) {
	
	// spr_void_wings for gray and lily 
	// spr_void_wings_dissipate for gray and lily (how exactly does this layer work?)
	
	// spr_void_wings_cif for cif 
	// spr_void_wings_dissipate_cif for cif
	
	// is precomputing this right here,, ok
	// ima just decide the sprite item in-func 
	// why isnt there a fucking func that lets me just change the graphics index without having to also lable the sprite item??
	
	// this code is fucking shit.
	
	bn::sound_items::snd_wingspawn.play();
	

	auto goofy = [](Effect* obj, Pos goofyP, Direction goofyDir, int offset) mutable -> void {
	
		obj->x = goofyP.x * 16;
		obj->y = goofyP.y * 16;
		if(goofyDir == Direction::Right) {
			obj->sprite.spritePointer.set_horizontal_flip(true);
			obj->x -= 8;
		} else if(goofyDir == Direction::Left) {
			obj->x += 8;
		}
	
		obj->x += offset;
		
		if(offset < 0) {
			obj->sprite.spritePointer.set_horizontal_flip(true);
		}
			
		if(goofyDir == Direction::Up || goofyDir == Direction::Down) {
			obj->y -= 4;
		}
		
		obj->sprite.spritePointer.set_tiles(
			globalGame->mode == 2 ? bn::sprite_tiles_items::dw_spr_void_wings_cif : bn::sprite_tiles_items::dw_spr_void_wings,
			0
		);
	
		obj->sprite.updateRawPosition(obj->x, obj->y);
		// dont set sprite prio when down bc, behind
		if(goofyDir == Direction::Down) {
			obj->sprite.spritePointer.set_z_order(0);
		} else {
			obj->sprite.spritePointer.set_z_order(-1);
		}
	};
	
	auto func2 = [](Effect* obj) mutable -> bool {
		obj->graphicsIndex++;
		if(globalGame->entityManager.player->wingsUse == 0) {
			return true;
		}
		obj->sprite.spritePointer.set_tiles(
			globalGame->mode == 2 ? bn::sprite_tiles_items::dw_spr_void_wings_cif : bn::sprite_tiles_items::dw_spr_void_wings,
			obj->graphicsIndex % 3
		);
		return false;
	};
	
	auto funcLeftRight = [p, dir, goofy](Effect* obj) mutable -> void {	
		goofy(obj, p, dir, 0);
	};
	
	auto funcUpDown1 = [p, dir, goofy](Effect* obj) mutable -> void {	
		goofy(obj, p, dir, -8);
	};
	
	auto funcUpDown2 = [p, dir, goofy](Effect* obj) mutable -> void {	
		goofy(obj, p, dir, 8);
	};
	
	if(dir == Direction::Left || dir == Direction::Right) {
		createEffect(funcLeftRight, func2);
	} else {
		createEffect(funcUpDown1, func2);
		createEffect(funcUpDown2, func2);
	}
}


