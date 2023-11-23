

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
	
	// this whole class desperately needs a rewrite
	
	//BN_ASSERT(tiles->tiles_ref().size() % (4 * width * height) == 0, "a bigsprite had a weird amount of tiles");
	
	optionCount = tiles->tiles_ref().size() / (4 * width * height);
	
	

	//draw(0);
	//firstDraw();
	
	// i dont like this code, but tbh im not sure of a better way around this.
	// i also really wish i could use a switch statement here
	// the decision of if to put the vars in the effectholder struct, or hard coded in here is difficult
	// gods i REALLY dont like this code 
	//bool doFirstDraw = true;
	// i now absolutely despise this code. 
	// the way that a chest(32 x 16 sprite) was exported somehow (curse)ed everything up here
	
	if(tiles == &bn::sprite_tiles_items::dw_spr_tail_boobytrap) {
		loadBoobTrap();
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_tail_upperbody) {
		loadTailHead();
	}
	
	if(width > 4 || height > 4) {
		
		xPos -= (8 * width);
		yPos -= (8 * height);
		
		isBigSprite = true;
		firstDraw();
		return;
	}

	
	int v;
	v = width * 16;
	v-=1;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v+=1;
			
	int newWidth = v;
	
	v = height * 16;
	v-=1;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v+=1;
	
	int newHeight = v;
	
	// this is a stupid bandaid solution.
	if(tiles->tiles_count_per_graphic() == 64) {
		newWidth = 64;
		newHeight = 64;
	}
	
	auto spriteShape = bn::sprite_shape_size(newWidth, newHeight);
	
	BN_LOG(newWidth, " ", newHeight);
	
	//BN_LOG(tiles->tiles_count(), " ", spriteShape.tiles_count(bn::bpp_mode::BPP_4));
	
	Sprite tempSprite(*tiles, spriteShape);
	//yPos -= ((8 * (4 - height)) + 8);
	yPos -= 8; // THIS YPOS IS TO OFFSET THE BORDER AROUND THE SCREEN
	tempSprite.updateRawPosition(xPos, yPos);
	
	sprites.push_back(tempSprite);
	
	if(tiles == &bn::sprite_tiles_items::dw_spr_chest) {
		loadChest();
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_birch) {
		loadTree();
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_gor) {
		loadGorHead();
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_gor_hair) {
		animationIndex = 1;
		animate();
	} else if(tiles == &bn::sprite_tiles_items::dw_spr_stinklines) {
		loadStink();
	}
	
}

void BigSprite::draw(int index) {
	
	BN_ASSERT(sprites.size() == 1, "tried to call draw on a bigsprite which was meant to actually just be one sprite, but it wasnt just one sprite. lol");
	
	BN_ASSERT(index < tiles->graphics_count(), "bigsprite tried drawing outside its valid graphics counts");
	
	animationIndex = index;
	
	sprites[0].spritePointer.set_tiles(
		*tiles,
		animationIndex
	);

	
}

void BigSprite::bigDraw(int index) { profileFunction();
	
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
			
			BN_ASSERT(spriteIndex < sprites.size(), "tried drawing a bigsprite sprite when that sprite was out of bounds");
			
			
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
	
	//int indexOffset = animationIndex * 4 * width * height;
	
	for(int y=0; y< height; y++) {
		for(int x=0; x<width; x++) {
						
			int spriteXPos = xPos + x * 16;
			int spriteYPos = yPos + y * 16;
			
			// check if this sprite will even be drawn onscreen.
			// check occurs first as it is inexpensive
			// a bit generious, just in case 
			if(spriteXPos < -16 || spriteXPos > 240+16 || spriteYPos < -16 || spriteYPos > 160+16) {
				continue;
			}
			
			
			/*
			if(!autoAnimate) {
				// dont do the sprite if this tile is blank. should really help with not hitting the sprite limit
				// is this 4, or ithis 8???
				// omfg (curse), its 4 for the number of subtiles, idiot
				for(int j=0; j<4; j++) {
					// quite goofy, but basically the set_tiles func like, does the 16x16 tile math, but we need to do it manually here.
					int offset = 4 * (x + (y * width)) + j + indexOffset;
					
					BN_ASSERT(offset < tiles->tiles_ref().size(), "bigsprite tried to load a invalid tile. max tileindex = ", tiles->tiles_ref().size(), " tried loading at ", offset);
					
					// is this pointer like,,, what tf
					const uint32_t* tileRef = (tiles->tiles_ref())[offset].data;
					
					for(int i=0; i<8; i++) {
						if(tileRef[i] != 0) {
							goto doBigTile; // (curse) code
						}
					}
				}
				
				// is continueing here ideal? the collision map wont update for blank tiles. is that ok?
				continue;
			
			}
			doBigTile:
			*/
			
			Sprite tempSprite = Sprite(*tiles);
			
			tempSprite.updateRawPosition(spriteXPos, spriteYPos);
			/*
			int tempX = xPos/16 + x;
			int tempY = yPos/16 + y;
			
			// this array doesnt need to be updated each time we do an update draw omg
			
			if(collide) {
				game->collisionMap[tempX][tempY] = 2;
			}
			*/
			
			tempSprite.spritePointer.set_bg_priority(priority);
			
			tempSprite.spritePointer.set_tiles(
				*tiles,
				x + (y * width) + (animationIndex * width * height)
			);
			
			//BN_LOG("attempting push");
			sprites.push_back(tempSprite);
			//BN_LOG("push success");
		}
	}
}

void BigSprite::updatePalette(Palette* pal) {
	for(int i=0; i<sprites.size(); i++) {
		sprites[i].spritePointer.set_palette(pal->getSpritePalette());
	}
}

void BigSprite::animate() { profileFunction();
	
	if(isBigSprite) {
		// icould maybe make this branchless by having animate be a default, but im tired ok
		if(customAnimate == NULL) {
			animationIndex = (animationIndex + 1) % optionCount;
		} else {
			animationIndex = customAnimate();
		}
		bigDraw(animationIndex);
	} else {
		
		animationIndex = (animationIndex + 1) % tiles->graphics_count();
		
		draw(animationIndex);
	}
}

// -----

void BigSprite::loadBoobTrap() {

	BN_LOG("booba detected");
		
	auto func1 = [](void* obj) -> void {
		
		BigSprite* bigSprite = static_cast<BigSprite*>(obj);
		
		static unsigned boobaBackup = 0;
		
		if(boobaCount != boobaBackup) {
			boobaBackup = boobaCount;
			
			bigSprite->effectsManager->doDialogue(
			"please dont touch me\rin that manner.\ryou'll regret it."
			);
			
			return;
		}
		
		// wtf. irlly need to switch over to namespaces
		if(bigSprite->effectsManager->entityManager->hasObstacle(Pos(12, 5))) {
			bigSprite->effectsManager->doDialogue(
			"i,,, why did you even do that?\rprobably wanted to see if i had programed it in\n"
			"well, i did\rlet me move that out of the way\n"
			"it might hurt a little bit though"
			);
			
			// it would be quite funny to somehow have her boobs kill you
			
			bigSprite->effectsManager->entityKill(globalGame->entityManager.player);
			bigSprite->effectsManager->entityManager->addKill(*(bigSprite->effectsManager->entityManager->getMap(Pos(12, 5)).begin()));
			return;
		}
	
		
		static unsigned msgIndex = 0;
		
		constexpr MessageStr msgs[] = {
			{"you wouldnt believe how much the idiot who made this remake spent on animating"
		" my boobs, head, and tail.\r"
		"I would say I appreciate it, but to be honest, its just borderlining on creepy now.\n"
		"like, jesus christ. you dont even have music, or shortcuts working\r"
		"but now you have boobs????\rwhy????????\r"
		"actually wait a minute, what is that jam in the background? i guess music is somehow in!"
		"\0"},
		{"anyway, you know the drill\rhead down the stairs, good luck\0"}
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
		
		if(boobaCount > 16 && randomGenerator.get_int(0, 256 - boobaCount) == 0) {
			bn::sound_items::metal_pipe_falling_sound_effect.play();
			int lmao = 0;
			while(lmao < 60 * 5) {
				lmao++;
				game->doButanoUpdate();
			}
			BN_ERROR("excessive, overflow, booba\nto much booba\ntouch grass. or maybe take some estrogen and\nget your own.\nBooba Error Code: ", boobaCount);
		}
		
		static int timesCalled = 0;
		
		if(timesCalled == 0) {
			game->playSound(&bn::sound_items::snd_bounce);
			game->removeSound(&bn::sound_items::snd_push_small);
		}
		
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
	
	// yes, these are the same object but with a slightly different pos, and yes, i am to scared to (curse)ing copy them
	Interactable* temp2 = new Interactable(Pos(10, 3),
		func1,
		func2,
		NULL,
		(void*)this
	);
	
	entityManager->addEntity(temp1);
	entityManager->addEntity(temp2);
	
	for(int i=3 ; i<=8; i++) {
		
		Interactable* temp = new Interactable(Pos(i, 4),
			[](void* obj) -> void { (void)obj; return; },
			[](void* obj) -> bool { 
				(void)obj; 
				
				static int tailSpeedFrames = 0;
				if(tailSpeedFrames == 0) {
					globalGame->playSound(&bn::sound_items::snd_bounceimpact);
				}
				
				// tails tail should speed up here. i have no easy method of getting a handler for it.
				// ya know what?? for loop. i dont give a HECK  anymore. 
				// at least making it static like,, yea
				// altho, if you reset/change rooms in the middle of hitting tail,,,,,,,,,
				// could i do something like i = int(1) in the captures???
				tailSpeedFrames++;
				
				bn::vector<BigSprite*, 128>& allBigSprites = globalGame->effectsManager.bigSprites;
				
				static BigSprite* tailBigSprite = NULL;
				
				if(tailBigSprite == NULL) {
					for(auto it = allBigSprites.begin(); it != allBigSprites.end(); ++it) {
						if( (*it)->tiles == &bn::sprite_tiles_items::dw_spr_tail_tail ) {
							tailBigSprite = *it;
							break;
						}
					}
					
					tailBigSprite->autoAnimateFrames = 20;
					
					BN_ASSERT(tailBigSprite != NULL, "couldnt find tails tail sprite ptr???");
				}	
				
				if(tailSpeedFrames % 5 == 0) {
					tailBigSprite->animate();
				}
	
				
				if(tailSpeedFrames == 60) {
					tailBigSprite = NULL;
					tailSpeedFrames = 0;
					return true;
				}
				
				
				return false; 
			},
			NULL,
			NULL
		);
		entityManager->addEntity(temp);
	}
	
	BN_ASSERT(tileManager->floorMap[9][4] == NULL, "bigsprite tried adding a tile when there was one already there??");
	BN_ASSERT(tileManager->floorMap[10][4] == NULL, "bigsprite tried adding a tile when there was one already there??");
	
	tileManager->floorMap[9][4] = new FloorTile(Pos(9,4));
	tileManager->floorMap[10][4] = new FloorTile(Pos(10,4));
}

void BigSprite::loadTailHead() {
	
	animationIndex = 2;
	autoAnimateFrames = 4;
	
	customAnimate = []() -> int {
		
		// this was a switch statement until i became a conspiracy theorist.
		BN_ASSERT(globalGame->entityManager.player != NULL, "WHAT THE (curse)");
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
}

void BigSprite::loadChest() {
	
	//BN_LOG("hey dumb(curse), load a chest");
	
	// this thing quite literaly:
	// isnt a bigsprite
	// isnt animated 
	// what the (curse) am i on 
	
	// and now that im going back to previously written code, and using it for completely unintended purposes, i am reminded that i am a trash programmer
	// i hijacked the KICK function to do this random bs????
	
	//sprites.push_back(Sprite(bn::sprite_tiles_items::dw_spr_chest, bn::sprite_tiles_items::dw_spr_chest_shape_size));
	
	bool isSuperRodChest = game->roomManager.roomIndex == 0;
	
	if(isSuperRodChest) {
		sprites[0].spritePointer.set_tiles(*tiles, entityManager->player->hasSuperRod);
	} else {
		sprites[0].spritePointer.set_tiles(*tiles, entityManager->player->hasRod | entityManager->player->hasSuperRod);
	}

	auto func1 = [isSuperRodChest](void* obj) -> void {
		
		
		if(entityManager->player->hasSuperRod) {
			return;
		} else if(!isSuperRodChest && globalGame->entityManager.player->hasRod) {
			return;
		}
		
		BigSprite* bigSprite = static_cast<BigSprite*>(obj);
		
		Pos testChestPos( (bigSprite->xPos) / 16, (bigSprite->yPos) / 16 );
		
		if(entityManager->player->p.y != testChestPos.y + 1) {
			return;
		}
		
		
		
		globalGame->cutsceneManager.introCutscene();
	
		
		bigSprite->sprites[0].spritePointer.set_tiles(*(bigSprite->tiles), 1);
	
		
		globalGame->entityManager.player->hasRod = !isSuperRodChest;
		globalGame->entityManager.player->hasSuperRod = isSuperRodChest;
		
		globalGame->tileManager.updateRod();
		globalGame->tileManager.floorLayer.reloadCells();
		
		return;
	};
	auto func2 = [](void* obj) -> bool {
		(void)obj;
		return true;
	};
	
	int yIndex = isSuperRodChest ? 3 : 4;
	
	Interactable* temp1 = new Interactable(Pos(6, yIndex),
		func1,
		func2,
		(void*)this,
		(void*)this
	);
	
	// yes, these are the same object but with a slightly different pos, and yes, i am to scared to (curse)ing copy them
	Interactable* temp2 = new Interactable(Pos(7, yIndex),
		func1,
		func2,
		(void*)this,
		(void*)this
	);
	
	entityManager->addEntity(temp1);
	entityManager->addEntity(temp2);
	
}

void BigSprite::loadTree() {
	
	switch(game->mode) {
		default:
		case 0:
			tiles = &bn::sprite_tiles_items::dw_spr_birch;
			break;
		case 1:
			tiles = &bn::sprite_tiles_items::dw_spr_birch;
			break;
		case 2:
			tiles = &bn::sprite_tiles_items::dw_spr_birch_b;
			if(strcmp(game->roomManager.currentRoomName(), "rm_rest_area_9\0") == 0) {
				animationIndex = 0;
			} else {
				animationIndex = 1;
			}
			break;
	}
	
	sprites[0].spritePointer.set_tiles(*tiles, animationIndex);
	sprites[0].spritePointer.set_bg_priority(1);
	sprites[0].spritePointer.set_z_order(-2);

	
	auto getInteractFunc = []() -> auto {
		
		bool alreadyTalked = false;
		
		return [alreadyTalked](void* unused) mutable -> void {
			(void)unused;
			
			// hehe, have me give them an egg if behind the tree
			
			
			
			auto eggFunc = [&alreadyTalked]() mutable -> void {

			
				// my use of lambdas has become excessive 
				auto giveEgg = []() -> void {
					
					bn::sound_items::egg.play();
				
					unsigned eggBackup = globalGame->saveData.eggCount;
					globalGame->saveData.eggCount++;
				
					if(eggBackup == 0) {
						globalGame->effectsManager.doDialogue("\0* You got the egg\0");
					} else {
						
						
						//char buffer[64];
						//memset(buffer, '\0', 64);
						//\0* You got another egg\0"
						// i miss snprintf
						//https://gvaliente.github.io/butano/faq.html#faq_sprintf
						bn::string<64> string;
						bn::ostringstream string_stream(string);
						// i dont think string stream likes me having this null char thestart
						// nvm c_str instead of data worked
						// nope???? wtf???
						string_stream << "* You got another egg. You now have: ";
						string_stream << globalGame->saveData.eggCount;
						string_stream << " eggs\0";
						
						char buffer[64];
						memset(buffer, '\0', 64); 
						strncpy(buffer + 1, string.c_str(), string.size());
			
						globalGame->effectsManager.doDialogue(buffer);
					}
				};
			
				if(alreadyTalked) {
					globalGame->effectsManager.doDialogue("* The fool is happily working away.\nBetter leave them to it\0");
					return;
				}	

			
				alreadyTalked = true;
				
				const char* eggMsg1 = ""
				"* Well, there is a fool here.\n"
				"* They would be happy to see anyone, including you.\n"
				"* What do you think?"
				"\0";
				
				const char* eggMsg2 = ""
				"* Well, there is a fool here.\n"
				"* They might be happy to see you.\n"
				"* What do you think?"
				"\0";
				
				const char* eggMsg3 = ""
				"\0* Well, there is someone here.\n"
				"* They are happy to see you.\n"
				"* You are as well."
				"\0";
				
				bool restRes = false;
				
				if(globalGame->saveData.eggCount == 0) {
					globalGame->effectsManager.doDialogue(eggMsg1);
					restRes = globalGame->effectsManager.restRequest("What do you think?\0");
				} else if(globalGame->saveData.eggCount < 5) {
					globalGame->effectsManager.doDialogue(eggMsg2);
					restRes = globalGame->effectsManager.restRequest("What do you think?\0");
				} else {
					globalGame->effectsManager.doDialogue(eggMsg3);
					restRes = true;
				}

				if(restRes) {
					
					// ohhh gods i need bigsprites(like sprites of the ppl) to actually make this make sense.
					// spr_lillie_port_eh
					// or i could just have a lil symbol for me.
					// that might be good
					// and good LORD, i would rather write a whole new class than interact with bigsprite ever again 
					// ill just have them be effects tbh
					
					Sprite idrk(bn::sprite_items::dw_idrk);
					idrk.setVisible(false);
					idrk.updatePosition(Pos(11, 4));
					
					if(globalGame->saveData.eggCount == 0) {
					
						globalGame->effectsManager.doDialogue("\0* The fool slowly turns around and stares at you\0");
						
						idrk.setVisible(true);
						globalGame->effectsManager.doDialogue("\0* Thank you for saying hello.\0", &bn::sound_items::snd_cifer);
						idrk.setVisible(false);
						
						globalGame->effectsManager.doDialogue("\0* After awkwardly fumbling through their pockets, they force a gift into your hands\0");
					
						giveEgg();
						
						idrk.setVisible(true);
						globalGame->effectsManager.doDialogue("\0* It reminds me of you. Please take good care of it.\0", &bn::sound_items::snd_cifer);
						idrk.setVisible(false);
						globalGame->effectsManager.doDialogue("\0* The fool returns to their computer, just as they were before.\0");
						
						globalGame->effectsManager.doDialogue("* What even was that?\0");
						
					} else if(globalGame->saveData.eggCount < 5) {
						
						globalGame->effectsManager.doDialogue("\0* The fool slowly turns around and stares at you\0");
						
						idrk.setVisible(true);
						globalGame->effectsManager.doDialogue("* Hello again. Did you enjoy the egg?\0", &bn::sound_items::snd_cifer);
						idrk.setVisible(false);
						
						restRes = globalGame->effectsManager.restRequest("Egg?\0");
						
						if(restRes) {
							giveEgg();
							idrk.setVisible(true);
							globalGame->effectsManager.doDialogue("\0* Great! I thought you would.\n* Please come back for more later if you'd like.\0", &bn::sound_items::snd_cifer);
							idrk.setVisible(false);
							globalGame->effectsManager.doDialogue("* Thank you for spending time with me. It means the world.\0", &bn::sound_items::snd_cifer);
						} else {
							idrk.setVisible(true);
							globalGame->effectsManager.doDialogue("\0* Thats,`,`,`ok.\n* They arent for everyone.\n* I understand that.\0", &bn::sound_items::snd_cifer);
							idrk.setVisible(false);
							globalGame->effectsManager.doDialogue("* They seem dejected, and return to their work\0");
						}
					} else {
						alreadyTalked = false;
						
						
						globalGame->effectsManager.doDialogue("\0* Your friend eagerly gets up from their bed and turns to you\0");
						
						idrk.setVisible(true);
						globalGame->effectsManager.doDialogue("\0* Thank you for taking such good care of my eggs. I finally am able to rest.\n* As payment, would you like some breakfast?\0", &bn::sound_items::snd_cifer);
						idrk.setVisible(false);

						globalGame->effectsManager.doDialogue("\0* They offer you some scrambled eggs.\n"
						"* You eat them without a second thought.\n"
						"* The taste reminds you of home.\0");
						
						idrk.setVisible(true);
						globalGame->effectsManager.doDialogue("\0"
						"* Again, thank you for spending time with me, and helping me with my eggs.\n"
						"* It means the world, to me.\n"
						"* Thank you. <3\0"
						, &bn::sound_items::snd_cifer);
						idrk.setVisible(false);
						
						globalGame->effectsManager.doDialogue("\0"
						"* Your friend lies back down, clean and well-rested\n"
						"* You feel proud.\0");
						
						idrk.setVisible(true);
						globalGame->effectsManager.doDialogue("\0"
						"* Please come back any time if you want more eggs!\n"
						"* Or,`to spend time with me.`I would appreciate it.\0"
						, &bn::sound_items::snd_cifer);
						idrk.setVisible(false);
						
					}

					// i understand now why dialogue systems exist.
					// this is going to be the most complex convo in the whole game lmao
					// have them take a bath and take a break after 5 eggs 
					// gods, i should take a break
					//globalGame->saveData.eggCount++;

				} else {
					globalGame->effectsManager.doDialogue("* You were better off leaving it alone.\0");
				}
			};
			
			auto inProgressFunc = []() -> void {
				
				
				const char* inProgressMsg = "[The tree seems to have something to show you, but cant yet.]\n"
				//"[Behind the tree, somene sits hunched over a keyboard, mumbling to themself, blissfully unaware of your presence.]\n"
				"[Behind the tree, someone sits with a slight hunch over a computer, quietly murmuring to themselves, completely unaware of your presence.]\n"
				"[Their hair is down to their knees.]\n"
				"[It seems like they are busy with their work, and haven't had a break in weeks.]\n"
				"[They could definitely use a good bath.]\n"
				"[Better move on]\0";
				
				const char* inProgressMsg2 = "[The tree seems to have something to show you, but cant yet.]\n"
				//"[Behind the tree, somene sits hunched over a keyboard, mumbling to themself, blissfully unaware of your presence.]\n"
				"[Behind the tree, someone sits with a slight hunch over a computer, quietly murmuring to themselves, completely unaware of your presence.]\n"
				"[Their hair is down to their knees.]\n"
				"[You would recognize that disheveled look anywhere. They are the egg bringer, arbiter of breakfast!]\r[Or maybe the eggs are about something else?]\n"
				"[They still havent taken a desperately needed bath.]\n"
				"[Maybe go say hello?]\0";
				
				const char* inProgressMsg3 = "[The tree seems to have something to show you, but cant yet.]\n"
				//"[Behind the tree, somene sits hunched over a keyboard, mumbling to themself, blissfully unaware of your presence.]\n"
				"[Behind the tree, someone is laying down on a bed, completely unaware of your presence.]\n"
				"[Their hair is down to their knees, and shines in the glow of the tree.]\n"
				"[It is your friend.]\n"
				"[And it seems they have finally taken a bath.]\n"
				"[Maybe go say hello?]\0";
				
				if(globalGame->saveData.eggCount == 0) {
					globalGame->effectsManager.doDialogue(inProgressMsg);
				} else if(globalGame->saveData.eggCount < 5) {
					globalGame->effectsManager.doDialogue(inProgressMsg2);
				} else {
					globalGame->effectsManager.doDialogue(inProgressMsg3);
				}
			};
			
			Pos playerPos = globalGame->entityManager.player->p;
			
			if(playerPos == Pos(6, 3)) {
				eggFunc();
			} else {
				switch(game->mode) {
					default:
					case 0:
						inProgressFunc();
						break;
					case 1:
						inProgressFunc();
						break;
					case 2:
						if(strcmp(game->roomManager.currentRoomName(), "rm_rest_area_9\0") == 0) {
							globalGame->cutsceneManager.cifDream();
						} else {
							globalGame->effectsManager.doDialogue("[This Lotus-Eater Machine doesn't seem to be operational]\n[Better move on]\0");
						}
						break;
				}
			}
		};
	};
	
	unsigned startFrame = frame+1;
	auto kickedFunc = [startFrame](void* bigSprite_) mutable -> bool {
		
		// i rlly still,, dislike using kickedfunc for anims. but,, it works, and gods do i not 
		// want to go back and rewrite this code
		// now that i think about it, am i,, basically having the kick overwrite bigsprites 
		// animation code????? i think i am?? 
		// wtf
		// and i dont have any of the counters which made this easy in effects, ugh
		// this is literally a jank effect written into a kick. 
		// could i,,,, maybe have it launch an effect?
		// tbh that might be better, except i could/would have multiple 
		// shakes going on at once. 
		// of course, i know my way around that.
		// ugh 
		
		BigSprite* bigSprite = static_cast<BigSprite*>(bigSprite_);
		
		static bool firstRun = true;
		static const bn::sprite_tiles_item* restoreTiles = NULL;
		static int restoreAnimationIndex = 0;
		
		// very weirdly, the padding on the shook and non shook trees are different 
		// im debating like,, should i not use padding for all bigsprites?
		// ugh 
		
		if(firstRun) {
			
			firstRun = false;
			
			game->playSound(&bn::sound_items::snd_push);
			game->removeSound(&bn::sound_items::snd_push_small);
			
			restoreAnimationIndex = bigSprite->animationIndex;
			restoreTiles = bigSprite->tiles;
			bigSprite->animationIndex = 0;
			
			if(restoreAnimationIndex == 0) {
				globalGame->effectsManager.treeLeaves();
				if(restoreTiles == &bn::sprite_tiles_items::dw_spr_birch) {
					bigSprite->tiles = &bn::sprite_tiles_items::dw_spr_birch_shake;
				} else {
					bigSprite->tiles = &bn::sprite_tiles_items::dw_spr_birch_shake_b;
				}
			} else { // bare tree
				if(restoreTiles == &bn::sprite_tiles_items::dw_spr_birch) {
					bigSprite->tiles = &bn::sprite_tiles_items::dw_spr_birchbare_shake;
				} else {
					bigSprite->tiles = &bn::sprite_tiles_items::dw_spr_birchbare_shake_b;
				}
			}
			
			bigSprite->sprites[0].spritePointer.set_x(bigSprite->sprites[0].spritePointer.x()-1);
			
			bigSprite->sprites[0].spritePointer.set_tiles(
				*bigSprite->tiles,
				bigSprite->animationIndex % 4
			);	
			
		}
		
		if((frame - startFrame) % 6 != 0) {
			return false;
		}
		bigSprite->animationIndex++;
		bigSprite->sprites[0].spritePointer.set_tiles(
			*bigSprite->tiles,
			bigSprite->animationIndex % 4
		);
		
		
		
		if(bigSprite->animationIndex == 6) {
			
			firstRun = true;
			
			bigSprite->animationIndex = restoreAnimationIndex;
			bigSprite->tiles = restoreTiles;
			restoreTiles = NULL;
			
			bigSprite->sprites[0].spritePointer.set_x(bigSprite->sprites[0].spritePointer.x()+1);
			
			bigSprite->sprites[0].spritePointer.set_tiles(
				*bigSprite->tiles,
				bigSprite->animationIndex
			);
			
			
			
			return true;
			
		}
		
		
		
		return false;
	};
	
	// todo, we gotta add shake here
	// shake also spawns leaves! do that too

	Interactable* temp1 = new Interactable(Pos(6, 4),
		getInteractFunc(),
		kickedFunc,
		NULL,
		this
	);
	
	entityManager->addEntity(temp1);
	
}

void BigSprite::loadGorHead() {
	
	
	/*
	Many college students have gone to college
	And gotten hooked on drugs, marijuana, and alcohol
	Listen, stop trying to be somebody else
	Don't try to be someone else
	Be yourself and know that that's good enough
	Don't try to be someone else
	Don't try to be like someone else
	Don't try to act like someone else, be yourself
	Be secure with yourself
	Rely and trust upon your own decisions
	On your own beliefs
	You understand the things that I've taught you
	Not to drink alcohol, not to use drugs
	Don't use that cocaine or marijuana
	Because that stuff is highly addictive
	When people become weed-heads
	They become sluggish, lazy, stupid and unconcerned
	Sluggish, lazy, stupid and unconcerned
	That's all marijuana does to you, okay?
	This is mom
	Unless you're taking it under doctor's, um, control
	Then it's regulated
	Do not smoke marijuana, do not consume alcohol
	Do not get in the car with someone who is inebriated
	This is mom, call me, bye
	*/
	
	auto wingInteractFunc = [](void* obj) -> void {
		(void)obj;
	};
	
	auto wingKickFunc = [](void* obj) -> bool {
		(void)obj;
		return true;
	};

	int talkCount = 0;
	auto gorInteractFunc = [talkCount](void* obj) mutable -> void {
		(void)obj;
		
		
		BigSprite* bigSprite = static_cast<BigSprite*>(obj);
		
		//bigSprite->sprites[0].spritePointer.tiles()
		
		if(bigSprite->tiles == &bn::sprite_tiles_items::dw_spr_gor_sleep) {
			globalGame->effectsManager.doDialogue("[A giant head lies before you, blocking your path]\0");
			return;
		}
		
		
		// give locust if they dont have any
		if(globalGame->entityManager.player->locustCount == 0) {
			globalGame->effectsManager.doDialogue(""
			"damnnnnnnnnnnn\n"
			"you are flat broke\n"
			"i got some spare change lying around, here\n"
			"now, go yeet yourself at the floor of that statue to the right, and have fun\n"
			"\0", &bn::sound_items::snd_gor);
			
			globalGame->entityManager.player->locustCount = 1;
			globalGame->tileManager.updateLocust();
			globalGame->tileManager.floorLayer.reloadCells();
			
			return;
		}
		
		if(talkCount == 0) {
		
			globalGame->effectsManager.doDialogue("\0Many college students have gone to college\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0And gotten hooked on drugs, marijuana, and alcohol\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Listen, stop trying to be somebody else\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Don't try to be someone else\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Be yourself and know that that's good enough\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Don't try to be someone else\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Don't try to be like someone else\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Don't try to act like someone else, be yourself\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Be secure with yourself\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Rely and trust upon your own decisions\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0On your own beliefs\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0You understand the things that I've taught you\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Not to drink alcohol, not to use drugs\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Don't use that cocaine or marijuana\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Because that stuff is highly addictive\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0When people become weed-heads\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0They become sluggish, lazy, stupid and unconcerned\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Sluggish, lazy, stupid and unconcerned\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0That's all marijuana does to you, okay?\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0This is mom\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Unless you're taking it under doctor's, um, control\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Then it's regulated\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Do not smoke marijuana, do not consume alcohol\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0Do not get in the car with someone who is inebriated\0", &bn::sound_items::snd_gor);
			globalGame->effectsManager.doDialogue("\0This is mom, call me, bye\0", &bn::sound_items::snd_gor);
		
		} else {
			globalGame->effectsManager.doDialogue("Thank you for listening to me recite the lyrics to Frank Ocean's Song: Be Yourself.\0", &bn::sound_items::snd_gor);
		}
			
		talkCount++;
		
		
	};
	
	auto gorKickFunc = [](void* obj) -> bool {
		
		BigSprite* bigSprite = static_cast<BigSprite*>(obj);
		
		static int frameCount = 0;
		
		if(frameCount == 0) {
			bn::sound_items::snd_bounceimpact.play();
		}
		
		frameCount++;
		
		if(frameCount > 10) {
			frameCount = 0;
			bigSprite->sprites[0].spritePointer.set_tiles(bn::sprite_tiles_items::dw_spr_gor);
			bigSprite->tiles = &bn::sprite_tiles_items::dw_spr_gor;
			globalGame->effectsManager.bigSprites[1]->animationIndex = 1;
			globalGame->effectsManager.bigSprites[1]->animate();
			return true;
		}
		
		bigSprite->sprites[0].spritePointer.set_tiles(bn::sprite_tiles_items::dw_spr_gor_hit);
		bigSprite->tiles = &bn::sprite_tiles_items::dw_spr_gor_hit;
		
		// this is jank and stupid
		globalGame->effectsManager.bigSprites[1]->animationIndex = 0;
		globalGame->effectsManager.bigSprites[1]->animate();
		
		return false;
	};
	
	const Pos wingInteractablePoses[] = {
		Pos(9, 5),
		Pos(8, 5),
		Pos(9, 4),
		Pos(8, 4),
		
		Pos(3, 5),
		Pos(4, 5),
		Pos(3, 4),
		Pos(4, 4),

	};
	
	for(unsigned i=0; i<sizeof(wingInteractablePoses)/sizeof(wingInteractablePoses[0]); i++) {
		Interactable* temp = new Interactable(wingInteractablePoses[i],
			wingInteractFunc,
			wingKickFunc,
			(void*)this,
			(void*)this
		);
		entityManager->addEntity(temp);
	}
	
	for(unsigned i=0; i<3; i++) {
		Interactable* temp = new Interactable(Pos(5 + i, 4),
			gorInteractFunc,
			gorKickFunc,
			(void*)this,
			(void*)this
		);
		entityManager->addEntity(temp);
	}	
	
	
	sprites[0].spritePointer.set_tiles(bn::sprite_tiles_items::dw_spr_gor_sleep);
	tiles = &bn::sprite_tiles_items::dw_spr_gor_sleep;
}

void BigSprite::loadStink() {
	autoAnimate = true;
	xPos-=7;
	yPos+=1;
	autoAnimateFrames = 8;
	sprites[0].updateRawPosition(xPos, yPos);
	sprites[0].spritePointer.set_bg_priority(3);
}

// -----
	
EffectsManager::EffectsManager(Game* game_) : game(game_), textGenerator(dw_fnt_text_12_sprite_font), verTextGenerator(common::variable_8x8_sprite_font),
	effectsLayer(bn::regular_bg_tiles_items::dw_default_bg_tiles)
	{
		
		
	
		
	// may not be the best idea?
	textGenerator.set_one_sprite_per_character(true);
	
	// copy over effectstiles
	bn::optional<bn::span<bn::tile>> tileRefOpt = tilesPointer.vram();
	BN_ASSERT(tileRefOpt.has_value(), "wtf");
	bn::span<bn::tile> tileRef = tileRefOpt.value();
	
	
	// now that these tiles can be made in memory, we could/should also change the,,, like, whats in them, but thats for later
	for(int i=0; i<bn::regular_bg_tiles_items::dw_customeffecttiles.tiles_ref().size(); i++) {
		
		BN_ASSERT(i < tileRef.size(), "size error when loading in customeffecttiles");
		
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
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 0);
		}
	}
	
	effectsLayer.rawMap.bgPointer.set_tiles(tilesPointer);
	
	for(int i=0; i<8; i++) {
		tileRef[122].data[i] = 0x11111111;
		tileRef[123].data[i] = 0x11111111;
		tileRef[124].data[i] = 0x11111111;
		tileRef[125].data[i] = 0x11111111;
		
		tileRef[126].data[i] = 0x11111111;
		tileRef[127].data[i] = 0x11111111;
	}
		
	//for(int i=0+1; i<30-1; i++) {
	for(int i=0; i<30; i++) {
		effectsLayer.rawMap.setTile(i, 0, 127, false, true);
		effectsLayer.rawMap.setTile(i, 19, 127, true, false);
	}

	//for(int i=0+1; i<20-1; i++) {
	for(int i=0; i<20; i++) {
		effectsLayer.rawMap.setTile(0, i, 126, true, false);
		effectsLayer.rawMap.setTile(29, i, 126, false, true);
	}
		
	effectsLayer.reloadCells();
	
}

EffectsManager::~EffectsManager() {
	for(int i=0; i<effectList.size(); i++) {
		if(effectList[i] != NULL) {
			delete effectList[i];
		}
		effectList[i] = NULL;
	}
	
	effectList.clear();
	removeEffectsList.clear();
	
	for(int i=0; i<bigSprites.size(); i++) {
		if(bigSprites[i] != NULL) {
			delete bigSprites[i];
		}
		bigSprites[i] = NULL;
	}
	
	bigSprites.clear();
	
	if(dialogueEndPointer != NULL) {
		delete dialogueEndPointer;
		dialogueEndPointer = NULL;
	}
}

void EffectsManager::updatePalette(Palette* pal) {
	
	for(int i=0; i<effectList.size(); i++) {
		effectList[i]->sprite.spritePointer.set_palette(pal->getSpritePalette());
	}
	
	for(int i=0; i<textSpritesLine1.size(); i++) {
		textSpritesLine1[i].set_palette(pal->getSpritePalette());
	}
	
	for(int i=0; i<textSpritesLine2.size(); i++) {
		textSpritesLine2[i].set_palette(pal->getSpritePalette());
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

bool EffectsManager::zoomEffect(bool inward, bool autoSpeed) {
	
	static bool firstRun = true;
	static int layer = 0;
	if(firstRun) {
		firstRun = false;
		layer = inward ? 30 : 0;
	}
	
	//if(autoSpeed && frame % 5 != 0) {
	//if(autoSpeed && frame % 3 != 0) {
	if(autoSpeed && frame % 2 != 0) {
		return false;
	}
	
	Pos p = entityManager->player->p;
	
	int xPos = (p.x * 2) + 1;
	int yPos = (p.y * 2) + 1;
	
	int increment = inward ? -1 : 1;
	int stopIndex = inward ? -1 : 30;
	
	
	if(layer != stopIndex) {
		
		// loop until we willdraw at least one thing 
		while(xPos-layer < 0 && yPos-layer < 0 && xPos+layer >= 30 && yPos+layer >= 20) {
			layer += increment;
			if(layer == stopIndex) {
				// breaking my static vars at bottom rule.
				firstRun = true;
				return true;
			}
		}
		
		/*
		Pos topLeft = safePos(p.x-layer, p.y-layer);
		Pos bottomRight = safePos(p.x+layer, p.y+layer);
		
		for(int x = topLeft.x; x<=bottomRight.x; x++) {
			if(p.y-layer >= 0) {
				//effectsLayer.setBigTile(x, topLeft.y, inward);
				effectsLayer.setTile(x, topLeft.y, 4*inward);
			}
			if(p.y+layer < 20) {
				//effectsLayer.setBigTile(x, bottomRight.y, inward);
				effectsLayer.setTile(x, bottomRight.y, 4*inward);
			}
		}
		
	
		for(int y = topLeft.y; y<=bottomRight.y; y++) {
			if(p.x-layer >= 0) {
				//effectsLayer.setBigTile(topLeft.x, y, inward);
				effectsLayer.setTile(topLeft.x, y, 4*inward);
			}
			if(p.x+layer < 30) {
				//effectsLayer.setBigTile(bottomRight.x, y, inward);
				effectsLayer.setTile(bottomRight.x, y, 4*inward);
			}
		}
		*/
		
		int topLeftX = CLAMP(xPos - layer, -1, 30);
		int topLeftY = CLAMP(yPos - layer, -1, 30);
		int bottomRightX = CLAMP(xPos + layer, -1, 30);
		int bottomRightY = CLAMP(yPos + layer, -1, 30);
		
		
		for(int x = topLeftX; x<=bottomRightX; x++) {
			if(topLeftY >= 0 && x >= 0) {
				effectsLayer.setTile(x, topLeftY, 4*inward);
			}
			if(bottomRightY < 20 && x >= 0) {
				effectsLayer.setTile(x, bottomRightY, 4*inward);
			}
		}
		
	
		for(int y = topLeftY; y<=bottomRightY; y++) {
			if(topLeftX >= 0 && y >= 0) {
				effectsLayer.setTile(topLeftX, y, 4*inward);
			}
			if(bottomRightX < 30 && y >= 0) {
				effectsLayer.setTile(bottomRightX, y, 4*inward);
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
		//fillLevel+=4;
		fillLevel+=8;
		
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

void EffectsManager::setBorderColor(bool black) {
	
	bn::span<bn::tile> tileRef = tilesPointer.vram().value();
	
	//unsigned borderVal = game->roomManager.isWhiteRooms() ? 0x22222222 : 0x11111111;
	unsigned borderVal = black ? 0x11111111 : 0x22222222;
	
	for(int i=0; i<8; i++) {
		tileRef[122].data[i] = borderVal;
		tileRef[123].data[i] = borderVal;
		tileRef[124].data[i] = borderVal;
		tileRef[125].data[i] = borderVal;
		tileRef[126].data[i] = borderVal;
		tileRef[127].data[i] = borderVal;
	}

}

bool EffectsManager::exitRoom() {
	
	// if we have a locusts death, flash to white 
	// if voided death, do a black from top down 
	// if we exit successfully, do the box zoom on the exit.
	
	// wait a lil before actually starting our anims here
	// i rlly wish all these static vars were more elegant, but im basically 
	// trying to be safe by being sure that the only place where true returns 
	// is the end of the func, right after we reset those vars, or something close to that
	
	doVBlank();
	
	BN_ASSERT(removeEffectsList.size() == 0, "removeEffectsList size was nonzero after a effectsmanager vblank. this should never happen!");
	
	
	static bool firstRun = true;
	static unsigned int firstFrame = 0;
	if(firstRun) {
		firstRun = false;
		firstFrame = frame;
	
		return false;
	}
	
	// i rlly need a method of lilke,, when to do this(as in how long to wait)
	if(frame - firstFrame < 45 * 1) {
		return false;
	}
	
	//setBorderColor();
	
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
	
	
	if(frame - firstFrame < 15 * 1) {
		return false;
	}
	
	bool res = playerWonLastExit ?  zoomEffect(false) : topDownEffect(false);

	if(res) {
		firstRun = true;
		//setBorderColor(!game->roomManager.isWhiteRooms());
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
		
		if(removeEffectsList.contains(*it)) {
			removeEffectsList.erase(*it);
			
			delete (*it);
			(*it) = NULL;
			it = effectList.erase(it);
			continue;
		}
		
		bool res = (*it)->animate();
		
		if(res) {
			delete (*it);
			(*it) = NULL;
			it = effectList.erase(it);
		} else {
			++it;
		}
	}
	
	BN_ASSERT(removeEffectsList.size() == 0, "removeEffectsList size was nonzero after a effectsmanager vblank. this should never happen!");
	
	if(dialogueEndPointer != NULL) {
		dialogueEndPointer->animate();
	}
	
	return;

	/*
	
	if(frame % 4 != 0) {
		return;
	}
	
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
	*/
	
	
	
	
	
	
}

void EffectsManager::loadEffects(EffectHolder* effects, int effectsCount) {

	// eventually, cleanup leftover effects here

	questionMarkCount = 0;
	exitGlowCount = 0;
	
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
	
	if(dialogueEndPointer != NULL) { 
		delete dialogueEndPointer;
		dialogueEndPointer = NULL;
	}
	
	removeEffectsList.clear();
	
	if(effectsCount == 1) { // the first effect is just a dummy to avoid 0 length arrays
		//return;
	}
	
	effectsCount--;
	effects++;
	
	for(int i=0; i<effectsCount; i++) {
		//if(effects->width == 1 && effects->height == 1) { 
		if(false) {
		
			// a smallsprite should be summoned as an effect
			
			// what the fuck was past me smoking when i wrote that^^ 
		
			if(effects->tiles == &bn::sprite_tiles_items::dw_spr_stinklines) {
				
				//BN_LOG("kys", effects->x, " ", effects->y);
				
				// minus 1 to play infinitely?
				// nvm, gods idek what im doing 
				//EffectTypeArray questionMark[] = {EffectType(bn::sprite_tiles_items::dw_spr_stinklines, 9999999)};
				//createEffect(Pos(effects->x / 16, effects->y / 16), EffectTypeCast(questionMark));
				
			} else {
				BN_ERROR("you are a idiot(not you, but me)");
			}
		} else {
			
			BN_LOG("loading bigsprite in Room: ", game->roomManager.currentRoomName(), " id index: ", i);
			bigSprites.push_back(new BigSprite(effects->tiles, effects->x, effects->y, effects->width, effects->height, effects->collide, effects->priority, effects->autoAnimate) );
		}
		effects++;
	}
	
	unsigned roomNameHash = game->roomManager.currentRoomHash();
	
	// i rlly need a better method than this 
	// and also, roonamess being named differently during hard mode just (curse)s me 
	// ill make a better option here, but ughhh its not ideal still
	switch(roomNameHash) {
		
		case hashString("rm_rest_area\0"):
		case hashString("rm_rest_area_1\0"):
		case hashString("rm_rest_area_2\0"):
		case hashString("rm_rest_area_3\0"):
		case hashString("rm_rest_area_4\0"):
		case hashString("rm_rest_area_5\0"):
		case hashString("rm_rest_area_6\0"):
		case hashString("rm_rest_area_7\0"):
		case hashString("rm_rest_area_8\0"):
		case hashString("rm_rest_area_9\0"):
			roomDust();
			break;
		default:
			if(game->roomManager.isWhiteRooms()) {
				roomDust();
			} 
			break;
	}
	
	
	
}

// -----

Dialogue::Dialogue(EffectsManager* effectsManager_, const char* data_) : effectsManager(effectsManager_), originalData(data_) {
	
	
	// gods should i just make a dialogue class 
	// honestly, i didnt need it, and this code is (curse)ing trash.
	// nv, hopefully not anymore, im going to make it less trash 
	
	data = data_;

}

const char* Dialogue::getNextWord() {
	// return the next WORD, in amount of chars 
	// this is done so that we properly cut off strings at word breaks.
	
	const char* tempData = data;
	while(!isWordEnd(*tempData)) {
		tempData++;
	}
	
	return tempData+1; 
}

int Dialogue::getNextLine() {
	// return the next line, in amount of chars 
	
	char buffer[256];
	
	int bufferIndex = 0;
	
	while(true) {
		
		const char* nextWordPtr = getNextWord();
	
		int nextWordCount = nextWordPtr - data - 1;
	
		for(int i=0; i<nextWordCount; i++) {
			buffer[bufferIndex + i] = data[i];
		}
		
		buffer[bufferIndex + nextWordCount] = '\0';
		
		int width = effectsManager->textGenerator.width(bn::string_view(buffer));
		
		buffer[bufferIndex + nextWordCount] = ' ';

		if(width >= 212) {
	       	buffer[bufferIndex] = '\0';
			break;
		}
   
    	data = nextWordPtr;
		bufferIndex += nextWordCount + 1;
		
		if(isLineEnd(*(nextWordPtr-1))) {
    	    break;
    	}
	}


	return bufferIndex;
}

int Dialogue::getNextDialogue(char* res) {
	
	// the bool represents if we should scroll or like, not 
	// now an int, 0 is continue, 1 is newline, 2 is stop 
	// buffer is/should be 256 chars 
	
	if(*data == '\0') {
	    return 2;
	}
	
	const char* dataStart = data;
	
	int charCount = getNextLine();
	
	for(int i=0; i<charCount; i++) {
		res[i] = dataStart[i];
	}
	
	// i swear, it was robs just the online compiler i was using, but i had to do this ok
	res[charCount-1] = (char)0;
	
	
	char* test = res;
	
	while(*test != 0) {
		if(*test == '\r' || *test == '\n') {
			BN_ERROR("wtf");
		}
		test++;
	}
	
	if(*data == '\0') {
	    return 2;
	}
	
	if(*(data-1) == '\n') {
	    return 1;
	}

	return 0;
}

// ----

void EffectsManager::hideForDialogueBox(bool vis, bool isCutscene) {
	
	entityManager->hideForDialogueBox(vis, isCutscene);
	
	int compareVal = isCutscene ? 0 : 6;
	compareVal = (compareVal * 16) - 8;
	
	for(auto it = effectList.cbegin(); it != effectList.cend(); ++it) {
		if((*it) == NULL) {
			continue;
		}
		if((*it)->getY() >= compareVal) {
			(*it)->sprite.spritePointer.set_visible(vis);
		}
    }
	
	if(isCutscene) {
		for(auto it = bigSprites.cbegin(); it != bigSprites.cend(); ++it) {
			if((*it) == NULL) {
				continue;
			}
			(*it)->setVis(vis);
		}
	}
}

void EffectsManager::doDialogue(const char* data, bool isCutscene, const bn::sound_item* sound) {
	
	if(sound == NULL) {
		switch(game->mode) {
			default:
			case 0:
				sound = &bn::sound_items::snd_voice2;
				break;
			case 1:
				sound = &bn::sound_items::snd_lillie;
				break;
			case 2:
				sound = &bn::sound_items::snd_cifer;
		}
	}
	
	// this function has got to be one of the worst things i have ever written in my life.

	// some notes 
	// in order for the text to properly scroll, we are going to need to temporarily overwrite the floor.
	// also, the onesprite per like, ugh 
	// ideally, i would be using background tiles to hide the sprites so i could disable the one char per sprite thing.
	// althox2, i suppose that like,,,, ugh idek anymore .
	// im just going to pray i dont hit that limit(but i def will)
	// will probs need to do some redrawing, with like, a max of 4 lines of sprites actually drawn?
	// ugh
	
	
	// TODO, FOR TEXT OUTLINE, RENDER THE TEXT 4 TIMES ON EACH DIR WITH BLACK TO GEN THE OUTLINE
	
	// TODO, spr_textbox_endpointer, rotate every 5/6 frames(ithink?)
	
	GameState restoreState = game->state;
	if(!isCutscene) {
		game->state = GameState::Dialogue;
	}
	
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
	
	//tileManager->floorLayer.rawMap.reloadCells();
	game->collision.reloadCells();
	//game->details.reloadCells();
	
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
	
	if(!isCutscene) {
		hideForDialogueBox(false, isCutscene);
	}
	
	// ok heres the plan. 
	// nullterm is the end of the text 
	// newline indicates the player should have to press z 
	// carriage return indicates the that like, to continue this text but on the next line
	
	// im rewriting this bs 
	// gods this whole func ended up being such a circlejerk
	
	// allocing this much memory might be,, extremely stupid tbh
	// ugh
	
	// each line contains a line of text 
	// each message contains multiple lines of text 
	// the full strings vector contains multiple messages
	// doing this via some sort of yield would be so much better 
	
	// im going to remove the \r stuff, and just have \n. \r will now be done automaticallys
	
	// oh (curse) 
	// textgenerator has the width( function 
	// wow. i really like 
	// so much of this project has been me only finding major features weeks after 
	
	// the c standard does not require const chars to be null termed. i didnt know this. im dumb
	
	// ok. the first char being null now means that we shouldnt update the screen at the end.
	// this is jank as (curse) 
	// i could/should also,,, have a first/second char be to decide the sound, but ugh 
	// this is primarily to play sound effects between dialogue without cutting the dialogue 
	// totally a better less patchy way to do this 
	
	bool dontUpdateAtEnd = false;
	if(*data == '\0') {
		dontUpdateAtEnd = true;
		data++;
	}
	
	Dialogue dialogue(this, data);
	textSpritesLine1.clear();
	textSpritesLine2.clear();
	
	char buffer[256];
	
	int res = 0;
	bool pressQueued = true;
	int textOffset = 0;
	bool skipScroll = false;
	bool enableSkipScroll = true; 
	
	const char* tempBuffer = data;
	while(*tempBuffer != '\0') {
		if(*tempBuffer == '`') {
			enableSkipScroll = false;
			break;
		}
		tempBuffer++;
	}
	
	auto scrollLine = [sound](bn::sprite_text_generator& textGeneratorObj, 
		bn::vector<bn::sprite_ptr, MAXTEXTSPRITES>& textSprites, 
		char* bufferPtr, 
		int offset,
		bool& skipScrollBool,
		bool enableSkipScrollBool
		) mutable -> bool {
			
		// cringe is occuring! this should/could maybe be static, but im (curse)ing scared
		char filteredBuffer[256];
		
		char* originalBufferPtr = bufferPtr;
		char* filteredBufferPtr = filteredBuffer;
		while(true) {
			
			char c = *originalBufferPtr;
			if(c == '`') {
				c = ' ';
			}
			
			*filteredBufferPtr = c;
			filteredBufferPtr++;
			originalBufferPtr++;
			
			if(c == '\0') {
				break;
			}
		}
		filteredBufferPtr = filteredBuffer;
			
		textGeneratorObj.set_one_sprite_per_character(true);
		textGeneratorObj.generate((bn::fixed)-120+8+4+4, (bn::fixed)40 + offset*16, bn::string_view(filteredBufferPtr), textSprites);
		for(int i=0; i<textSprites.size(); i++) {
			textSprites[i].set_bg_priority(0);
			textSprites[i].set_visible(false);
			textSprites[i].set_palette(spritePalette->getSpritePalette());
		}
		globalGame->doButanoUpdate();
		
		// in the future, this needs to be gotten dynamically
		//const bn::sound_item* sound = &bn::sound_items::snd_voice2;
		
		// i may be bsing, but i think a sound plays at the start and and and when a space occurs 
		// this is actually completley wrong! but im tired ok, and also kinda like it so im leaving it in
		//globalGame->playSound(sound);
		sound->play();
		
		int bufferIndex = 0;
		for(int i=0; i<textSprites.size(); i++) {
			
			if(skipScrollBool && enableSkipScrollBool) {
				break;
			}
						
			if(bufferPtr[bufferIndex] == ' ' || bufferPtr[bufferIndex] == '`') {
				globalGame->playSound(sound);
				if(bufferPtr[bufferIndex] == '`') {
					// delay scroll, this will eat up skipscrolls, but im tired.
					for(int j=0; j<60; j++) {
						globalGame->doButanoUpdate();
					}
				}
				bufferIndex++;
			}
			bufferIndex++;
			textSprites[i].set_visible(true);
			
			for(int j=0; j<2; j++) {
				if(bn::keypad::a_pressed() && enableSkipScrollBool) {
					skipScrollBool = true;
					break;
				}
				globalGame->doButanoUpdate();
			}
		}
		
		globalGame->playSound(sound);
		
		textSprites.clear();
		textGeneratorObj.set_one_sprite_per_character(false);
		textGeneratorObj.generate((bn::fixed)-120+8+4+4, (bn::fixed)40 + offset*16, bn::string_view(filteredBufferPtr), textSprites);
		for(int i=0; i<textSprites.size(); i++) {
			textSprites[i].set_visible(true);
			textSprites[i].set_bg_priority(0);
			textSprites[i].set_palette(spritePalette->getSpritePalette());
		}
		
		globalGame->doButanoUpdate();
		
		return skipScrollBool;
	};
	
	// TODO, THIS WILL NOT PROPERLY SPAWN IN GAMESTATE IS IN CUTSCENE
	// have a special pointer in effectsmanager literally just for this.
	dialogueEndPointer = generateDialogueEndpointer();
	
	while(true) {

		dialogueEndPointer->sprite.spritePointer.set_bg_priority(0);
	
		if(bn::keypad::a_pressed() || pressQueued) {
			dialogueEndPointer->sprite.spritePointer.set_bg_priority(3);
			
			pressQueued = false;
			
			skipScroll = false;
			
			if(res == 2) {
				break;
			}
			
			if(res == 1) {
				// previous res was one, meaning we should clear all sprites
				textSpritesLine1.clear();
				textSpritesLine2.clear();
				textOffset = 0;
			}
			
			if(textSpritesLine1.size() != 0) {
				
				textSpritesLine1.clear();
				textSpritesLine1 = textSpritesLine2;
				textSpritesLine2.clear();

				for(int i=0; i<textSpritesLine1.size(); i++) {
					textSpritesLine1[i].set_y(textSpritesLine1[i].y() - 16);
				}
				
				textOffset=1;
				
				game->doButanoUpdate();
			}
			
			if(textOffset == 0) { 
				
				res = dialogue.getNextDialogue(buffer);
				
				scrollLine(textGenerator, textSpritesLine1, buffer, textOffset, skipScroll, enableSkipScroll);
				
				textOffset++;
			}
		
			if(textOffset == 1 && res == 0) {
				
				res = dialogue.getNextDialogue(buffer);

				scrollLine(textGenerator, textSpritesLine2, buffer, textOffset, skipScroll, enableSkipScroll);
				
			}
			
			
		}

		
		game->doButanoUpdate();
	}
	
	
	delete dialogueEndPointer;
	dialogueEndPointer = NULL;
	
	
	//game->doButanoUpdate();
	
	//game->doButanoUpdate();
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 0);
		}
	}
	effectsLayer.update();
	
	// this is trash, and will cause frame drops 
	game->fullTileDraw();
	
	if(!isCutscene) {
		hideForDialogueBox(true, isCutscene);	
	}
	
	textSpritesLine1.clear();
	textSpritesLine2.clear();
	
	if(!dontUpdateAtEnd) {
		game->doButanoUpdate();
	}
	
	
	game->state = restoreState;
	

}

bool EffectsManager::restRequest(const char* questionString, bool getOption) {
	
	// TODO, THIS TEXT HAS A RDER AROUND IT!!!!
	//probs,,,, oh gods 
	// going to have to modify a lot of stuff to fix that 
	// go with making a border with a palete index of 5, which is transparent most of the time 
	// but if needed, we like yea 
	
	GameState restoreState = game->state;
	game->state = GameState::Dialogue;
	
	bn::vector<bn::sprite_ptr, 64> restSprites;
	bn::vector<bn::sprite_ptr, 4> yesSprites;
	bn::vector<bn::sprite_ptr, 4> noSprites;
	
	textGenerator.set_one_sprite_per_character(false);
	
	auto activeTextPalette = spritePalette->getSpritePalette().create_palette();
	auto alternateTextPalette = spritePalette->getAlternateSpritePalette().create_palette();
	auto blackTextPalette = spritePalette->getBlackSpritePalette().create_palette();
	
	if(game->roomManager.isWhiteRooms()) {
		
		activeTextPalette = spritePalette->getLightGraySpritePalette().create_palette();
		alternateTextPalette = spritePalette->getDarkGraySpritePalette().create_palette();
		
	}
	
	for(int i=0; i<60; i++) {
		game->doButanoUpdate();
	}
	
	int restX = -16;
	int restY = -24;
	
	if(strcmp(questionString, "Rest?\0") != 0) {
		restX -= (textGenerator.width(bn::string_view(questionString)) / 2);
	}
	
	
	//textGenerator.generate((bn::fixed)-16, (bn::fixed)-30, bn::string_view("Rest?\0"), textSpritesLine1);
	textGenerator.generate((bn::fixed)restX, (bn::fixed)restY, bn::string_view(questionString), restSprites);
	for(int i=0; i<restSprites.size(); i++) {
		restSprites[i].set_bg_priority(0);
		restSprites[i].set_palette(activeTextPalette);
	}
	
	bn::vector<bn::vector<bn::sprite_ptr, 64>, 4> restSpritesOutline;
	
	
	for(int j=0; j<4; j++) {
		
		restSpritesOutline.push_back(bn::vector<bn::sprite_ptr, 4>());
		
		bn::fixed x = restX;
		bn::fixed y = restY;
		
		// stupid way of doing this but i dont want to write a switch case
		Pos dif = Pos(1, 1);
		dif.move(static_cast<Direction>(j));
		
		x += (dif.x - 1);
		y += (dif.y - 1);
		
		textGenerator.generate(x, y, bn::string_view(questionString), restSpritesOutline[j]);
		for(int i=0; i<restSpritesOutline[j].size(); i++) {
			restSpritesOutline[j][i].set_bg_priority(0);
			restSpritesOutline[j][i].set_z_order(1);
			restSpritesOutline[j][i].set_palette(blackTextPalette);
		}
	}
	
	int res = 0;
	
	if(getOption) {
	
		for(int i=0; i<60; i++) {
			game->doButanoUpdate();
		}
		
		textGenerator.generate((bn::fixed)-70, (bn::fixed)-10, bn::string_view("[Yes]\0"), yesSprites);
		textGenerator.generate((bn::fixed)40, (bn::fixed)-10, bn::string_view("[No]\0"), noSprites);
		
		for(int i=0; i<yesSprites.size(); i++) {
			yesSprites[i].set_bg_priority(0);
			yesSprites[i].set_palette(alternateTextPalette);
		}
		for(int i=0; i<noSprites.size(); i++) {
			noSprites[i].set_bg_priority(0);
			noSprites[i].set_palette(alternateTextPalette);
		}
		
		while(true) { 
		
			if(res != 0 && bn::keypad::a_pressed()) {
				break;
			}
			
			if(bn::keypad::left_pressed()) {
				res = 2;
				for(int i=0; i<yesSprites.size(); i++) {
					yesSprites[i].set_bg_priority(0);
					yesSprites[i].set_palette(activeTextPalette);
				}
				for(int i=0; i<noSprites.size(); i++) {
					noSprites[i].set_bg_priority(0);
					noSprites[i].set_palette(alternateTextPalette);
				}			
			} else if(bn::keypad::right_pressed()) {
				res = 1;
				for(int i=0; i<yesSprites.size(); i++) {
					yesSprites[i].set_bg_priority(0);
					yesSprites[i].set_palette(alternateTextPalette);
				}
				for(int i=0; i<noSprites.size(); i++) {
					noSprites[i].set_bg_priority(0);
					noSprites[i].set_palette(activeTextPalette);
				}
			}
			
			
			game->doButanoUpdate(); 
		
		}
	} else {
		while(true) {
			if(bn::keypad::a_pressed()) {
				break;
			}
			game->doButanoUpdate(); 
		}
	}
	
	
	for(int i=0; i<4; i++) {
		restSpritesOutline[i].clear();
	}
	restSpritesOutline.clear();
	restSprites.clear();
	yesSprites.clear();
	noSprites.clear();
	game->doButanoUpdate(); 
	
	bool answer = res - 1;
	
	bool doCutsceneDialogue = strcmp(questionString, "Rest?\0") == 0;
	
	if(!answer) {
		if(doCutsceneDialogue) {
			doDialogue("[You decide to move on]\0", false);
		}
		game->state = restoreState;
		return false;
	}
	
	// ` marks are used to delay the text
	if(doCutsceneDialogue) {
		doDialogue("[Slowly,`surely,`dreams embrace you]\0", false);
	}
	// PLAY THE EXIT THINGy
	
	
	game->state = restoreState;
	return true;
}

// -----

// jesus (curse)
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

void MenuOption::setVisible(bool vis) {
	for(int i=0; i<textSprites.size(); i++) {
		textSprites[i].set_visible(vis);
	}
}

void EffectsManager::setBrandColor(int x, int y, bool isTile) {
	
	// this whole func took me a pathetically long length of time due to not eating
	
	bn::optional<bn::span<bn::tile>> tileRefOpt = tilesPointer.vram();
	BN_ASSERT(tileRefOpt.has_value(), "wtf");
	bn::span<bn::tile> tileRef = tileRefOpt.value();
	
	int quadrant = -1;
	
	if(x < 3 && y < 3) {
		quadrant = 0;
	} else if(x >= 3 && y < 3) {
		quadrant = 1;
	} else if(x < 3 && y >= 3) {
		quadrant = 2;
	} else {
		quadrant = 3;
	}
	
	// goofy ah
	
	int xIndex = quadrant % 2 == 0 ? 2+(2*x) : (x-3)*2;
	int yIndex = quadrant < 2 ? 2+(2*y) : (y-3)*2;
	
	//BN_LOG(x, " ", y, " ", xIndex, " ", yIndex, " ", quadrant);
	
	for(int xOffset=0; xOffset<=1; xOffset++) {
		for(int yOffset=0; yOffset<=1; yOffset++) {
			uint32_t tile = tileRef[122 + quadrant].data[yIndex + yOffset];
			
			tile &= ~(0xF << ((xIndex + xOffset) * 4));
			if(isTile) {
				tile |= (0x2 << ((xIndex + xOffset) * 4));
			} else {
				tile |= (0x4 << ((xIndex + xOffset) * 4));
			}
		
			tileRef[122 + quadrant].data[yIndex + yOffset] = tile;
		}
	}
}

void EffectsManager::setMenuVis(bool vis) {
	for(int i=0; i<menuOptions.size(); i++) {
		menuOptions[i].setVisible(vis);
	}
	
	for(int i=0; i<verTextSprites.size(); i++) {
		verTextSprites[i].set_visible(vis);
	}
}

void EffectsManager::doMenu() {
	
	// room switching, profiler reseting, palete switching, and such
	// also show compile time, date, and butano version 
	// BN_VERSION_STRING   __DATE__   __TIME__
	
	setBorderColor();
	
	GameState restoreState = game->state;
	game->state = GameState::Paused;
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 1);
		}
	}
	
	for(int i=0; i<6; i++) {
		unsigned temp = tileManager->playerBrand[i];
		for(int j=0; j<6; j++) {
			setBrandColor(5-j, i, temp & 1);
			temp >>=1;
		}
	}
	
	
	effectsLayer.rawMap.setTile(27, 1, 122);
	effectsLayer.rawMap.setTile(28, 1, 123);
	effectsLayer.rawMap.setTile(27, 2, 124);
	effectsLayer.rawMap.setTile(28, 2, 125);
	
	
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
	
	// these sprites are STATIC and should be put into a bg layer(perhaps the cutscene layer?) to keep 
	// enough sprite slots open such that pausing in tail's room wont crash
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
	
	menuOptions[menuOptions.size() - 1].bPress = []() -> void {
		globalGame->cutsceneManager.inputCustomPalette();
	};
	
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
			
			return player->hasRod || player->hasSuperRod ? ( player->hasSuperRod ? "super" : "normal") : "none";
		},
		[](int val) { 
			(void)val;
			Player* player = globalGame->entityManager.player;
			BN_ASSERT(player != NULL, "in a menufunc, player was null");
	
			
			// trash code 
			if(player->hasRod) {
				if(val > 0) {
					player->hasRod = false;
					player->hasSuperRod = true;
				} else {
					player->hasRod = false;
					player->hasSuperRod = false;
				}
			} else if(player->hasSuperRod) {
				if(val > 0) {
					player->hasRod = false;
					player->hasSuperRod = false;
				} else {
					player->hasRod = true;
					player->hasSuperRod = false;
				}
			} else {
				if(val > 0) {
					player->hasRod = true;
				} else {
					player->hasSuperRod = true;
				}
			}
			
			//player->hasSuperRod	= !player->hasSuperRod;
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
	
	for(int i=0; i<menuOptions.size(); i++) {
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
			// else if(isActive && bn::keypad::b_pressed()) {
			} else if(bn::keypad::b_pressed()) {
				if(menuOptions[selectedOption].bPress != NULL) {
					menuOptions[selectedOption].bPress();
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
	setBorderColor(!game->roomManager.isWhiteRooms());
	
	effectsLayer.reloadCells();
	game->doButanoUpdate();
	
	game->state = restoreState;

	
}

// -----

void EffectsManager::glassBreak(Pos p) {
	
	// we cannot use static vars in this joint, so static "vars" will have to be,,, alloced in real time. i think?
	// or i can just use the canerious std::functional
	// gods this is absolutely horrid.
	// i really should of just made (curse)ing,, i should of just made graphicsIndex a static var 
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

void EffectsManager::sparkle(Pos p, int sparkleLength) {
	
	// sparkle
	createEffect(
	[p](Effect* obj) mutable -> void {
		
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
	[sparkleLength](Effect* obj) mutable -> bool {
		obj->graphicsIndex++;
		if(obj->graphicsIndex == sparkleLength) {
			return true;
		}
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_sparkle,
			obj->graphicsIndex % 8
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
			//obj->x -= 8;
		} else if(dir == Direction::Down) {
			obj->x += 16;
			//obj->x -= 8;
		} else { // left or right
			obj->sprite.spritePointer.set_rotation_angle(270);			
			if(dir == Direction::Right) {
				obj->sprite.spritePointer.set_vertical_flip(true);
				obj->x += 16;
			}
			//obj->y += 8;
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
	sparkle(p);
	
	// rod
	createEffect(
	[p, dir](Effect* obj) mutable -> void {
		
		// tbh,,, i should of went with sprite instead of sprite_tiles for like,, everything 
		// but changing that back would be way to much
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		
		if(globalGame->entityManager.player->hasSuperRod) {
			obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_void_rod_endless, bn::sprite_shape_size(32, 32));
			obj->tiles = &bn::sprite_tiles_items::dw_spr_void_rod_endless;
			
			constexpr int xOffsets[4] = {2, 2, 3, 3};
			obj->x += xOffsets[static_cast<int>(dir)];
			
			constexpr int yOffsets[4] = {3, 3, 3, 3};
			obj->y += yOffsets[static_cast<int>(dir)];
		} else {
			obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_void_rod);
			obj->tiles = &bn::sprite_tiles_items::dw_spr_void_rod;
		}
		
		
		// U D L R
		// R U L D
		
		constexpr int dirConverter[4] = {1, 3, 2, 0};
		
		obj->graphicsIndex = dirConverter[static_cast<int>(dir)];
		
		// todo, make this work with the super rod
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex
		);
		

		
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
	
	if(globalGame->entityManager.player->hasSuperRod) {
		
		// do the super rods effect anim 
		// the issue? using 4 sprites for the outline like i am rn is,,, ugh 
		
		auto activeTextPalette = spritePalette->getSpritePalette().create_palette();
		auto alternateTextPalette = spritePalette->getAlternateSpritePalette().create_palette();
		auto blackTextPalette = spritePalette->getBlackSpritePalette().create_palette();
		
		bn::vector<bn::sprite_ptr, 16> mainNumberSprites;
		//bn::vector<bn::sprite_ptr, 16> outlineNumberSprites[4];
		// a basic array didnt seem to want to work with bn::move, valid tho
		bn::vector<bn::vector<bn::sprite_ptr, 16>, 4> outlineNumberSprites(4, bn::vector<bn::sprite_ptr, 16>());
		
		Pos playerPos = globalGame->entityManager.player->p;
		int val = globalGame->entityManager.player->rod.size();
		
		bn::string<16> string;
		bn::ostringstream stream(string);
		
		stream << val;
		
		int xVal = (16 * playerPos.x) - 240/2 + 12;
		int yVal = (16 * playerPos.y) - 160/2 + 8;

		textGenerator.set_center_alignment();

		textGenerator.generate((bn::fixed)xVal, (bn::fixed)yVal, bn::string_view(string), mainNumberSprites);
		for(int i=0; i<mainNumberSprites.size(); i++) {
			mainNumberSprites[i].set_bg_priority(0);
			mainNumberSprites[i].set_z_order(-2);
			mainNumberSprites[i].set_palette(blackTextPalette);
		}
		
		
		// i dislike seperating these calls, but i dont trust shit
		for(int j=0; j<4; j++) {

			// stupid way of doing this but i dont want to write a switch case
			Pos dif = Pos(1, 1);
			dif.move(static_cast<Direction>(j));
			
			int tempX = xVal + (dif.x - 1);
			int tempY = yVal + (dif.y - 1);
			
			textGenerator.generate((bn::fixed)tempX, (bn::fixed)tempY, bn::string_view(string), outlineNumberSprites[j]);

			for(int i=0; i<outlineNumberSprites[j].size(); i++) {
				outlineNumberSprites[j][i].set_bg_priority(0);
				outlineNumberSprites[j][i].set_z_order(-1);
				outlineNumberSprites[j][i].set_palette(activeTextPalette);
			}
		}
		
		textGenerator.set_left_alignment();
		
		//while(true){game->doButanoUpdate();}
		
		createEffect([](Effect* obj) mutable -> void {
			obj->sprite.updateRawPosition(-32, -32);
		},
		// this syntax is something i should of been doing for a while
		// i could also declare less complex, static vars in here 
		// bn::move is also,,, something that i wish i knew earlier. it isnt in the docs
		[
		mainNumberSprites = bn::move(mainNumberSprites), 
		outlineNumberSprites = bn::move(outlineNumberSprites)
		](Effect* obj) mutable -> bool {
			
			
			if(obj->tempCounter < 8) {
				for(int i=0; i<mainNumberSprites.size(); i++) {
					mainNumberSprites[i].set_y(mainNumberSprites[i].y() - 1);
				}
				
				for(int i=0; i<4; i++) {
					for(int j=0; j<outlineNumberSprites[i].size(); j++) {
						outlineNumberSprites[i][j].set_y(outlineNumberSprites[i][j].y() - 1);
					}
				}
			}
			
			
			obj->tempCounter++;
			if(obj->tempCounter == 12) {
				return true;
			}
			return false;
		}
		);
	}
		
		
}

void EffectsManager::wings() {
	
	// spr_void_wings for gray and lily 
	// spr_void_wings_dissipate for gray and lily (how exactly does this layer work?)
	
	// spr_void_wings_cif for cif 
	// spr_void_wings_dissipate_cif for cif
	
	// is precomputing this right here,, ok
	// ima just decide the sprite item in-func 
	// why isnt there a (curse)ing func that lets me just change the graphics index without having to also lable the sprite item??
	
	// this code is (curse)ing (curse).
	

	
	bn::sound_items::snd_wingspawn.play();
	
	Player* player = entityManager->player;
	
	auto getCreateFunc = [player](int wingNum) -> auto {
		auto createFunc = [player, wingNum](Effect* obj) mutable -> void {
			
			obj->tempCounter = wingNum; // discern wings from each other 
			obj->tempCounter2 = 1; // first run bypass

			obj->tiles = globalGame->mode == 2 ? &bn::sprite_tiles_items::dw_spr_void_wings_cif : &bn::sprite_tiles_items::dw_spr_void_wings;
			
			
			obj->sprite.spritePointer.set_x(120+32);
			obj->sprite.spritePointer.set_y(80+32);
			
			// this is something i should be doing much more :
			obj->animateFunc(obj);
	
		};
		return createFunc;
	};
	
	auto wingTickFunc = [player](Effect* obj) -> bool {
		
		
		if(globalGame->entityManager.player->wingsUse == 0) {
			// play the wing destroy anim here!
			
			bn::fixed xPos = obj->sprite.spritePointer.x();
			bn::fixed yPos = obj->sprite.spritePointer.y();
			
			bool horizontalFlip = obj->sprite.spritePointer.horizontal_flip();
			
			if(player->currentDir == Direction::Left || player->currentDir == Direction::Right) {
				if((player->currentDir == Direction::Right) != horizontalFlip) {
					horizontalFlip = player->currentDir == Direction::Right;
					xPos += player->currentDir == Direction::Right ? -8 : 8;
				}
			}
			
			auto dissipateCreateFunc = [xPos, yPos, horizontalFlip](Effect* obj2) -> void {
				
				obj2->tiles = globalGame->mode == 2 ? &bn::sprite_tiles_items::dw_spr_void_wings_dissipate_cif : &bn::sprite_tiles_items::dw_spr_void_wings_dissipate;
				
				obj2->sprite.spritePointer.set_x(xPos);
				obj2->sprite.spritePointer.set_y(yPos);
				
				obj2->sprite.spritePointer.set_horizontal_flip(horizontalFlip);
				
				obj2->graphicsIndex = -1;
				obj2->animateFunc(obj2);
				
			};
			
			auto dissipateTickFunc = [](Effect* obj2) -> bool {
				obj2->graphicsIndex++;
				
				if(obj2->graphicsIndex == 13) {
					return true;
				}
				obj2->sprite.spritePointer.set_tiles(
					*obj2->tiles,
					obj2->graphicsIndex
				);
				
				return false;
			};
			
			globalGame->effectsManager.createEffect(dissipateCreateFunc, dissipateTickFunc);
			
			return true;
		}
		
		if(frame % 2 == 0 && obj->tempCounter2 == 0) {
			return false;
		}
		obj->tempCounter2 = 0;
		
		obj->graphicsIndex++;
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex % 3
		);
		
		Direction dir = player->currentDir;
		
		int xOffset = 0;
		int yOffset = 0;
		
		/*
		obj->x = goofyP.x * 16;
		obj->y = goofyP.y * 16;
		if(goofyDir == Direction::Right) {
			obj->sprite.spritePoint er.set_horizontal_flip(true);
			obj->x -= 8;
		} else if(goofyDir == Direction::Left) {
			obj->x += 8;
		}
	
		obj->x += offset;
		
		if(offset < 0) {
			obj->sprite.spritePointer.set_horizontal_flip(true);
		}
		
		if(player->currentDir == Direction::Up || player->currentDir == Direction::Down) {
			obj->y -= 4;
		}
		*/
		
		if(dir == Direction::Left || dir == Direction::Right) {
			obj->sprite.spritePointer.set_horizontal_flip(dir == Direction::Right);
			
			xOffset = dir == Direction::Right ? -8 : 8;
			//yOffset = -2;
		} else {
			xOffset = obj->tempCounter == 0 ? -8 : 8;
			obj->sprite.spritePointer.set_horizontal_flip(obj->tempCounter == 0);
		}
		
		// dont set sprite prio when down bc, behind
		if(dir == Direction::Down) {
			obj->sprite.spritePointer.set_z_order(0);
		} else {
			obj->sprite.spritePointer.set_z_order(-1);
		}
		
		
		
		obj->sprite.spritePointer.set_x(player->sprite.spritePointer.x() + xOffset);
		obj->sprite.spritePointer.set_y(player->sprite.spritePointer.y() + yOffset);
		
		
		return false;
	};

	
	createEffect(getCreateFunc(0), wingTickFunc);
	createEffect(getCreateFunc(1), wingTickFunc);
	
}

void EffectsManager::explosion(Pos p)  {
	
	// spr_explosion, seems that we need two of them, are their poses randomized?
	
	auto goofy = [](Effect* obj, Pos goofyP, int offset) mutable -> void {
	
		obj->x = goofyP.x * 16;
		obj->y = goofyP.y * 16;

		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_explosion,
			0
		);
	
		obj->x += offset;
		obj->y += offset;
	
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
		obj->sprite.spritePointer.set_z_order(-2);
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
		obj->graphicsIndex++;

		if(obj->graphicsIndex == 7 * 4) {
			return true;
		}
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_explosion,
			obj->graphicsIndex % 7
		);
		return false;
	};
	
	auto func1 = [p, goofy](Effect* obj) mutable -> void {	
		goofy(obj, p, 4);
	};
	
	auto func2 = [p, goofy](Effect* obj) mutable -> void {	
		goofy(obj, p, -4);
	};
	
	createEffect(func1, tickFunc);
	createEffect(func2, tickFunc);
	
}

void EffectsManager::sword(Pos p, Direction dir) {
	
	
	auto createFunc = [p, dir](Effect* obj) mutable -> void {
		
		obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_void_rod);
		
		// U D L R
		// R U L D
		
		constexpr int dirConverter[4] = {1, 3, 2, 0};
		
		obj->graphicsIndex = dirConverter[static_cast<int>(dir)];
		
		obj->sprite.spritePointer.set_tiles(
			globalGame->mode == 2 ? bn::sprite_tiles_items::dw_spr_void_sword_cif : bn::sprite_tiles_items::dw_spr_void_sword,
			obj->graphicsIndex
		);
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		
		obj->sprite.updateRawPosition(obj->x, obj->y);
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
		obj->tempCounter++;
		if(obj->tempCounter == 12) {
			return true;
		}
		return false;
	};
	
	createEffect(createFunc, tickFunc);
	
}

void EffectsManager::monLightning(Pos p, Direction dir) {
	
	// spr_mon_shock_small for when it kills you
	
	// i dont believe this is the right sound 
	game->playSound(&bn::sound_items::snd_player_damage);
	
	p.move(dir);
	
	// wow; wtf
	while(p != globalGame->entityManager.player->p) {
		auto createFunc = [p, dir](Effect* obj) mutable -> void {
			
			obj->sprite.spritePointer.set_tiles(
				bn::sprite_tiles_items::dw_spr_mon_shock_small,
				obj->graphicsIndex
			);
			
			if(dir == Direction::Up || dir == Direction::Down) {
				obj->sprite.spritePointer.set_rotation_angle(90);
			}
			
			obj->x = p.x * 16;
			obj->y = p.y * 16;
			
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
		};
		
		auto tickFunc = [](Effect* obj) mutable -> bool {
		
			obj->tempCounter++;
			if(obj->tempCounter == 30) {
				return true;
			}
			
			obj->graphicsIndex = !obj->graphicsIndex;
			
			// this should get destroyed once the new level loads, and it being here means we are dead, so like, its only going to return false
			obj->sprite.spritePointer.set_tiles(
				bn::sprite_tiles_items::dw_spr_mon_shock_small,
				obj->graphicsIndex
			);
			return false;
		};
		
		createEffect(createFunc, tickFunc, 2);
	
		p.move(dir);
	}
	
	// dw_spr_mon_shock
	
	auto shockCreateFunc = [p](Effect* obj) mutable -> void {
			
			obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_mon_shock, bn::sprite_tiles_items::dw_spr_mon_shock_shape_size);
			
			obj->sprite.spritePointer.set_tiles(
				bn::sprite_tiles_items::dw_spr_mon_shock,
				obj->graphicsIndex
			);
			
			obj->x = p.x * 16;
			obj->y = p.y * 16;
			
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
		};
	
	
	auto shockTickFunc = [](Effect* obj) mutable -> bool {
		
		obj->tempCounter++;
		if(obj->tempCounter == 30) {
			return true;
		}
		
		obj->graphicsIndex = !obj->graphicsIndex;
		
		// this should get destroyed once the new level loads, and it being here means we are dead, so like, its only going to return false
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_mon_shock,
			obj->graphicsIndex
		);
		return false;
	};
	
	createEffect(shockCreateFunc, shockTickFunc, 2);
	
	// initing this outside of the lambdas will cause char to not swap if someone pauses while dying 
	// but tbh actually, they cant pause while dying anyway 
	
	// these sprites are the gblink sprites, first 4 are swaped through, last one is the final glass, play the glass noise when you get there 
	// snd_golden
	
	// index with 4*mode + dir, dir is up down left right
	constexpr const bn::sprite_tiles_item* tiles[12] = {
		&bn::sprite_tiles_items::dw_spr_player_gblink_u, &bn::sprite_tiles_items::dw_spr_player_gblink_d, &bn::sprite_tiles_items::dw_spr_player_gblink_l, &bn::sprite_tiles_items::dw_spr_player_gblink_r,
		&bn::sprite_tiles_items::dw_spr_lil_gblink_u, &bn::sprite_tiles_items::dw_spr_lil_gblink_d, &bn::sprite_tiles_items::dw_spr_lil_gblink_l, &bn::sprite_tiles_items::dw_spr_lil_gblink_r,
		&bn::sprite_tiles_items::dw_spr_cif_gblink_u, &bn::sprite_tiles_items::dw_spr_cif_gblink_d, &bn::sprite_tiles_items::dw_spr_cif_gblink_l, &bn::sprite_tiles_items::dw_spr_cif_gblink_r
	};
	
	
	const bn::sprite_tiles_item* useTile = tiles[4*game->mode + static_cast<int>(entityManager->player->currentDir)];
	
	auto blinkCreateFunc = [p, useTile](Effect* obj) mutable -> void {
			
		obj->sprite.spritePointer.set_tiles(
			*useTile,
			obj->graphicsIndex
		);
		
		globalGame->entityManager.player->sprite.setVisible(false);
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		
		obj->sprite.updateRawPosition(obj->x, obj->y);
		obj->sprite.spritePointer.set_z_order(-2); // this is above the player right?
		
	};
	
	
	auto blinkTickFunc = [p, useTile](Effect* obj) mutable -> bool {
		
		obj->tempCounter++;
		if(obj->tempCounter >= 60) {
			if(obj->tempCounter == 60) {
				globalGame->playSound(&bn::sound_items::snd_golden);
				obj->sprite.spritePointer.set_tiles(
					*useTile,
					4
				);
				// this looks ugly, but im tired
				globalGame->effectsManager.sparkle(p, 24);
			}
			
			return false;
		}
		
		obj->graphicsIndex = (obj->graphicsIndex + 1) % 4;
		
		// this should get destroyed once the new level loads, and it being here means we are dead, so like, its only going to return false
		obj->sprite.spritePointer.set_tiles(
			*useTile,
			obj->graphicsIndex
		);
		return false;
	};
	
	createEffect(blinkCreateFunc, blinkTickFunc);
	
}

Effect* EffectsManager::getRoomDustEffect(bool isCutscene) {
	
	auto createFunc = [](Effect* obj) mutable -> void {

		if(randomGenerator.get() & 1) {
			obj->tiles = &bn::sprite_tiles_items::dw_spr_dustparticle;
		} else {
			obj->tiles = &bn::sprite_tiles_items::dw_spr_dustparticle2;
		}
	
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			0
		);
	
		obj->sprite.spritePointer.set_z_order(1);
	
		obj->x = -32;
		obj->y = -32;
		obj->sprite.updateRawPosition(obj->x, obj->y);
	};
	
	auto tickFunc = [
		x = (bn::fixed)-32, 
		y = (bn::fixed)-32, 
		image_speed = (bn::fixed)0,
		y_speedup = randomGenerator.get_int(2, 6 + 1),
		t = randomGenerator.get_int(0, 180 + 1),
		amplitude = ((bn::fixed)randomGenerator.get_int(4, 12 + 1)) / 20,
		graphicsIndex = (bn::fixed)0,
		freezeFrames = randomGenerator.get_int(0, 60 + 1),
		isCutscene
		](Effect* obj) mutable -> bool {
		
		if(y < -16) {
			if(randomGenerator.get() & 1) {
				obj->tiles = &bn::sprite_tiles_items::dw_spr_dustparticle;
			} else {
				obj->tiles = &bn::sprite_tiles_items::dw_spr_dustparticle2;
			}
			x = randomGenerator.get_int(16 * 14);
			//y = 16*5+randomGenerator.get_int(16);
			if(y == -32) {
				if(isCutscene) {
					y = 16 + randomGenerator.get_int(16 * 9);
				} else {
					y = 16 + randomGenerator.get_int(16 * 5);
				}
			} else if(isCutscene) {
				y = 160 + randomGenerator.get_int(32);
			} else {
				y = 16*4+randomGenerator.get_int(32);
			}
			
			
			
			image_speed = (bn::fixed)0;
			y_speedup = randomGenerator.get_int(2, 6 + 1);
			t = randomGenerator.get_int(0, 180 + 1);
			amplitude = ((bn::fixed)randomGenerator.get_int(4, 12 + 1)) / 40;
			//graphicsIndex = (bn::fixed)0;
			graphicsIndex = (bn::fixed)randomGenerator.get_int(0, 9 + 1);
			freezeFrames = randomGenerator.get_int(0, 60 + 1);
			
			randomGenerator.update();
		}
		
		if(image_speed > 9) {
			freezeFrames = randomGenerator.get_int(0, 60 + 1);
		}
		

		image_speed += 0.02;
		//image_speed += 0.20;
		
		//y -= (0.1 * y_speedup);
		y -= (0.075 * y_speedup);
		
		t = ((t + 1) % 360);
		x = (x + (amplitude * sinTable[t]));
		
		if(x > 240) {
			x -= 240;
		} else if(x < 0) {
			x += 240;
		}
		
		
		
		BN_ASSERT(obj->tiles != NULL, "dust tileset pointer was null. wtf");
		
		graphicsIndex += image_speed / 60;
		
		// replacing this with freezeFrames > 0
		// while bugged, also worked quite well
		if(freezeFrames == 0) {
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				graphicsIndex.integer() % 9
			);
		} else {
			freezeFrames--;
		}
	
		obj->x = x.integer();
		obj->y = y.integer();
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
		return false;
	};
	
	Effect* e = new Effect(createFunc, tickFunc);
	
	return e;
}

void EffectsManager::roomDust() {

	// gml_Object_obj_dustparticle2_Step_0 seems to have the code 
	
	BN_LOG("CREATING ROOMDUST");
	
	for(int unused = 0; unused<16; unused++) {
	
		Effect* e = getRoomDustEffect();
		effectList.push_back(e);	
	}

}

void EffectsManager::entityKill(Entity* entity) {
	
	EntityType t = entity->entityType();
	Pos p = entity->p;
	
	// using these such as to get the past poses of the entities, where they should be/are currently
	bn::fixed entityX = entity->sprite.screenx;
	bn::fixed entityY = entity->sprite.screeny;
	
	
	if(p != entity->sprite.getCurrentScreenPos()) {
		p = entity->sprite.getCurrentScreenPos();
	}
	
	entity->sprite.setVisible(false);
	
	if(t == EntityType::Player) {
		game->playSound(&bn::sound_items::snd_player_damage);
		
		entityManager->player->wingsUse = 0;
		
		const bn::sprite_tiles_item* tiles;
		
		switch(game->mode) {
			default:
			case 0:
				tiles = &bn::sprite_tiles_items::dw_spr_player_hit;
				break;
			case 1:
				tiles = &bn::sprite_tiles_items::dw_spr_lil_hit;
				break;
			case 2:
				tiles = &bn::sprite_tiles_items::dw_spr_cif_hit;
				break;
		}
		
		
		auto createFunc = [p, entityX, entityY, tiles](Effect* obj) mutable -> void {
			obj->sprite.spritePointer.set_tiles(
				*tiles,
				0
			);
			obj->x = p.x * 16;
			obj->y = p.y * 16;
			obj->sprite.updateRawPosition(obj->x-1, obj->y);
			
			//obj->sprite.spritePointer.set_x(entityX-1);
			//obj->sprite.spritePointer.set_y(entityY);
		};
		
		auto tickFunc = [p, entityX, entityY, tiles](Effect* obj) mutable -> bool {
			
			if(frame % 6 != 0) {
				return false;
			}
			obj->graphicsIndex++;
			obj->sprite.spritePointer.set_tiles(
				*tiles,
				obj->graphicsIndex % tiles->graphics_count()
			);
			
			if(obj->tempCounter == 0) {
				//obj->sprite.spritePointer.set_x(entityX+1);
				obj->sprite.setRawX(p.x * 16 + 1);
			} else {
				//obj->sprite.spritePointer.set_x(entityX-1);
				obj->sprite.setRawX(p.x * 16 - 1);
			}
			
			obj->tempCounter = !obj->tempCounter;
			
			
			return false;
		};
		
		createEffect(createFunc, tickFunc);
	
	} else {
		
		
		
		// the fact that i cannot set a graphics index with a tiles ptr, bc  
		// tiles ptrs are only supposed to be used for single graphicsindex sprites
		// yet the only method i have of returning the tiles from a spritepointer is to get the sprite_tiles_ptr, is, atrocious
		// additionally, i have no method of grabbing a graphicsindex from a spritePtr.
		// i could,,, clone the spriteptr?
		// but i still need the current sprite item, which i have 
		// and tbh if i have the item(which i do), ill just rotate through graphicsindexes and be done
		// due to general bull(curse)ery, i think ill have to assume that the only things that can be passed into here are 16x16 sprites.
		// omfg, tilesptr ONLY RETURNSTHE TILES FOR LIKE WHAT ITS CURRENT FRAME IS
		// ok, theres options here. 
		// i may not have access to graphicsindex, but thats ok. 
		// i can,,, access the spriteTilesArray array in the entity for what frame its on?
		// spriteTilesArray[tileIndex], should work,,, unless im updating it in another area, which i hope im not 
		
		//const bn::sprite_tiles_ptr& tilesPtr = entity->sprite.spritePointer.tiles();
		
		const bn::sprite_tiles_item& tilesItem = entity->spriteTilesArray[entity->tileIndex];
		
		auto createFunc = [p, entityX, entityY, tilesItem](Effect* obj) mutable -> void {
			obj->sprite.spritePointer.set_tiles(
				tilesItem,
				0
			);
			
			obj->x = p.x * 16;
			obj->y = p.y * 16;
			obj->sprite.updateRawPosition(obj->x, obj->y);
			//obj->sprite.spritePointer.set_x(entityX);
			//obj->sprite.spritePointer.set_y(entityY);
			
		};
		
		auto tickFunc = [tilesItem](Effect* obj) mutable -> bool {
			if(frame % 6 != 0) {
				return false;
			}
			obj->graphicsIndex++;
			obj->sprite.spritePointer.set_tiles(
				tilesItem,
				obj->graphicsIndex % tilesItem.graphics_count()
			);
			return false;
		};
		
		createEffect(createFunc, tickFunc);
		
	}
	
	
	
}

void EffectsManager::entityFall(Entity* entity) {

	struct fallFrame {
		const bn::sprite_tiles_item* spriteTiles = NULL;
		int frameCount = 0;
	};
	
	entity->sprite.setVisible(false);
	
	EntityType t = entity->entityType();
	Pos p = entity->p;
	
	if(t == EntityType::Player && p != entity->sprite.getCurrentScreenPos()) {
		p = entity->sprite.getCurrentScreenPos();
	}
	
	bn::fixed entityX = entity->sprite.screenx;
	bn::fixed entityY = entity->sprite.screeny;
	
	auto createFallEffect = [p, entityX, entityY](SaneVector<fallFrame, 8> fallData) -> Effect* {
		
		return new Effect(
		[fallData, p, entityX, entityY](Effect* e) -> void {
			
			e->graphicsIndex = 0;
			e->tempCounter = 0;
			
			e->x = p.x * 16;
			e->y = p.y * 16;
			e->sprite.updateRawPosition(e->x, e->y);
			//e->sprite.spritePointer.set_x(entityX);
			//e->sprite.spritePointer.set_y(entityY);
			
			e->sprite.spritePointer.set_tiles(
				*fallData[e->tempCounter].spriteTiles,
				e->graphicsIndex
			);
			
			
		},
		[fallData, p](Effect* e) -> bool {
			
			if(globalGame->tileManager.hasFloor(p)) {
				
				// at this point, you have to start wondering if im only doing this for the bit
				
				globalGame->playSound(&bn::sound_items::snd_reveal);
				
				auto createPlonk = [p](bool left) -> void {
					globalGame->effectsManager.createEffect(
						[p, left](Effect* e2) -> void {
							
							e2->tiles = &bn::sprite_tiles_items::dw_spr_plonk;
							
							e2->sprite = Sprite(*e2->tiles, bn::sprite_shape_size(32, 32));
							
							e2->graphicsIndex = 0;
							e2->tempCounter = 0;
							
							e2->x = p.x * 16;
							
							e2->x += left ? 16 : -16;
							
							e2->y = p.y * 16 - 8;
							e2->sprite.updateRawPosition(e2->x, e2->y);
							
							e2->sprite.spritePointer.set_horizontal_flip(!left);
							
							e2->sprite.spritePointer.set_tiles(
								*e2->tiles,
								e2->graphicsIndex
							);
							
							
						},
						[](Effect* e2) -> bool {
							
							if(frame % 2 != 0) {
								return false;
							}
						
							e2->graphicsIndex++;	
							
							if(e2->graphicsIndex == e2->tiles->graphics_count()) {
								return true;
							}
							
							e2->sprite.spritePointer.set_tiles(
								*e2->tiles,
								e2->graphicsIndex
							);
							
							
							return false;
						}
					);
				};
				
				createPlonk(false);
				createPlonk(true);
				
				
				return true;
			}
		
			if(frame % 8 != 0) {
				return false;
			}
			
			e->graphicsIndex++;
			if(e->graphicsIndex == fallData[e->tempCounter].frameCount) {
				e->graphicsIndex = 0;
				e->tempCounter++;
				if(e->tempCounter == fallData.size()) {
					return true;
				}
			}
			
			
			e->sprite.spritePointer.set_tiles(
				*fallData[e->tempCounter].spriteTiles,
				e->graphicsIndex % fallData[e->tempCounter].spriteTiles->graphics_count()
			);
			
			return false;
		}
		);
	};
	

	if(t == EntityType::Player) {
		entityManager->player->wingsUse = 0;
		game->playSound(&bn::sound_items::snd_player_fall);
	} else {
		game->playSound(&bn::sound_items::snd_fall);
	}

	switch(t) {
		case EntityType::Player     :
			switch(game->mode) {
				default:
				case 0:
						effectList.push_back(createFallEffect({ {&bn::sprite_tiles_items::dw_spr_player_fall, 6} }));
						break;
				case 1:
						effectList.push_back(createFallEffect({ {&bn::sprite_tiles_items::dw_spr_lil_fall, 6} }));
						break;
					case 2:
						effectList.push_back(createFallEffect({ {&bn::sprite_tiles_items::dw_spr_cif_fall, 6} }));
						break;
			}
			break;
		case EntityType::Leech      :
			effectList.push_back(createFallEffect({ {&bn::sprite_tiles_items::dw_spr_cl_falling, 6} }));
			break;
		case EntityType::Maggot     :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_cc_falling, 6}}));
			break;
		case EntityType::Eye        :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_ch_falling, 6}}));
			break;
		case EntityType::Bull       :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_cg_falling, 6}}));
			break;
		case EntityType::Chester    :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_cs_fall, 8}, {&bn::sprite_tiles_items::dw_spr_cs_falling, 6}}));
			break;
		case EntityType::WhiteMimic :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_cm_falling, 6}}));	
			break;
		case EntityType::GrayMimic  :	
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_cm_falling1, 6}}));	
			break;
		case EntityType::BlackMimic :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_cm_falling2, 6}}));	
			break;
		case EntityType::Diamond    :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_co_fall, 2}, {&bn::sprite_tiles_items::dw_spr_co_falling, 6}}));
			break;
		case EntityType::Shadow     :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_fall, 6}})); // this is actually,,, correct		
			break;
		case EntityType::Boulder    :
		case EntityType::AddStatue  :
		case EntityType::EusStatue  :
		case EntityType::BeeStatue  :
		case EntityType::MonStatue  :
		case EntityType::TanStatue  :
		case EntityType::GorStatue  :
		case EntityType::LevStatue  :
		case EntityType::CifStatue  :
			effectList.push_back(createFallEffect({{&bn::sprite_tiles_items::dw_spr_fall, 6}}));
			break;
		default:
			BN_ERROR("unknown entitytype passed into entityFall, wtf");
			break;
	}
}

void EffectsManager::playerBrandRoomBackground() {
	
	auto createFunc = [](Effect* obj) mutable -> void {

		int randIndex = randomGenerator.get_int(0, 4);

		if(randIndex == 0) {
			obj->sprite = Sprite(bn::sprite_items::dw_default_sprite_32_32);
		} else if(randIndex == 1) {
			obj->sprite = Sprite(bn::sprite_items::dw_default_sprite_64_32);
		} else if(randIndex == 2) {
			obj->sprite = Sprite(bn::sprite_items::dw_default_sprite_32_64);
		} else {
			obj->sprite = Sprite(bn::sprite_items::dw_default_sprite_64);
		}
	
		const bn::sprite_palette_ptr tempPalettes[3] = {
		globalGame->pal->getWhiteSpritePalette().create_palette(),
		globalGame->pal->getDarkGraySpritePalette().create_palette(),
		globalGame->pal->getLightGraySpritePalette().create_palette()};
	
		obj->sprite.spritePointer.set_palette(tempPalettes[randomGenerator.get_int(0, 3)]);

		obj->sprite.spritePointer.set_bg_priority(3);
	
		obj->tempCounter = randomGenerator.get_int(1, 8);
		obj->tempCounter2 = randomGenerator.get_int(1, 8);
	
		obj->tempCounter = randomGenerator.get_int(0, 2) == 0 ? obj->tempCounter : -obj->tempCounter;
		obj->tempCounter2 = randomGenerator.get_int(0, 2) ==0 ? obj->tempCounter2 : -obj->tempCounter2;
		
	
		obj->x = randomGenerator.get_int(-32, 32);
		obj->y = randomGenerator.get_int(-32, 32);
		
		obj->sprite.spritePointer.set_position(obj->x, obj->y);
	};

	auto tickFunc = [](Effect* obj) mutable -> bool {
			
		obj->x += obj->tempCounter;
		obj->y += obj->tempCounter2;
		obj->sprite.spritePointer.set_position(obj->x, obj->y);
		
		const int cutOff = 32;
		if(obj->x > 240/2 + cutOff || obj->y > 160/2 + cutOff ||
		obj->x < -(240/2+cutOff) || obj->y < -(160/2+cutOff)) {
			return true;
		}
		
		return false;
	};
	
	// creating all rects on the same frame was causing not frame drops, but vblank drops 
	// which fucked up the audio. this will fix it 
	
	auto generatorCreateFunc = [](Effect* obj) mutable -> void {
		
		obj->x = -32;
		obj->y = -32;
		
		obj->sprite.spritePointer.set_position(obj->x, obj->y);
	};
	
	
	auto generatorTickFunc = [this, createFunc, tickFunc](Effect* obj) mutable -> bool {
		(void)obj;
		
		static int effectCount = 0;
		
		if(effectCount >= 16) {
			effectCount = 0;
			return true;
		}
		
		createEffect(createFunc, tickFunc);
		//Effect* e = new Effect(createFunc, tickFunc);
		//effectList.insert(effectList.begin(), e);
		
		
		effectCount++;
		
		return false;	
	};
	
	
	createEffect(generatorCreateFunc, generatorTickFunc);

}

Effect* EffectsManager::generateSweatEffect(Entity* sweatEntity) {
	
	// we return the effect here such as to destruct it properly
	// in the playerinput func 
	
	// i feel like this has a high likelyhood of being dumb
	// but it at least will make my accesses shorter
	// i really should of overloaded all sprite_ptr funcs into spritepointer, but its wayyyy far gone now
	if(sweatEntity == NULL) {
		sweatEntity = entityManager->player;
	}
	
	auto createFunc = [sweatEntity](Effect* obj) mutable -> void {
		
		obj->graphicsIndex = 0;
		
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_sweat,
			obj->graphicsIndex
		);
		
		int xDiffs[4] = {-8, 8, 8, -8};
		//int yDiffs[4] = {-8, -8, -8, -8};
		
		// using tempCounters as offsets 
		obj->tempCounter  = xDiffs[static_cast<int>(sweatEntity->currentDir)];
		//obj->tempCounter2 = yDiffs[static_cast<int>(player->currentDir)];
		obj->tempCounter2 = -8;
		
		if(sweatEntity->currentDir == Direction::Up || sweatEntity->currentDir == Direction::Right) {
			obj->sprite.spritePointer.set_horizontal_flip(true);
		}
		
		obj->sprite.spritePointer.set_x(obj->tempCounter + sweatEntity->sprite.spritePointer.x());
		obj->sprite.spritePointer.set_y(obj->tempCounter2 + sweatEntity->sprite.spritePointer.y());
		obj->sprite.spritePointer.set_z_order(-2);
	};
	
	auto tickFunc = [sweatEntity](Effect* obj) mutable -> bool {
	
		if(frame % 2 == 0) {
			return false;
		}
		
		obj->graphicsIndex++;
		
		// this should get destroyed once the new level loads, and it being here means we are dead, so like, its only going to return false
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_sweat,
			obj->graphicsIndex % 5
		);
		
		obj->sprite.spritePointer.set_x(obj->tempCounter + sweatEntity->sprite.spritePointer.x());
		obj->sprite.spritePointer.set_y(obj->tempCounter2 + sweatEntity->sprite.spritePointer.y());
		return false;
	};
	
	// i rlly should just have createeffect return the (curse)ing effect ptr 
	// also i never use animationfreq even when i should
	Effect* e = new Effect(createFunc, tickFunc);
	effectList.push_back(e);
	return e;	

}

Effect* EffectsManager::generateDialogueEndpointer() {
	
	auto createFunc = [](Effect* obj) mutable -> void {
		
		obj->graphicsIndex = 0;
		
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_textbox_endpointer,
			obj->graphicsIndex
		);
		
		// effects automatically makes the sprite visible, but we dont want 
		// this sprite to be visible initially. 
		// this will let the dialogue thingy cover it up, and we can change it at will.
		obj->sprite.spritePointer.set_bg_priority(3); 
		
		// tempCounter is 
		
		obj->sprite.spritePointer.set_x(108);
		obj->sprite.spritePointer.set_y(68);
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		if(frame % 5 != 0) {
			return false;
		}
		
		obj->graphicsIndex++;
	
		obj->sprite.spritePointer.set_tiles(
			bn::sprite_tiles_items::dw_spr_textbox_endpointer,
			obj->graphicsIndex % 6
		);
		
		
		return false;
	};
	
	Effect* e = new Effect(createFunc, tickFunc);
	//effectList.push_back(e);
	return e;
}

void EffectsManager::questionMark() {
	
	// spr_question_black
	// 9 frames
	
	// will this get properly reset on load?
	//static int questionMarkCount = 0;
	
	if(questionMarkCount > 0) {
		return;
	}
	
	Pos playerPos = entityManager->player->p;
	
	auto createFunc = [playerPos](Effect* obj) mutable -> void {
		
		obj->graphicsIndex = 0;
		
		obj->tiles = &bn::sprite_tiles_items::dw_spr_question_black;
		
		obj->x = playerPos.x * 16;
		obj->y = playerPos.y * 16;
		obj->y -= 8;
		//obj->sprite.updateRawPosition(obj->x, obj->y-16);
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
		obj->sprite.spritePointer.set_z_order(-2);
		
		obj->tempCounter = 1; // make sure first run goes through
		
		obj->graphicsIndex = -1;
		obj->animateFunc(obj);
	};
	
	auto tickFunc = [this](Effect* obj) mutable -> bool {
	
		if(frame % 2 != 0 && obj->tempCounter == 0) {
			return false;
		}
		
		obj->tempCounter = 0;
		obj->graphicsIndex++;
		
		if(obj->graphicsIndex >= 9) {
			obj->tempCounter2++;
			
			if(obj->tempCounter2 == 10) {
				questionMarkCount--;
				return true;
			}
			
			return false;
		}
		
		obj->y--;
		
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};
	
	questionMarkCount++;
	Effect* e = new Effect(createFunc, tickFunc);
	effectList.push_back(e);
	
}

void EffectsManager::treeLeaves() {
	
	// spr_leaves
	// 8 frames 
	// tree base is Pos(6, 4)
	
	auto createFunc = [](Effect* obj) mutable -> void {
		
		obj->tiles = &bn::sprite_tiles_items::dw_spr_leaves;
		
		obj->tempCounter = 1;
		obj->x = randomGenerator.get_int(5 * 16, 7 * 16);
		obj->y = 3 * 16 + 8;
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
		obj->sprite.spritePointer.set_z_order(-2);

		obj->graphicsIndex = -1;
		obj->animateFunc(obj);
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		// maybe,, calling animate from create is a bad idea tbh
		// also the frame modulo trick is kinda (curse), bc the frame a anim is called like, changes 
		// i should of had a framestart thing in effects, but going back to change that all is more time than its worth
		if(frame % 8 == 0 || obj->tempCounter == 1) { 
			obj->graphicsIndex++;
		}
		obj->tempCounter = 0;

		if(obj->graphicsIndex == 8) {
			return true;
		}

		obj->sprite.spritePointer.set_y(obj->sprite.spritePointer.y() + 0.25);
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};
	
	Effect* e = new Effect(createFunc, tickFunc);
	effectList.push_back(e);
	
}

void EffectsManager::chestBonus(Chest* chest) {
	
	
	// hide the original chest sprite, 
	// spr_chest_regular_flash
	// some explosions (i have a func for that already right?)
	
	
	auto createFunc = [chest](Effect* obj) mutable -> void {
		
		chest->sprite.setVisible(false);
		
		bn::sound_items::snd_resurrect.play();
			
		obj->tiles = &bn::sprite_tiles_items::dw_spr_chest_regular_flash;
		
		obj->graphicsIndex = 0;
		
		obj->sprite.updatePosition(chest->p);
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);
	};
	
	auto tickFunc = [chest](Effect* obj) mutable -> bool {
	
		if(frame % 2 != 0) {
			return false;
		}
		
		if(obj->tempCounter % 8 == 0) {
			globalGame->effectsManager.explosion(chest->p);
		}
		
		obj->tempCounter++;

		if(obj->tempCounter == 16) {
			chest->sprite.setVisible(true);
			return true;
		}
		
		
		
		obj->graphicsIndex++;

		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex % 3
		);		
		
		return false;
	};
	
	Effect* e = new Effect(createFunc, tickFunc);
	effectList.push_back(e);
	
	
	
}

Effect* EffectsManager::levStatueActive(LevStatue* levStatue) {
	
	
	auto createFunc = [levStatue](Effect* obj) mutable -> void {
		
		// honestly,,, could/should of just used sprites for everything instead of tiles,,, maybe?
		// im only using tiles bc i figured out their whack ass syntax first
		obj->tiles = &bn::sprite_tiles_items::dw_spr_watcher_eyes_glow; // 9 frames
		
		obj->graphicsIndex = 0;
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex
		);
		
		obj->sprite.updatePosition(levStatue->p);
		
		obj->sprite.spritePointer.set_z_order(-2);
	};
	
	auto tickFunc = [levStatue](Effect* obj) mutable -> bool {
	
		if(frame % 2 == 0) {
			return false;
		}
		
		static bool shouldBlink = false;
		//static int blinkIndex = 0;
		// unlike normal, a static var would actually work/be what i want here. 
		// it will however, have the intresting consequence of blinking more frequently the move lev statues there are, but it be like that 
		if(!shouldBlink && (bruhRand() & 0xFF) == 0) {
			shouldBlink = true;
		}
		
		if(shouldBlink) {
			obj->graphicsIndex++;
		} else {
			obj->graphicsIndex = 0;
		}
		
		if(obj->graphicsIndex >= 9) {
			shouldBlink = false;
		}
		
		if(shouldBlink) {
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				obj->graphicsIndex % 9
			);
		} else {
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				0
			);
		}
		
		obj->sprite.updatePosition(levStatue->p);
		
		
		return false;
	};

	Effect* e = new Effect(createFunc, tickFunc);
	effectList.push_back(e);
	return e;	
	
}

void EffectsManager::levKill() {
	
	/*
	
	all statues flash 
	
	screen blacks 
	
	lightning 
	
	screen unblacks (all lev statues should not be in the flashed state) (which is (curse)ing great for not having to have a one off) 
	
	play player death anim
	
	hand off to reset 
	
	the last two we already have a func for. this is going to be easy
	
	*/
	
	game->cutsceneManager.backupLayer(3);
	
	entityManager->shouldTickPlayer = false;

	unsigned startFrame = frame;
	
	bn::sound_items::snd_lockdamage.play();
	
	while(frame - startFrame < 60) {
	
		game->doButanoUpdate(); // is it ok to do this up here, instead of bottom of loop?
	
		if(frame % 2 != 0) {
			continue;
		}
		
		// how will this perform if kicking a lev statue off?
		// should this technically be a cutscene? i could either add an assert as the start to ensure that haskills.size == 0, or i could just have this func be blocking .
		// same with how i should of done mon statue!
		for(auto it = globalGame->entityManager.obstacleList.begin(); it != globalGame->entityManager.obstacleList.end(); ++it) {
			if(*it == NULL) {
				continue;
			}
			
			if((*it)->entityType() == EntityType::LevStatue) {
				
				(*it)->animationIndex = ((*it)->animationIndex + 1) % 3;
				(*it)->sprite.spritePointer.set_tiles(
					bn::sprite_tiles_items::dw_spr_watcher,
					(*it)->animationIndex
				);
			}
		}
	}
	
	startFrame = frame;
	
	entityManager->player->sprite.spritePointer.set_bg_priority(0);
	
	effectsLayer.black();
	effectsLayer.reloadCells();
	
	// how will this perform if kicking a lev statue off?
		// should this technically be a cutscene? i could either add an assert as the start to ensure that haskills.size == 0, or i could just have this func be blocking .
		// same with how i should of done mon statue!
	for(auto it = globalGame->entityManager.obstacleList.begin(); it != globalGame->entityManager.obstacleList.end(); ++it) {
		if(*it == NULL) {
			continue;
		}
		
		if((*it)->entityType() == EntityType::LevStatue) {
			(*it)->sprite.spritePointer.set_tiles(
				bn::sprite_tiles_items::dw_spr_watcher,
				0
			);
		}
	}
	
	// spr_judgment
	// 5 frames. 
	// actually, its spr_judgment_dark
	// this is an example, off,, a time i should change up bigsprite.
	// but tbh, using a bg layer has been working well for me
	
	const bn::regular_bg_item* judgementBackgrounds[5] = {&bn::regular_bg_items::dw_spr_judgment_dark_index0, &bn::regular_bg_items::dw_spr_judgment_dark_index1, &bn::regular_bg_items::dw_spr_judgment_dark_index2, &bn::regular_bg_items::dw_spr_judgment_dark_index3, &bn::regular_bg_items::dw_spr_judgment_dark_index4};
	
	game->cutsceneManager.delay(15);
	
	Pos playerPos = entityManager->player->p;
	
	bn::sound_items::snd_judgment.play();
	
	for(int i=0; i<5; i++) {
		game->cutsceneManager.cutsceneLayer.rawMap.create(*judgementBackgrounds[i]);
		
		game->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_x(playerPos.x * 16 + 8 - 8);
		game->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_y(playerPos.y * 16 + 8 - 128 + 256);
		game->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_y(256-48-16+8+playerPos.y * 16); // i rlly should understand bg offsets by now
		
		// why do i not make this func global omfg 
		game->cutsceneManager.delay(4);
	}
	
	// i PRAY this works
	game->cutsceneManager.restoreLayer(3);
	
	game->cutsceneManager.delay(15);
	
	effectsLayer.clear();
	effectsLayer.reloadCells();
	
	entityManager->player->sprite.spritePointer.set_bg_priority(1);
	
	entityKill(entityManager->player);
	
	
}

void EffectsManager::fadeBrand() {
	
	
	// cutsceneManager->cutsceneLayer.rawMap.bgPointer.set_palette(game->pal->getBlackBGPalette());
	
	
	auto createFunc = [](Effect* obj) mutable -> void {
		
		// goofy af.
		obj->sprite.updateRawPosition(-32, -32);
		
		obj->tempCounter = 0;
		
		
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		obj->tempCounter++;
		
		constexpr int factor = 8;
		constexpr int offset = 8;
		
		if(obj->tempCounter == offset + factor * 1) {
			globalGame->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(globalGame->pal->getBGPaletteFade(1, false));	
		}
	
	
		if(obj->tempCounter == offset + factor * 2) {
			globalGame->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(globalGame->pal->getBGPaletteFade(2, false));	
		}
		
		if(obj->tempCounter == offset + factor * 3) {
			globalGame->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(globalGame->pal->getBGPaletteFade(3, false));	
		}
		
		if(obj->tempCounter == offset + factor * 4) {
			globalGame->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(globalGame->pal->getBGPaletteFade(4, false));
		}
		
		if(obj->tempCounter == offset + factor * 5) {
			globalGame->cutsceneManager.cutsceneLayer.rawMap.bgPointer.set_palette(globalGame->pal->getBGPalette());
			return true;
		}
	
		
		return false;
	};
	
	Effect* e = new Effect(createFunc, tickFunc);
	effectList.push_back(e);
	
	
	
}
	
void EffectsManager::glassShineSpark(const Pos& p) {
	
	
	auto getCreateFunc = [](int x, int y) -> auto {
		return [x, y](Effect* obj) mutable -> void {
			
			obj->tiles = &bn::sprite_tiles_items::dw_spr_glassshinespark;
			
			obj->tempCounter = 1;
			obj->x = x;
			obj->y = y;
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			obj->sprite.spritePointer.set_z_order(-2);

			obj->graphicsIndex = -1;
			obj->animateFunc(obj);
		};
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		if(frame % 4 == 0 || obj->tempCounter == 1) { 
			obj->graphicsIndex++;
		}
		obj->tempCounter = 0;

		if(obj->graphicsIndex == 10) {
			return true;
		}
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};
	
	int x = p.x * 16;
	int y = p.y * 16;
	
	Effect* e1 = new Effect(getCreateFunc(x-1, y-1), tickFunc);
	Effect* e2 = new Effect(getCreateFunc(x+10, y+10), tickFunc);
	
	effectList.push_back(e1);
	effectList.push_back(e2);
	
}

void EffectsManager::switchGlow(const Pos& p) {
	
	auto getCreateFunc = [](int x, int y) -> auto {
		return [x, y](Effect* obj) mutable -> void {
			
			obj->tiles = &bn::sprite_tiles_items::dw_spr_floorswitch_glow3;
			
			obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_floorswitch_glow3, bn::sprite_shape_size(32, 32));
			
			obj->tempCounter = 1;
			obj->x = x;
			obj->y = y;
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			obj->sprite.spritePointer.set_z_order(1);

			obj->graphicsIndex = -1;
			obj->animateFunc(obj);
		};
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		if(frame % 4 == 0 || obj->tempCounter == 1) { 
			obj->graphicsIndex++;
		}
		obj->tempCounter = 0;

		if(obj->graphicsIndex == 9) {
			return true;
		}
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};
	
	int x = p.x * 16;
	int y = p.y * 16;
	
	Effect* e1 = new Effect(getCreateFunc(x, y), tickFunc);
	
	effectList.push_back(e1);

}

void EffectsManager::exitGlow(const Pos& p) {
	
	// todo, THIS. WHAT SPRITE(S) EVEN IS IT?
	// spr_exit_target_b2 spr_exit_target ???
	// very weird behaviour. may deserve looking at some gml code 
	// appears at start of level(i think?) if covered.
	// appears if afk for a while, and they stays until player input.
	// has a very fancy,, dissapear anim? not sure if ill put that it
	// use floor 65 (rm_0066) for testing	
	
	// spawns on level start, and when inactive for,,, 10secs?
	
	// dw_spr_exit_target_b seems to be one of the sprites,,,, whats the other?
	// dw_spr_exit_target_arrow,, seems to be like,, one frame of it?? forthe circle // INDEX 5
	// as for the dissapear animation,,, idrk 
	// this func took me a pathetic amount of time. i probably didnt eat enough today.
	
	if(exitGlowCount != 0) {
		return;
	}
	
	int x = p.x * 16;
	int y = p.y * 16;
	
	int initFrame = playerIdleFrame;
	
	// is passing this vs passing a ref to the exitGlowCount better?
	auto getTickFunc = [this, x, y, initFrame](const bool flip, bool isDot) -> auto { // the axis in this case, is what diag axis tomove along
		
		int offsetVal = 8;
		
		// i can safely say that the stress is getting to me. what is this
		
		int xStart = x + (flip ? -offsetVal : offsetVal);
		int tempVal = (flip ? -offsetVal : offsetVal);
		int yStart = y + (isDot ? -tempVal : tempVal);
		
		int paletteIndex = 0; // should captures like these be replacements for the tempcounter?
		int shouldDisappear = 0;
		int startOffset = 80+16; // like,, i was going to try fitting this as a temp counter, but it being here honestly is just better
		return [this, xStart, yStart, flip, isDot, paletteIndex, initFrame, shouldDisappear, startOffset](Effect* obj) mutable -> bool {
			
			// THIS SHOULD TOGGLE WHEN DOTICK IS CALLED. TODO, INTEGRATE THAT
			
			if(frame % 12 == 0) {
				paletteIndex++;
				switch(paletteIndex & 0b11) { // i have had,,,, weird interactions when putting paletes inside of arrays. additionally, this will let the palette change even when the user swaps it
					default:
					case 0:
						obj->sprite.spritePointer.set_palette(globalGame->pal->getWhiteSpritePalette());
						break;
					case 1:
						obj->sprite.spritePointer.set_palette(globalGame->pal->getLightGraySpritePalette());
						break;
					case 2:
						obj->sprite.spritePointer.set_palette(globalGame->pal->getDarkGraySpritePalette());
						break;
					case 3:
						obj->sprite.spritePointer.set_palette(globalGame->pal->getLightGraySpritePalette());
						break;
				}
			}
			
			if(playerIdleFrame != initFrame && !shouldDisappear) {
				// todo, possible disentagrate anim
				shouldDisappear = 1;
			}
			
			if(shouldDisappear) {
				
				if(frame % 2 != 0) {
					return false;
				}
				
				shouldDisappear++;
				
				if(shouldDisappear >= 4) {
					exitGlowCount--;
					return true;
				}
				
				obj->graphicsIndex++;
				
				if(obj->graphicsIndex == obj->tiles->graphics_count()) {
					exitGlowCount--;
					return true;
				}
			
				obj->sprite.spritePointer.set_tiles(
					*obj->tiles,
					obj->graphicsIndex
				);
				
			}
		
			if(frame % 8 == 0) {
				obj->tempCounter2 += obj->tempCounter ? 1 : -1;
			
				if(ABS(obj->tempCounter2) >= 3) {
					obj->tempCounter = !obj->tempCounter;
				}
			}
			
			int tempX = xStart + (true == obj->tempCounter);
			int tempY = yStart + (flip == obj->tempCounter);
			
			if(startOffset) {
				tempX += (flip) ? -startOffset : startOffset;
				tempY += (flip == isDot) ? startOffset : -startOffset;
				startOffset-=4;
			}
			
			tempX = CLAMP(tempX, -32, 240 + 32);
			tempY = CLAMP(tempY, -32, 160 + 32);
			
			obj->sprite.updateRawPosition(tempX, tempY);
		
			return false;
		};
	};
		
	auto generateEffect = [this, getTickFunc](const bool flip, const bn::sprite_tiles_item* tiles) -> void {
		exitGlowCount++;
		
		auto dotCreateFunc = [flip, tiles](Effect* obj) mutable -> void {
			
			//obj->tiles = &bn::sprite_tiles_items::dw_spr_exit_target_arrow;
		
			obj->tiles = tiles;
		
			//obj->x = x + (flip ? -12 : 12);
			//obj->y = y + (flip ? 12 : -12);
			
			obj->x = -32;
			obj->y = -32;
				
			obj->sprite.updateRawPosition(obj->x + !flip, obj->y + flip);
			
			obj->sprite.spritePointer.set_z_order(-2); // -2 SHOULD be low enough
			
			obj->sprite.spritePointer.set_vertical_flip(!flip);
			
			if(tiles == &bn::sprite_tiles_items::dw_spr_exit_target_arrow) {
				obj->sprite.spritePointer.set_horizontal_flip(flip);
				obj->graphicsIndex = 5;
			} else {
				obj->sprite.spritePointer.set_horizontal_flip(!flip);
				obj->graphicsIndex = 0;
			}
		
			obj->sprite.spritePointer.set_palette(globalGame->pal->getWhiteSpritePalette());
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				obj->graphicsIndex
			);
			
			obj->tempCounter = !flip; // being used as a bool(moreso, a direction) for what direction to move in 
			obj->tempCounter2 = 0; // offset to use
		};
		
		createEffect(dotCreateFunc, getTickFunc(flip, tiles == &bn::sprite_tiles_items::dw_spr_exit_target_arrow));
		
	};
	
	generateEffect(false, &bn::sprite_tiles_items::dw_spr_exit_target_b);
	generateEffect(true, &bn::sprite_tiles_items::dw_spr_exit_target_b);
	
	generateEffect(false, &bn::sprite_tiles_items::dw_spr_exit_target_arrow);
	generateEffect(true, &bn::sprite_tiles_items::dw_spr_exit_target_arrow);
	
}

void EffectsManager::copyGlow(const Pos& p) {
	
	
	// spr_add_shock
	// 5 frames
	
	auto getCreateFunc = [](int x, int y) -> auto {
		return [x, y](Effect* obj) mutable -> void {
			
			obj->tiles = &bn::sprite_tiles_items::dw_spr_add_shock;
			
			obj->sprite = Sprite(bn::sprite_tiles_items::dw_spr_add_shock, bn::sprite_shape_size(64, 64));
			
			obj->tempCounter = 1;
			obj->x = x;
			obj->y = y;
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			obj->sprite.spritePointer.set_z_order(1);

			obj->graphicsIndex = -1;
			obj->animateFunc(obj);
		};
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		if(frame % 4 == 0 || obj->tempCounter == 1) { 
			obj->graphicsIndex++;
		}
		obj->tempCounter = 0;

		if(obj->graphicsIndex == 5) {
			return true;
		}
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};
	
	int x = p.x * 16;
	int y = p.y * 16;
	
	Effect* e1 = new Effect(getCreateFunc(x+8, y+8), tickFunc);
	
	effectList.push_back(e1);
	
	
}

void EffectsManager::shadowCreate(const Pos& p) {
	
	// since this sprite is actually a full tile, by making its prio higher(lower), we chillin, mega chillin
	
	//spr_cr_create
	
	auto getCreateFunc = [](int x, int y) -> auto {
		return [x, y](Effect* obj) mutable -> void {
			
			obj->tiles = &bn::sprite_tiles_items::dw_spr_cr_create;
		
			obj->tempCounter = 1;
			obj->x = x;
			obj->y = y;
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			obj->sprite.spritePointer.set_z_order(-2); // -2 SHOULD be low enough

			obj->graphicsIndex = -1;
			obj->animateFunc(obj);
		};
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		if(frame % 4 == 0 || obj->tempCounter == 1) { 
			obj->graphicsIndex++;
		}
		obj->tempCounter = 0;

		if(obj->graphicsIndex == 5) {
			return true;
		}
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};

	int x = p.x * 16;
	int y = p.y * 16;
	
	Effect* e1 = new Effect(getCreateFunc(x, y), tickFunc);
	
	effectList.push_back(e1);

}

void EffectsManager::shadowDeath(Shadow* shadow) {
	

	Pos tempPos = entityManager->playerStart;
	
	if(!entityManager->killAtPos(tempPos)) {
		return;
	}
	
	//shadow->doUpdate();
	for(auto it = entityManager->shadowList.begin(); it != entityManager->shadowList.end(); ++it) {
		(*it)->doUpdate();
	}

	generateSweatEffect(shadow);
	
	delay(60);
	
	entityFall(shadow);
	
}

void EffectsManager::smokeCloud(Pos p, const Direction dir) {
	

	if(!p.moveInvert(dir, true, true)) {
		return;
	}

	auto createFunc = [p, dir](Effect* obj) mutable -> void {
			
		obj->tiles = (dir == Direction::Left || dir == Direction::Right) ? &bn::sprite_tiles_items::dw_spr_smokecloud_h : &bn::sprite_tiles_items::dw_spr_smokecloud_v;
	
		obj->tempCounter = 1;
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
		if(dir == Direction::Right) {
			obj->sprite.spritePointer.set_horizontal_flip(true);
		} else if(dir == Direction::Down) {
			obj->sprite.spritePointer.set_vertical_flip(true);
		}
		
		//obj->sprite.spritePointer.set_z_order(-2); // -2 SHOULD be low enough

		obj->graphicsIndex = -1;
		obj->animateFunc(obj);
	
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
	
		if(frame % 4 == 0 || obj->tempCounter == 1) { 
			obj->graphicsIndex++;
		}
		obj->tempCounter = 0;

		if(obj->graphicsIndex == 8) {
			return true;
		}
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->graphicsIndex 
		);		
		
		return false;
	};

	Effect* e1 = new Effect(createFunc, tickFunc);
	
	effectList.push_back(e1);
	
}

void EffectsManager::deathTile(Pos p) {
	
	// i REALLY dont want to be using a whole ass bg layer for this lightning sprite 
	
	auto createFunc = [p](Effect* obj) mutable -> void {
			
		obj->tiles = &bn::sprite_tiles_items::dw_spr_judgment_small;
	
		obj->tempCounter = 0;
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		obj->sprite.updateRawPosition(-32, -32);
		
	};
	
	bn::vector<Sprite, 8> sprites;
	
	for(int i=0; i<8; i++) {
		sprites.push_back(Sprite(bn::sprite_tiles_items::dw_spr_judgment_small, bn::sprite_shape_size(16, 16)));
		sprites[i].updateRawPosition(16 * p.x, 16 * (p.y - (7-i)));
		sprites[i].spritePointer.set_bg_priority(0);
	}
	
	auto tickFunc = [sprites = bn::move(sprites)](Effect* obj) mutable -> bool {
		
		if(obj->tempCounter == 5) {
			return true;
		}
		
		for(int i=0; i<8; i++) {
			sprites[i].spritePointer.set_tiles(
				bn::sprite_tiles_items::dw_spr_judgment_small,
				i + (obj->tempCounter * 8)
			);
		}
		
		if(frame % 2 == 0) {
			obj->tempCounter++;
		}


		return false;
	};

	Effect* e1 = new Effect(createFunc, tickFunc);
	
	effectList.push_back(e1);
	
	
	
}

void EffectsManager::deathTileAnimate(Pos p) {
	
	// should,,, this also be done for the sword anim??

	
	auto createFunc = [p](Effect* obj) mutable -> void {
			
		obj->tiles = &bn::sprite_tiles_items::dw_spr_deathfloor;
		
		obj->sprite.spritePointer.set_bg_priority(2);
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->tempCounter
		);
		
		obj->tempCounter = 0;
		
		obj->x = p.x * 16;
		obj->y = p.y * 16;
		obj->sprite.updateRawPosition(obj->x, obj->y);
		
	};
	
	auto tickFunc = [](Effect* obj) mutable -> bool {
		
		if(obj->tempCounter == 4) {
			return true;
		}
		
		obj->sprite.spritePointer.set_tiles(
			*obj->tiles,
			obj->tempCounter
		);
		
		if(frame % 16 == 0) {
			obj->tempCounter++;
		}


		return false;
	};

	Effect* e1 = new Effect(createFunc, tickFunc);
	
	effectList.push_back(e1);

}

void EffectsManager::corpseSparks() {
	
	//Pos p = Pos(6, 5);
	
	// spr_soulstar_spark_b
	
	
	auto createEffect = [this](bn::fixed xDir, bn::fixed yDir) -> void {
		
		auto createFunc = [xDir, yDir](Effect* obj) mutable -> void {


			//obj->x = 6 * 16 + (8 * xDir);
			//obj->y = 5 * 16 + (8 * yDir);

			obj->sprite.updateRawPosition(-32, -32);
		
			obj->tiles = &bn::sprite_tiles_items::dw_spr_soulstar_spark_b;

			obj->sprite.spritePointer.set_bg_priority(0);
			
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				obj->graphicsIndex % 3
			);
			
		};
		
		// gods being able to do count = 0,, so many other effects would of been so much more simple
		auto tickFunc = [
		xPos = (bn::fixed)6 * 16 + (8 * xDir),
		yPos = (bn::fixed)5 * 16 + (8 * yDir),
		count = 0, xDir, yDir](Effect* obj) mutable -> bool {
			
			if(count == 8) {
				return true;
			}
			
			count++;
			obj->graphicsIndex++;
			
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				obj->graphicsIndex % 3
			);
			
			xPos += 2*xDir;
			yPos += 2*yDir;
			obj->sprite.spritePointer.set_position(xPos - (224/2)+8+4, yPos - (144/2)+8);
			
			return false;
		};
		
		effectList.push_back(new Effect(createFunc, tickFunc));
	
	};
	
	createEffect(-0.707, 0.707);
	createEffect(-1, 0);
	createEffect(-0.707, -0.707);
	
	createEffect(0.707, 0.707);
	createEffect(1, 0);
	createEffect(0.707, -0.707);
	
	createEffect(0, 1);
	createEffect(0, -1);
	
}

void EffectsManager::corpseFuzz() {
	
	//Pos p = Pos(6, 4);
	
	
	auto createEffect = [](int tileSelector) -> Effect* {
		
		auto createFunc = [tileSelector](Effect* obj) mutable -> void {

			obj->x = 6 * 16;
			obj->y = 4 * 16;
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			//tileSelector = randomGenerator.get_int(0, 5);
				
			const bn::sprite_tiles_item* spriteTiles[5] = {
				&bn::sprite_tiles_items::dw_spr_soulglow_big,
				&bn::sprite_tiles_items::dw_spr_soulglow_bigmed,
				&bn::sprite_tiles_items::dw_spr_soulglow_med,
				&bn::sprite_tiles_items::dw_spr_soulglow_medsma,
				&bn::sprite_tiles_items::dw_spr_soulglow_sma,
			};
			
			const bn::sprite_shape_size spriteShapes[5] = {
				bn::sprite_tiles_items::dw_spr_soulglow_big_shape_size,
				bn::sprite_tiles_items::dw_spr_soulglow_bigmed_shape_size,
				bn::sprite_tiles_items::dw_spr_soulglow_med_shape_size,
				bn::sprite_tiles_items::dw_spr_soulglow_medsma_shape_size,
				bn::sprite_tiles_items::dw_spr_soulglow_sma_shape_size,
			};
		
			obj->tiles = spriteTiles[tileSelector];
			
			obj->sprite.spritePointer.set_tiles(*obj->tiles, spriteShapes[tileSelector]);
		
			obj->sprite.spritePointer.set_bg_priority(3);
			
		};
		
		auto tickFunc = [
			x = (bn::fixed)6 * 16 + (randomGenerator.get_int(0, 16) - 8),
			y = (bn::fixed)4 * 16 + (randomGenerator.get_int(0, 16) - 8),
			image_speed = (bn::fixed)0,
			t = 0,
			y_speedup = 4 + randomGenerator.get_int(2, 6 + 1),
			freezeFrames = randomGenerator.get_int(0, 60 + 1),
			graphicsIndex = (bn::fixed)randomGenerator.get_int(0, 5 + 1),
			amplitude = ((bn::fixed)randomGenerator.get_int(4, 12 + 1)) / 20
		](Effect* obj) mutable -> bool {
			
			if(y < -16) {
				return true;
			}
			
			image_speed += 0.02;
			
			
			
			t = ((t + 1) % 360);
			x = (x + (amplitude * sinTable[t]));
		
			y -= 0.75 * (0.095 * y_speedup);
			x += 0.75 * (0.065 * y_speedup);
		
			BN_ASSERT(obj->tiles != NULL, "dust tileset pointer was null. wtf");
			
			//graphicsIndex += image_speed / 60;
			graphicsIndex += image_speed / 20;
			
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				graphicsIndex.integer() % 5
			);
			
			obj->x = x.integer();
			obj->y = y.integer();
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			return false;
		};
		
		return new Effect(createFunc, tickFunc);
	};
	
	int count = randomGenerator.get_int(1, 3 + 1);
	for(int i=0; i<count; i++) {
		effectList.push_back(createEffect(randomGenerator.get_int(0, 5)));
	}

	
	// have them move upright
	// should be on layer 3 
	//,,, i should REALLLY try to reuse the fuzz code i already have
	
}






