

#include "EffectsManager.h"

#include "Game.h"

Palette* EffectsManager::spritePalette = &defaultPalette;
Game* BigSprite::game = NULL;


BigSprite::BigSprite(const bn::sprite_tiles_item* tiles_, int x_, int y_, int width_, int height_, bool collide) :
	width(width_), height(height_), tiles(tiles_), xPos(x_), yPos(y_) {
	
	// was *16, div two, now just *8
	// is that really ok?
	xPos -= (8 * width);
	yPos -= (8 * height);
	
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
			
			// dont do the sprite if this tile is blank. should really help with not hitting the sprite limit
			for(int j=0; j<4; j++) {
				// quite goofy, but basically the set_tiles func like, does the 16x16 tile math, but we need to do it manually here.
				int offset = 4 * (x + (y * width)) + j;
				
				const uint32_t* tileRef = (tiles->tiles_ref())[offset].data;
				
				for(int i=0; i<8; i++) {
					if(tileRef[i] != 0) {
						goto doBigTile;
					}
				}
			}
			
			continue;
			
			doBigTile:

			Sprite tempSprite = Sprite(*tiles);
			
			tempSprite.updateRawPosition(spriteXPos, spriteYPos);
			
			int tempX = xPos/16 + x;
			int tempY = yPos/16 + y;
			
			if(collide) {
				game->collisionMap[tempX][tempY] = 2;
			} else {
				tempSprite.spritePointer.set_bg_priority(1);
			}
			
			tempSprite.spritePointer.set_tiles(
				*tiles,
				x + (y * width)
			);
			
			sprites.push_back(tempSprite);
		}
	}
		
	
	
}

void BigSprite::updatePalette(Palette* pal) {
	for(int i=0; i<sprites.size(); i++) {
		sprites[i].spritePointer.set_palette(pal->getSpritePalette());
	}
	
	
}

// -----

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

void EffectsManager::doVBlank() {
	
	// eventually, we should call ticks on random misc effects here
	
	if(frame % 4 != 0) {
		return;
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
		bigSprites.push_back(new BigSprite(effects->tiles, effects->x, effects->y, effects->width, effects->height, effects->collide) );
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
		if((*it)->p.y >= 6) {
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

				if(*data == '\r') {
					// bad code 
					data++;
					textOffset++;
					goto drawNextLine;
				}

				for(int i=0; i<textSprites.size(); i++) {
					textSprites[i].set_bg_priority(3);
					textSprites[i].set_visible(false);
					textSprites[i].set_palette(spritePalette->getSpritePalette());
				}

				newText = true;
				textShowIndex = 0;			
			} else { // scroll the existing text
				if(currentlyScrolling) { // we never finished displaying the current text, so do that.
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
	
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			effectsLayer.setBigTile(x, y, 0);
		}
	}
	effectsLayer.update();
	
	// this is trash, and will cause frame drops 
	game->fullTileDraw();
	
	hideForDialogueBox(true);
	
	textSprites.clear();
	game->state = restoreState;
	

}
	
	


