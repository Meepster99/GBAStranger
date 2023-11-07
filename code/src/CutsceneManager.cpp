

#include "EffectsManager.h"

#include "Game.h"

#include "bn_hw_irq.h"

CutsceneManager::CutsceneManager(Game* game_) : game(game_), disTextGenerator(dw_fnt_etext_12_sprite_font) {
	
	maps[0] = &game->collision.rawMap;
	maps[1] = &game->tileManager.floorLayer.rawMap;
	maps[2] = &game->effectsManager.effectsLayer.rawMap;
	maps[3] = &cutsceneLayer.rawMap;
	
	for(int i=0; i<4; i++) {
		//tilesBackup.push_back(maps[i]->bgPointer.tiles());
		mapBackup.push_back(maps[i]->bgPointer.map());
	}
	
}

// -----

void CutsceneManager::doVBlank() {
	for(int i=0; i<vBlankFuncs.size(); i++) {
		vBlankFuncs[i]();
	}
	
	if(effectsManager->dialogueEndPointer != NULL) {
		effectsManager->dialogueEndPointer->animate();
	}
	
}

void CutsceneManager::resetRoom() {
	disOsTextSprites.clear();
	for(int i=0; i<disTextSprites.size(); i++) {
		disTextSprites[i].clear();
	}
	disTextSprites.clear();
	disText.clear();
}

void CutsceneManager::introCutscene() {
	
	
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;
	game->state = GameState::Cutscene;

	backupAllButEffects();

	
	// this should REWALLLLY be passed in,, but im tired ok
	int mode = game->mode;
	bool isSuperRodChest = game->roomManager.roomIndex == 0;
	
	// 2 * mode + isSuperrod
	// below caused vram issues??
	// i rlly need to solve the issues of the double uses for some allocs.	
	// something is hanging onto them. what is it???
	// switch statement also failed?
	
	/*
	const bn::regular_bg_item* map3Items[6] = { // would storing all these in one map help with compression?
		&bn::regular_bg_items::dw_spr_vd_gray_index0,
		&bn::regular_bg_items::dw_spr_vd_gray_index1,
		&bn::regular_bg_items::dw_spr_vd_gray_index2,
		&bn::regular_bg_items::dw_spr_vd_gray_index3,
		&bn::regular_bg_items::dw_spr_vd_gray_index4,
		&bn::regular_bg_items::dw_spr_vd_gray_index5
	};
	*/
	
	/*
	
	spr_vd_gray_face
	spr_vd_lillie_face
	spr_vd_cif_face
	
	*/
	
	const bn::sprite_tiles_item* faceItem = &bn::sprite_tiles_items::dw_spr_vd_gray_face;
	auto spriteShape = bn::sprite_shape_size(64, 32);
	int xPos = 76;
	int yPos = 41;
	if(mode == 1) {
		faceItem = &bn::sprite_tiles_items::dw_spr_vd_lillie_face;
		spriteShape = bn::sprite_shape_size(64, 64);
		xPos -= 6;
		yPos += 9;
	} else if(mode == 2) {
		faceItem = &bn::sprite_tiles_items::dw_spr_vd_cif_face;
		spriteShape = bn::sprite_shape_size(32, 32);
		xPos += 41;
		yPos -= 1;
	}
	
	Sprite face(*faceItem, spriteShape);
	face.updateRawPosition(xPos, yPos);
	
	maps[0]->create(bn::regular_bg_items::dw_spr_vd_bg_index0, 3);
	maps[1]->create(bn::regular_bg_items::dw_spr_vd_bg2_index0, 2);
	//maps[3]->create(*map3Items[2 * mode + isSuperRodChest], 3);
	//maps[3]->create(*map3Items[2], 3);

	int temp = 2 * isSuperRodChest + mode;
	if(mode == 2) {
		temp = 4 + isSuperRodChest;
	}
	
	if(temp == 0) { 
		maps[3]->create(bn::regular_bg_items::dw_spr_vd_gray_index0, 1);
	} else if(temp == 1) {
		maps[3]->create(bn::regular_bg_items::dw_spr_vd_gray_index1, 1);
	} else if(temp == 2) {
		maps[3]->create(bn::regular_bg_items::dw_spr_vd_gray_index2, 1);
	} else if(temp == 3) {
		maps[3]->create(bn::regular_bg_items::dw_spr_vd_gray_index3, 1);
	} else if(temp == 4) {
		maps[3]->create(bn::regular_bg_items::dw_spr_vd_gray_index4, 1);
	} else if(temp == 5) {
		maps[3]->create(bn::regular_bg_items::dw_spr_vd_gray_index5, 1);
	} else {
		
	}
	
	maps[1]->bgPointer.set_y(maps[1]->bgPointer.y() + 16 + 32);
	//maps[1]->reloadCells();
	
		
	game->effectsManager.hideForDialogueBox(false, true);	
	
	maps[0]->bgPointer.set_y(maps[0]->bgPointer.y() - 32);
	maps[1]->bgPointer.set_y(maps[1]->bgPointer.y() - 32);
	maps[3]->bgPointer.set_y(maps[3]->bgPointer.y() - 32);
	yPos -= 32;
	face.updateRawPosition(xPos, yPos);
	
	
	for(int i=0; i<32; i++) {
		
		maps[0]->bgPointer.set_y(maps[0]->bgPointer.y() + 1);
		maps[1]->bgPointer.set_y(maps[1]->bgPointer.y() + 1);
		maps[3]->bgPointer.set_y(maps[3]->bgPointer.y() + 1);
		yPos++;
		face.updateRawPosition(xPos, yPos);
		bn::core::update();
		bn::core::update();
		
	}

	
	game->effectsManager.doDialogue("[You aquired a strange rod]\n[Simply holding it makes you feel uneasy]\n[You begin to imagine your whole life existing on a 2001 handheld]\n[Something is completely fucked]\0", true);
	game->playSound(&bn::sound_items::void_stranger_ost_56);
	
	// its insane that i never overloaded the set_tiles func
	for(int i=1; i<=3; i++) {
		face.spritePointer.set_tiles(*faceItem, i);
		bn::core::update();
		bn::core::update();
		bn::core::update();
		bn::core::update();
		bn::core::update();
		bn::core::update();
	}
	
	for(int j=0; j<32; j++) {
		bn::core::update();
	}
	
	for(int i=0; i<=3; i++) {
	
		/*
		maps[0]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		maps[1]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		maps[3]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		
		face.spritePointer.set_palette(game->pal->getSpritePaletteFade(i));
		*/
		
		
		maps[0]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		maps[1]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		maps[3]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		
		face.spritePointer.set_palette(game->pal->getSpritePaletteFade(i));
		
		for(int j=0; j<16; j++) {
			bn::core::update();
		}
		
	}
	

	
	
	// todo, eyebrows!(as sprites, i think is the way heree
	
	for(int i=0; i<98 - (6 * 3) - (16 * 4) - 32; i++) {
		//maps[3]->bgPointer.set_y(maps[3]->bgPointer.y() + 1);
		bn::core::update();
	}
	
	restoreAllButEffects();
	
	game->effectsManager.hideForDialogueBox(true, true);
	face.setVisible(false);
	bn::core::update();
	
	
	game->state = restoreState;

	
}

void CutsceneManager::cifDream() {
	
	
	// most assets are found by searching for "cdream" or "cifdream"
	// song is msc_cif_regret
	// for the glowys, just search glow / soulglow
	// actually, i think these might not be the right glows
	// unless palette changes are being used on some?
	
	/*
	while spr_cdream_add_eus_b could/should/would be a bigsprite 
	
	i dont want to fucking do that
	so going to make it 2 64x64 sprites, having to custom do that in the formatter 
	bc butano wont load shit is,, annoying
	
	or,,, i could do it with a bg item?? but like, should i? it seems inefficent and stupid 
	
	
	
	*/
	
	// put the want to rest dialogue up here 
	
	
	//while(EffectsManager::zoomEffect(bool inward) {
	
	//game->effectsManager.doDialogue("still programming this in its actually pretty close to being finished, but just in case of crashes i commented it out.\0");
	
	//return;
	
	BN_LOG("cifdream");
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;


	if(game->effectsManager.bigSprites[0]->animationIndex != 0) {
		game->effectsManager.doDialogue("[This Lotus-Eater Machine doesn't seem to be operational]\n[Although in the bark's reflection, you dont seem to be either]\n[Better move on]\0", false);
		// in the glistening of the bark, you seem drained as well
		
		return;
	}
	
	
	
	
	game->effectsManager.doDialogue(""
	"[This Lotus-Eater Machine is still operational]\n"
	"[Maybe you could take a quick rest?]\n"
	"\0", false);
	
	if(!game->effectsManager.restRequest()) {
		return;
	}
	
	
	
	game->state = GameState::Dialogue;
	
	delay(60);
	
	
	while(!game->effectsManager.zoomEffect(true, false)) {
		//delay(1);
		delay(20);
		//BN_LOG("HEY FOOL, CHANGE THIS DELAY BACK TO 20");
	}
	
	game->state = GameState::Cutscene;
	
	game->effectsManager.hideForDialogueBox(false, true);
	backupAllButEffects();

	
	
	/*
	
	//screen is still black
	
	Wake up.
	
	// queue actual scene, THERE IS A VERY BRIEF FADEIN, is it the
	// reverse of the fade we already have?
	// sorta,, except this one fades from black 
	// might not put that in 
	
	// wait a a few(5) seconds
	
	What a little miracle you are.
	But no matter how hard I try...
	This is as close as I can get.
	The rest is up to you.
	Soon you'll grow up...
	You must keep working diligently and take good care of your sisters...
	Even when you argue and make mistakes.
	Even if I...
	
	// delay for 4 seconds 
	
	You're my little lightbringer.
	You're my pride.
	And your name shall be...
	
	// CUT, kill the tree
	
	or maybe, INA, as a nice refrence to me
	
	*/
	
	
	/*
	
	starting to actually look through gml has been very helpful
	it made getting the dust outside by the tree much easier, and im going to use it for this aswell
	
	spr_cdream_add_eus_b
	
	cifdream/cdream, check that all out
	
	
	*/
	
	delay(60 * 5);
	
	bn::music_items::cifdream.play();
	
	delay(60 * 5);
	maps[1]->create(bn::regular_bg_items::dw_default_black_bg, 1);
	game->effectsManager.doDialogue("Wake up.\0", true);
	
	maps[1]->bgPointer.set_priority(0);
	effectsManager->effectsLayer.clear();
	
	bn::vector<Effect, 16> effects;
	
	
	//maps[3]->create(bn::regular_bg_items::dw_default_black_bg, 0);
	
	// gml_Object_obj_cifdream_eyecatch_Create_0 contains the spawn indexes. (what is their origin?)
	Sprite mon(bn::sprite_tiles_items::dw_spr_cdream_mon, bn::sprite_shape_size(64, 32), 37  , 107  );
	mon.spritePointer.set_bg_priority(2);
	Sprite bee(bn::sprite_tiles_items::dw_spr_cdream_bee, bn::sprite_shape_size(16, 16), 140 , 32 - 8  );
	bee.spritePointer.set_bg_priority(2);
	Sprite gor(bn::sprite_tiles_items::dw_spr_cdream_gor, bn::sprite_shape_size(64, 32), 148 , 127  );
	gor.spritePointer.set_bg_priority(2);
	Sprite tan(bn::sprite_tiles_items::dw_spr_cdream_tan, bn::sprite_shape_size(32, 32), 134 , 96   );
	tan.spritePointer.set_bg_priority(2);
	Sprite lev(bn::sprite_tiles_items::dw_spr_cdream_lev, bn::sprite_shape_size(32, 32), 32  , 92   );
	lev.spritePointer.set_bg_priority(2);
	Sprite cif(bn::sprite_tiles_items::dw_spr_cdream_cif, bn::sprite_shape_size(16, 16), 119 + 2 , 37 - 6   );
	cif.spritePointer.set_bg_priority(2);
	
	Sprite cifGlow(bn::sprite_tiles_items::dw_spr_soulglow_big, bn::sprite_shape_size(32, 32), 119 + 2 - 6 - 1, 37 - 2 );
	cifGlow.spritePointer.set_z_order(1);
	cifGlow.spritePointer.set_bg_priority(2);
	
	
	// the more that i think of the positions of the lords, the more symbolism i see. its insane. i love this game sm
	
	maps[0]->create(bn::regular_bg_items::dw_spr_cdream_add_eus_b_index0, 2);
	maps[0]->bgPointer.set_x(74 + (64 / 2) - 8);
	maps[0]->bgPointer.set_y(10 + (132 / 2) - 16);
	
	// ugh.
	// the fade in with cif having 2 light levels is, an inconvience. i should just make a palete class, but i would have to rewrite the sprite class as well 
	// and chnge a ton of shit. this is the best i can do
	//bn::blending::set_transparency_alpha(0.75);
	
	auto bgPalette = globalGame->pal->getBGPalette();
	auto spritePalette = globalGame->pal->getSpritePalette();
	auto darkSpritePalette = globalGame->pal->getDarkSpritePalette();
	
	maps[0]->bgPointer.set_palette(bgPalette.create_palette());
	cif.spritePointer.set_palette(darkSpritePalette.create_palette());
	game->doButanoUpdate();
	cif.spritePointer.set_palette(spritePalette.create_palette());
	
	vBlankFuncs.push_back([this, cif, cifGlow, bgPalette]() mutable {
		const bn::regular_bg_item* addBackgrounds[4] = {&bn::regular_bg_items::dw_spr_cdream_add_eus_b_index0, &bn::regular_bg_items::dw_spr_cdream_add_eus_b_index1, &bn::regular_bg_items::dw_spr_cdream_add_eus_b_index2, &bn::regular_bg_items::dw_spr_cdream_add_eus_b_index3};
		static int addBackgroundsIndex = 0;
		static int x = 74 + (64 / 2) - 8;
		static bn::fixed y = 10 + (132 / 2) - 16;
		const static bn::fixed yStart = 10 + (132 / 2) - 16;
		//static int degree = 0;
		
		// should it be 18 or 11 here?
		// actually, that vid was 30fps i think,,,,,,,soooo,,
		
		if(frame % 20 == 0) {
			maps[0]->create(*addBackgrounds[addBackgroundsIndex], 1);
			addBackgroundsIndex = (addBackgroundsIndex + 1) % 4;
			maps[0]->bgPointer.set_x(x);
			maps[0]->bgPointer.set_y(y);
		}
		
		static int cifGlowIndex = 0; // sprite class should rlly have a thing which lets me access its graphicsindex
		if(frame % 10 == 0) {
			cifGlow.spritePointer.set_tiles(bn::sprite_tiles_items::dw_spr_soulglow_big, cifGlowIndex % 5);
			cifGlowIndex++;
		}
		
		// just look at gml_Object_obj_cifdream_eyecatch_Step_0 next time fool
		
		static bool floatDir = false;
		static int freezeFrames = 0;
		
		if(freezeFrames) {
			freezeFrames--;
			return;
		}
		
		//bn::fixed temp = y + 0.15 * sinTable[degree % 360];
		
		bn::fixed temp = y + (floatDir ?  0.1 : -0.1);
		
		bn::fixed tempDif = temp - yStart;
		if(tempDif > -2.0 && tempDif < 2.0) {
			y = temp;
			cif.setRawY(37 - 6 + tempDif);
			cifGlow.setRawY(37 - 2 + tempDif);
		} else {
			floatDir = !floatDir;
			freezeFrames = 26; // setting the alarm to 16 seems to be like,, yea
		}
		
		maps[0]->bgPointer.set_y(y);
		
		return;
	});
	
	// static vars, declared inside the lambda, dont reset on a second func call?? that is quite weird

	auto createEffect = [spritePalette, darkSpritePalette]() -> Effect {
		
		auto createFunc = [spritePalette, darkSpritePalette](Effect* obj) mutable -> void {
			if(randomGenerator.get() & 1) {
				obj->tiles = &bn::sprite_tiles_items::dw_spr_dustparticle;
				obj->sprite.spritePointer.set_palette(darkSpritePalette);
			} else {
				obj->tiles = &bn::sprite_tiles_items::dw_spr_dustparticle2;
				obj->sprite.spritePointer.set_palette(spritePalette);
			}
		
			obj->sprite.spritePointer.set_tiles(
				*obj->tiles,
				0
			);
		
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
			tileSelector = -1,
			spritePalette,
			darkSpritePalette
			](Effect* obj) mutable -> bool {
			
			if(y < -16) {
	
				tileSelector = randomGenerator.get_int(0, 5);
				
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
			
				obj->sprite.spritePointer.set_bg_priority(tileSelector == 0 ? 1 : 3);
				//obj->sprite.spritePointer.set_palette(tileSelector == 0 ? globalGame->pal->getSpritePaletteFade(3, false) : globalGame->pal->getSpritePaletteFade(2, false));
				//obj->sprite.spritePointer.set_palette(spritePalette);
			
				obj->sprite.spritePointer.set_palette(tileSelector != 0 ? darkSpritePalette : spritePalette);
			
				//obj->sprite.spritePointer.set_blending_enabled(tileSelector != 0);
					
				
				
				x = randomGenerator.get_int(16 * 14);
				//y = 16*5+randomGenerator.get_int(16);
				if(y == -32) {
					y = randomGenerator.get_int(16 * 9);
				} else {
					y = 16 * 10;
				}
				
				image_speed = (bn::fixed)0;
				y_speedup = 4 + randomGenerator.get_int(2, 6 + 1);
				t = randomGenerator.get_int(0, 180 + 1);
				amplitude = ((bn::fixed)randomGenerator.get_int(4, 12 + 1)) / 40;
				//graphicsIndex = (bn::fixed)0;
				graphicsIndex = (bn::fixed)randomGenerator.get_int(0, 5 + 1);
				freezeFrames = randomGenerator.get_int(0, 60 + 1);
				
				randomGenerator.update();
			}
			
			if(image_speed > 9) {
				freezeFrames = randomGenerator.get_int(0, 60 + 1);		
			}
			

			image_speed += 0.02;
			//image_speed += 0.20;
			
			//y -= (0.1 * y_speedup);
			y -= (0.095 * y_speedup);
			
			t = ((t + 1) % 360);
			x = (x + (amplitude * sinTable[t]));
			
			if(x > 240) {
				x -= 240;
			} else if(x < 0) {
				x += 240;
			}
		
			BN_ASSERT(obj->tiles != NULL, "dust tileset pointer was null. wtf");
			
			graphicsIndex += image_speed / 60;
			if(freezeFrames == 0) {
				obj->sprite.spritePointer.set_tiles(
					*obj->tiles,
					graphicsIndex.integer() % 5
				);
			} else {
				freezeFrames--;
			}
		
			obj->x = x.integer();
			obj->y = y.integer();
			obj->sprite.updateRawPosition(obj->x, obj->y);
			
			return false;
		};
		
		return Effect(createFunc, tickFunc);
	};
	
	while(effects.size() != effects.max_size()) {
		effects.push_back(createEffect());
	}
	
	// init all the effects such that the palete table wont be moving
	for(int i=0; i<effects.size(); i++) {
		effects[i].animate();
	}
			
	vBlankFuncs.push_back([this, effects, spritePalette, darkSpritePalette]() mutable {
		
		for(int i=0; i<effects.size(); i++) {
			effects[i].animate();
		}
		
		
		return;
	});
	
	
	// fade should be done here, i believe
	// ugh.
	// ok so, currently palette stuff in general is so circlejerkish, and spread all over the place .
	// i tried replacing it with another option, but like, ugh 
	// so,,, heres an idea 
	// i take a even stupider hybrid approach.
	// i backup the palette table, and then fade into it afterword.
	
	// i rlly should of just made a class
	
	// in order for us to backup the palette table, butano update has to push it to actual ram 
	// this lets us to a frame update, without flashing shit
	// ok nvm, this just aint going to work. 
	// ugh 
	// 
	
	// FADE OCCURS HERE TO DELAY THE GLOW FROM PASSING THROUGH IT
	game->doButanoUpdate();
	game->fadePalette(0);
	game->doButanoUpdate();
	game->fadePalette(1);
	game->doButanoUpdate();
	maps[1]->bgPointer.set_priority(3);
	delay(5);
	game->fadePalette(2);
	delay(5);
	game->fadePalette(3);
	delay(5);
	game->fadePalette(4);
	delay(5);


	delay(60 * 5);
	
	delay(60 * 1);
	
	
	game->effectsManager.doDialogue(""
	"What a little miracle you are.\n"
	"But no matter how hard I try...\n"
	"This is as close as I can get.\n" // regretably.
	"The rest is up to you.\n"
	"Soon you'll grow up...\n"
	"You must keep working diligently and take good care of your sisters...\n"
	"Even when you argue and make mistakes.\n"
	"Even if I...\0"
	"", true);
	
	
	// delay for 4 seconds 
	delay(60 * 4);
	
	
	game->effectsManager.doDialogue(""
	"You're my little lightbringer.\n"
	"You're my pride.\n"
	"And your name shall be...\0"
	, true);
	
	bn::music::stop();
	
	// hide VRAM bulshittery.
	maps[1]->bgPointer.set_priority(0);
	bn::sprite_text_generator textGenerator(dw_fnt_text_12_sprite_font);
	textGenerator.set_center_alignment();
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> tempSprites;
	textGenerator.generate((bn::fixed)0, (bn::fixed)0, bn::string_view("INA"), tempSprites);
	textGenerator.generate((bn::fixed)0, (bn::fixed)48, bn::string_view("THANK YOU FOR PLAYING<3"), tempSprites);
	bn::blending::set_transparency_alpha(0.25);
	for(int i=0; i<tempSprites.size(); i++) {
		tempSprites[i].set_palette(spritePalette);
		tempSprites[i].set_bg_priority(0);
		tempSprites[i].set_visible(true);
		tempSprites[i].set_blending_enabled(true);
	}
	game->doButanoUpdate();
	
	// CUT, kill the tree
	
	// or maybe, INA, as a nice refrence to me
	
	/*while(true) {
		delay(60*5);
	}*/

	/*
	maps[1]->create(bn::regular_bg_items::dw_spr_cdream_add_eus_b_index1, 3);
	maps[3]->create(bn::regular_bg_items::dw_spr_cdream_add_eus_b_index2, 3);
	//maps[1]->bgPointer.set_y(maps[1]->bgPointer.y() + 16 + 32);
	delay(60*5);
	*/
	
	// 
	
	tempSprites.clear();
	vBlankFuncs.clear();
	effects.clear();
	
	game->effectsManager.bigSprites[0]->animate();
	
	game->effectsManager.hideForDialogueBox(true, true);
	restoreAllButEffects();
	
	game->state = restoreState;
	
}

void CutsceneManager::brandInput() {
	
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;
	game->state = GameState::Cutscene;

	maps[2]->bgPointer.set_priority(3);
	
	backupAllButEffects();
	
	Sprite* brandState[6][6];
	
    for(int i=0; i<6; i++) {
        for (int j=0; j<6; j++) {
            brandState[i][j] = NULL;
        }
    }
	
	maps[1]->create(bn::regular_bg_items::dw_default_black_bg, 3);
	
	// this bg is scaled up ingame. 
	// i should/would/could, use an affine bg, but id rather die than go an interact with more of 
	// the butano backend at this point. 
	// probs just going to scale it up in the formatter.
	// sprites can already scale, so that wont be a problem
	maps[3]->create(bn::regular_bg_items::dw_spr_puumerkki_bigframe_index0, 2);
	// why does setting x to 0 actually set the offset to 8 ingame?
	maps[3]->bgPointer.set_x((bn::fixed)(128/2));
	maps[3]->bgPointer.set_y((bn::fixed)(3*16) + 8);
	
	// lots of assets seem to be under "puumerkki"
	
	Sprite cursor(bn::sprite_tiles_items::dw_spr_puumerkki_kursori, bn::sprite_shape_size(32, 32));
	
	Pos cursorPos(4, 1);
	cursor.spritePointer.set_x(cursorPos.x * 16 - 240/2 + 16 + 1);
	cursor.spritePointer.set_y(cursorPos.y * 16 - 160/2 + 16 + 1);
	cursor.spritePointer.set_bg_priority(0);
	
	//Sprite allDoneSprite(bn::sprite_tiles_items::dw_spr_puumerkki);
	//allDoneSprite.spritePointer.set_tiles(bn::sprite_tiles_items::dw_spr_puumerkki, 1);
	Sprite allDoneSprite(bn::sprite_items::dw_default_sprite_64);
	Sprite allDoneSpriteLeft(bn::sprite_items::dw_default_sprite_64);
	Sprite allDoneSpriteRight(bn::sprite_items::dw_default_sprite_64);
	
	// i did not know i could inline shit like this
	// annoying i couldnt do it with refs tho
	for(Sprite* sprite : {&allDoneSprite, &allDoneSpriteLeft, &allDoneSpriteRight}) {
		sprite->spritePointer.set_palette(game->pal->getLightGraySpritePalette());
		sprite->spritePointer.set_bg_priority(1);
		sprite->spritePointer.set_y(64);
		sprite->spritePointer.set_vertical_scale((bn::fixed)0.25);
		sprite->setVisible(false);
	}
	
	allDoneSpriteLeft.spritePointer.set_horizontal_scale(2);
	allDoneSpriteRight.spritePointer.set_horizontal_scale(2);
	
	//allDoneSprite.spritePointer.set_mosaic_enabled(true);
	
	int allDoneSpriteWidth = 0;
	
	bn::vector<Effect*, 16> effects;
	while(effects.size() != effects.max_size()) {
		effects.push_back(effectsManager->getRoomDustEffect(true));
	}
	
	// init all the effects such that the palete table wont be moving
	for(int i=0; i<effects.size(); i++) {
		effects[i]->animate();
	}
			
	vBlankFuncs.push_back([this, effects]() mutable {
		
		for(int i=0; i<effects.size(); i++) {
			effects[i]->animate();
		}

		return;
	});
	
	bn::sprite_text_generator textGenerator(dw_fnt_text_12_sprite_font);
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> textSprites;
	textGenerator.set_center_alignment();
	textGenerator.generate((bn::fixed)0, (bn::fixed)64, bn::string_view("Engrave your brand.\0"), textSprites);
	for(int i=0; i<textSprites.size(); i++) {
		textSprites[i].set_palette(game->pal->getSpritePalette());
		textSprites[i].set_bg_priority(0);
	}
	
	bn::timer inputTimer;
	
	// yandere dev ass code
	
	retryBrand:
	
	allDoneSprite.setVisible(false);
	allDoneSpriteLeft.setVisible(false);
	allDoneSpriteRight.setVisible(false);
	allDoneSpriteWidth = 0;
	cursor.spritePointer.set_x(cursorPos.x * 16 - 240/2 + 16 + 1);
	cursor.spritePointer.set_y(cursorPos.y * 16 - 160/2 + 16 + 1);
	cursor.setVisible(true);
	
	bool allDone = false;
	while(true) {
		
		if(allDone && bn::keypad::a_held()) {
			allDoneSpriteWidth++;
		} else if(allDoneSpriteWidth > 0) {
			allDoneSpriteWidth--;
		}
			
		if(allDoneSpriteWidth > 0) {
			allDoneSprite.setVisible(true);
			if(allDoneSpriteWidth >= 20) {
				allDoneSpriteLeft.setVisible(true);
				allDoneSpriteRight.setVisible(true);
				int val = 2 *(allDoneSpriteWidth-20);
				allDoneSpriteRight.spritePointer.set_x(val);
				allDoneSpriteLeft.spritePointer.set_x(-val);
			} else {
				allDoneSpriteLeft.setVisible(false);
				allDoneSpriteRight.setVisible(false);
			}
			allDoneSprite.spritePointer.set_horizontal_scale((bn::fixed)allDoneSpriteWidth / 10);		
		} else {
			allDoneSprite.setVisible(false);
			allDoneSpriteLeft.setVisible(false);
			allDoneSpriteRight.setVisible(false);
		}
		
		if(allDoneSpriteWidth >= 50) {
			// kachow;
			break;
		}
			
		
		
		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > FRAMETICKS * 3) {
			inputTimer.restart();
			if(allDone) {
		
				if(bn::keypad::up_pressed()) {
					cursorPos.move(Direction::Up);
					cursor.spritePointer.set_x(cursorPos.x * 16 - 240/2 + 16 + 1);
					cursor.spritePointer.set_y(cursorPos.y * 16 - 160/2 + 16 + 1);
					allDone = false;
					
					textSprites.clear();
					textGenerator.generate((bn::fixed)0, (bn::fixed)64, bn::string_view("Engrave your brand.\0"), textSprites);
					for(int i=0; i<textSprites.size(); i++) {
						textSprites[i].set_palette(game->pal->getSpritePalette());
						textSprites[i].set_bg_priority(0);
					}
					cursor.setVisible(true);
				}
			} else {

				if(bn::keypad::up_pressed()) {
					cursorPos.move(Direction::Up);
				} else if(bn::keypad::down_pressed()) {
					cursorPos.move(Direction::Down);
				} else if(bn::keypad::left_pressed()) {
					cursorPos.move(Direction::Left);
				} else if(bn::keypad::right_pressed()) {
					cursorPos.move(Direction::Right);
				}
				
				if(cursorPos.y == 1+6) {
				
					allDone = true;
					
					textSprites.clear();
					textGenerator.generate((bn::fixed)0, (bn::fixed)64, bn::string_view("All done?\0"), textSprites);
					for(int i=0; i<textSprites.size(); i++) {
						textSprites[i].set_palette(game->pal->getSpritePalette());
						textSprites[i].set_bg_priority(0);
					}
					
					cursor.setVisible(false);
					
					continue;
				}
				
				//cursorPos.x = CLAMP(cursorPos.x, 4, 4+6-1);
				//cursorPos.y = CLAMP(cursorPos.y, 1, 1+6-1);
				if(cursorPos.x < 4) {
					cursorPos.x = 4+6-1;
				} else if(cursorPos.x > 4+6-1) {
					cursorPos.x = 4;
				}
				
				if(cursorPos.y < 1) {
					cursorPos.y = 1+6-1;
				} else if(cursorPos.y > 1+6-1) {
					cursorPos.y = 1;
				}
				
				
				if(bn::keypad::a_pressed()) {
					
					int xIndex = cursorPos.x - 4;
					int yIndex = cursorPos.y - 1;
					
					if(brandState[xIndex][yIndex] != NULL) {
						brandState[xIndex][yIndex]->setVisible(false);
						delete brandState[xIndex][yIndex];
						brandState[xIndex][yIndex] = NULL;
					} else {
						brandState[xIndex][yIndex] = new Sprite(bn::sprite_tiles_items::dw_spr_puumerkki);
						brandState[xIndex][yIndex]->spritePointer.set_tiles(bn::sprite_tiles_items::dw_spr_puumerkki, 1);
						//brandState[xIndex][yIndex]->updatePosition(cursorPos);
						brandState[xIndex][yIndex]->spritePointer.set_x(cursorPos.x * 16 - 240/2 + 16);
						brandState[xIndex][yIndex]->spritePointer.set_y(cursorPos.y * 16 - 160/2 + 16);
						brandState[xIndex][yIndex]->setVisible(true);
					}
				}
				
				cursor.spritePointer.set_x(cursorPos.x * 16 - 240/2 + 16 + 1);
				cursor.spritePointer.set_y(cursorPos.y * 16 - 160/2 + 16 + 1);
				
			}
		}
		
		game->doButanoUpdate();
	}
	
	unsigned tempBrand[6] = {0, 0, 0, 0, 0, 0};
	
	for (int j=0; j<6; j++) {
		unsigned temp = 0;
		for(int i=0; i<6; i++) {
			temp = ((temp << 1) | !!(brandState[i][j] != NULL));
		}
		tempBrand[j] = temp;
	}
	
	if(game->tileManager.checkBrandIndex(tempBrand) != -1) {
		cursorPos.move(Direction::Up);
		goto retryBrand;
	}
	
	textSprites.clear();
	textGenerator.generate((bn::fixed)0, (bn::fixed)64, bn::string_view("So be it!\0"), textSprites);
	for(int i=0; i<textSprites.size(); i++) {
		textSprites[i].set_palette(game->pal->getSpritePalette());
		textSprites[i].set_bg_priority(0);
	}
	
	
	const bn::sprite_palette_ptr tempPalettes[3] = {
	game->pal->getWhiteSpritePalette().create_palette(),
	game->pal->getBlackSpritePalette().create_palette(),
	game->pal->getDarkGraySpritePalette().create_palette()};
	
	
	for(int i=0; i<30; i++) {		

		for(int j=0; j<textSprites.size(); j++) {
			textSprites[j].set_palette(tempPalettes[i % 3]);
		}
		
		for(int j=0; j<2; j++) {
			game->doButanoUpdate();
		}
	}
	
	for(int i=0; i<6; i++) {
		game->tileManager.playerBrand[i] = tempBrand[i];
	}
	
	for(int i=0; i<6; i++) {
		for (int j=0; j<6; j++) {
			if(brandState[i][j] != NULL) {
				delete brandState[i][j];
				brandState[i][j] = NULL;
			}
		}
	}
	
	for(int i=0; i<effects.size(); i++) {
		delete effects[i];
	}

	cursor.setVisible(false);
	
	//allDoneSprite.spritePointer.set_mosaic_enabled(false); // almost 100% def not actually needed, but this project has driven me insane
	// if this flag isnt cleared and then would cause issues in the future like, yea 
	// and i would prefer to not have to reset this flag on every sprite init(altho that wouldnt be to expensive)
	
	restoreAllButEffects();
	maps[2]->bgPointer.set_priority(0);

	game->state = restoreState;
}

void CutsceneManager::createPlayerBrandRoom() {

	cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_spr_confinement_index0, 2);
	cutsceneLayer.rawMap.bgPointer.set_x(16);
	cutsceneLayer.rawMap.bgPointer.set_y(64-8);

	game->collision.rawMap.create(bn::regular_bg_items::dw_spr_confinement_index1, 2);
	game->collision.rawMap.bgPointer.set_y(64-8-16);

	//collision.rawMap.bgPointer.set_visible(false);
	// this seems to bump the prio up even with equal prios?
	game->tileManager.floorLayer.rawMap.bgPointer.set_priority(1);

	auto func1 = [this](void* obj) -> void {
		(void)obj;
		
		effectsManager->doDialogue(""
		"i was going to try writing something meaningful here, but im a bit too tired to do so. "
		"i really should of ate today, or slept, or took care of myself in any form, "
		"but the moment i stop working on this project, the thoughts just come flooding back. "
		"and so i dont stop working.\n"
		
		"i have to wonder, where in my life did i go wrong. I can do all this, program whatever i want, but i cant take basic care of myself.\n"
		"but, regardless of all that. thanks for playing this. i put a lot of effort into it. ik its just an objectively worse version of the actual game, but still, thank you\n"
		"With love,`Inana <3\n"
		"\0");

		return;
	};
	
	// holy shit. 
	// gml_Object_obj_confinement_Step_0, has scr_errormessage(40)
	// in gml_Object_obj_errormessages_Create_0, it says:
	// str_error40 = ">THIS DESECRATION CAN NOT CONTINUE\n"
	// but you never get to see that bc of the popup 
	// that,,, that explains both so much, and nothing 
	// is it grays womb, or add's womb??
	
	// gml_Object_obj_errormessages_Draw_0 for draw code
	
	auto func2 = [this](void* obj) -> bool {
		(void)obj;
		
		static int count = 0;
		count++;
		if(count == 24) {
			globalGame->state = GameState::Paused;
			for(int i=0; i<game->effectsManager.effectList.size(); i++) {
				game->effectsManager.effectList[i]->sprite.setVisible(false);
			}
			restoreAllButEffects();
			game->doButanoUpdate();
			backupAllButEffects();
			game->doButanoUpdate();
			
			
			
			game->effectsManager.hideForDialogueBox(false, true);
			
			
			maps[3]->create(bn::regular_bg_items::dw_default_black_bg, 3);
			//maps[2]->create(bn::regular_bg_items::dw_default_black_bg, 2);
			maps[1]->create(bn::regular_bg_items::dw_spr_un_stare_index0, 0);
			maps[1]->bgPointer.set_y(48 + 16);
			
			restoreLayer(0);
			
			globalGame->collision.rawMap.bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_spr_glitchedsprites);
			globalGame->collision.rawMap.bgPointer.set_priority(1);
			globalGame->collision.rawMap.bgPointer.set_palette(game->pal->getBGPalette());
			
			maps[2]->bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_spr_glitchedsprites);
			maps[2]->bgPointer.set_priority(1);
			
			maps[3]->bgPointer.set_palette(game->pal->getBGPalette());
			maps[2]->bgPointer.set_palette(game->pal->getBGPalette());
			maps[1]->bgPointer.set_palette(game->pal->getBGPalette());
			maps[0]->bgPointer.set_palette(game->pal->getBGPalette());
			
			int n = globalGame->collision.rawMap.bgPointer.tiles().tiles_count();
			
			bn::sound::stop_all();
			
			for(int x=0; x<32; x++) {
				for(int y=0; y<32; y++) {
					globalGame->collision.rawMap.setTile(x, y, randomGenerator.get_int(0, n), randomGenerator.get_int(0, 1), randomGenerator.get_int(0, 1));
					maps[2]->setTile(x, y, randomGenerator.get_int(0, n), randomGenerator.get_int(0, 1), randomGenerator.get_int(0, 1));
				}
			}
		
		
			while(true) {
				//globalGame->playSound(&bn::sound_items::snd_ex_heartbeat_b);
				globalGame->playSound(&bn::sound_items::snd_ex_heartbeat);
				
				for(int i=0; i<20; i++) {
					globalGame->doButanoUpdate();
				}
			}
		}
		
		globalGame->playSound(&bn::sound_items::snd_ex_heartbeat_b);
		globalGame->playSound(&bn::sound_items::snd_ex_heartbeat);
		
		effectsManager->playerBrandRoomBackground();
		
		return true;
	};

	Interactable* temp1 = new Interactable(Pos(6, 4),
		func1,
		func2,
		NULL,
		NULL
	);
	
	Interactable* temp2 = new Interactable(Pos(7, 4),
		func1,
		func2,
		NULL,
		NULL
	);
	
	game->entityManager.addEntity(temp1);
	game->entityManager.addEntity(temp2);
	
	
}

void CutsceneManager::crashGame() {
	
	// totally use some of the hacky shit you learned from writing the cart thing to 
	// corrupt stuff here

	bn::sound::stop_all();
	bn::music::stop();
	
	// mute sound so disabling vblank iqr doesnt fuck me 
	*(reinterpret_cast<unsigned short*>(0x04000080)) = 0;
	*(reinterpret_cast<unsigned short*>(0x04000082)) = 0;
	*(reinterpret_cast<unsigned short*>(0x04000084)) = 0;
	
	for(int i=0; i<5; i++) {
		game->doButanoUpdate();
	}
	//bn::core::set_vblank_callback(doNothing);
	//game->doButanoUpdate();
	bn::hw::irq::disable(bn::hw::irq::id::VBLANK);
	
	//unsigned* bgVram = reinterpret_cast<unsigned*>(0x06000000);
	//unsigned* randomStuff = reinterpret_cast<unsigned*>(0x03000000);
	//memcpy(bgVram, randomStuff, 0x18000/4);
	
	/*
	u8* bgVram = reinterpret_cast<u8*>(0x06000000);
	u8* randomStuff = reinterpret_cast<u8*>(0x03000000);
	
	for(int i=0; i<0x18000; i++) {
		while(*randomStuff == 0) {
			randomStuff++;
		}
		*bgVram = *randomStuff;
		bgVram++;
		randomStuff++;
	}
	*/
	
	/*
	volatile unsigned short* palettePointer = reinterpret_cast<volatile unsigned short*>(0x05000000);
	for(int i=0; i<1024/2; i++) {
		switch(i % 5) {
			case 0:
				if(i % 16 == 0) {
					palettePointer[i] = *col0;
				} else {
					palettePointer[i] = *col1;
				}
				break;
			case 1:
				palettePointer[i] = *col1;
				break;
			case 2:
				palettePointer[i] = *col2;
				break;
			case 3:
				palettePointer[i] = *col3;
				break;
			case 4:
				palettePointer[i] = *col4;
				break;
			default:
				palettePointer[i] = 0;
				break;
		}
	}
	
	unsigned* bgVram = reinterpret_cast<unsigned*>(0x06000000);
	unsigned temp;
	unsigned maxTile = 512;
	unsigned tileIndex;

	for(int i=0; i<0x18000; i++) {
		temp = *bgVram ^ *(bgVram + ((short)bruhRand()));
		
		
		tileIndex = (temp & 0x01FF) % maxTile;
		if(tileIndex == 0) { // one extra attempt
			temp = bruhRand() & 0x0FFF;
			tileIndex = (temp & 0x01FF) % maxTile;
		}
		temp = (temp & ~0x01FF) | tileIndex;
		
		*bgVram = temp;
		bgVram++;
	}
	*/
	
	// having butano still running in here means i can potentially do some,, sound stuff?
	// and program easier
		
	
	
	*(reinterpret_cast<unsigned short*>(0x04000000)) |= 0b0000000010000000;
	
	unsigned short* bg1Control = (reinterpret_cast<unsigned short*>(0x400000A));
	unsigned short* bg2Control = (reinterpret_cast<unsigned short*>(0x400000C));
	
	
	unsigned short bg1CharBlock = (*bg1Control & 0b0000000000001100) >> 2;
	unsigned short bg2CharBlock = (*bg2Control & 0b0000000000001100) >> 2;
	
	unsigned short bg1MapBlock =  (*bg1Control & 0b0001111100000000) >> 8;
	unsigned short bg2MapBlock =  (*bg2Control & 0b0001111100000000) >> 8;
	
	BN_LOG(bg1CharBlock, " ", bg1MapBlock);
	BN_LOG(bg2CharBlock, " ", bg2MapBlock);
	
	volatile unsigned short* mapPtr1 = reinterpret_cast<unsigned short*>(0x06000000 + (2 * bg1MapBlock * 1024));
	volatile unsigned short* mapPtr2 = reinterpret_cast<unsigned short*>(0x06000000 + (2 * bg2MapBlock * 1024));
	unsigned* tilesPtr1 = reinterpret_cast<unsigned*>(0x06000000 + (bg1CharBlock * 16 * 1024));
	unsigned* tilesPtr2 = reinterpret_cast<unsigned*>(0x06000000 + (bg2CharBlock * 16 * 1024));
	
	unsigned short maxTile = *glitchTilesCount/(4*8);
	
	auto getRandomTile = [maxTile]() -> unsigned short {
		
		//retry:
		unsigned short rand = bruhRand();
		unsigned short tileIndex = rand % maxTile;
		
		unsigned short x = tileIndex % 12;
		unsigned short y = tileIndex / 12;
		
		if(x > 8 || y > 8) { // one extra attempt
			//goto retry;
			tileIndex = bruhRand() % maxTile;
		}
		return tileIndex | (rand & 0b0000110000000000);
	};
	
	for(int i=0; i<*stareMapCount/2; i++) {
		// https://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		mapPtr1[i] = (mapPtr1[i] & 0xF000) | getRandomTile();
		mapPtr2[i] = (mapPtr2[i] & 0xF000) | getRandomTile();
	}
	
	for(int i=0; i<*glitchTilesCount/4; i++) {
		tilesPtr1[i] = glitchTiles[i];
		tilesPtr2[i] = glitchTiles[i];
	}

	//unsigned short* bg2YOffset = reinterpret_cast<unsigned short*>(0x0400001A);
	//*bg2YOffset = (*bg2YOffset + 1) & 0x01FF;
	*(reinterpret_cast<volatile unsigned short*>(0x04000000)) &= ~0b0000000010000000;
	
	unsigned short* bg2XOffset = reinterpret_cast<unsigned short*>(0x04000018);
	unsigned short* bg2YOffset = reinterpret_cast<unsigned short*>(0x0400001A);
	unsigned short bg2XOffsetValue = 0;
	unsigned short bg2YOffsetValue = 0; // the pointer is write only. im ashamed at how long this took me

	//volatile unsigned short VCOUNT = 0;
	volatile unsigned short* greenswap = reinterpret_cast<volatile unsigned short*>(0x04000002);
	volatile unsigned short* DISPSTAT  = reinterpret_cast<volatile unsigned short*>(0x04000004);
	unsigned greenFrames = 0;
	bool frameStarted = false;
	while(true) {
		
		//VCOUNT = *(reinterpret_cast<volatile unsigned short*>(0x04000006));
		
		if((*DISPSTAT & 1) == 0 && frameStarted) {
			frameStarted = false;
			if(greenFrames == 0 && (bruhRand() & 0x3FF) == 0) {
				greenFrames = (bruhRand() & 0xF);
			}
		}
		
		if((*DISPSTAT & 1) == 1 && !frameStarted) {
			frameStarted = true;
			frame++;
			
			if(greenFrames > 0) {
				greenFrames--;
				*greenswap = 1;
				bg2XOffsetValue = bruhRand() & 1;
				bg2YOffsetValue = (bg2YOffsetValue - 1) & 0x01FF;
			} else {
				*greenswap = 0;
				bg2XOffsetValue = 0;
				bg2YOffsetValue = 0;
			}
			
			*bg2XOffset = bg2XOffsetValue;
			*bg2YOffset = bg2YOffsetValue;
		}
		
		
		//bn::core::update();
	}

	BN_ERROR("game 'crashed', i havent implimented it yet ");
	
}

void CutsceneManager::carcusEnding() {
	
	for(int i=0; i<60; i++) {
		game->doButanoUpdate();
	}
	
	BN_ERROR("look, idrk if ima make the carcus ending, TODO.");
	
}

void CutsceneManager::disCrash(FloorTile* testTile, bool isPickup) {
	// it is critical that we pass a ref of the pointer to do proper comparisons
	// wait no is it?
	// the bool isPickup isnt neccessary, but will reduce lookups
	
	/*
	
	picking up the ?? from B??? will crash your game @Inana
	picking up VO or ID from VOID will crash you
	picking up INFINITY
	going down stairs with an incomplete BXXX value (no last 2 digits)
	picking up void rod
	
	i could probs,, just like check the state of things when you die and set the count to that 
	for non crash things, crash things occur here.
	and also the anims in the background 
	
	console is printed from as top being the most recent line.
	most recent line is lightgray, following are darkgray
	top line is "DIS OS REPORT 05/11/112023
	// use build date for date
	
	locust counter:
	">ERR: INVALID/MISSING LI VALUE"
	">LI VALUE RESTORED"
	
	locust sprite:
	">ERR: LI NULL"
	">LI RESTORED"

	">ERR: INVALID/MISSING BR VALUE"
	">BR VALUE RESTORED"
	
	">ERR: MEM1 REMOVED"
	">MEM1 RESTORED"
	
	">ERR: MEM2 REMOVED"
	">MEM2 RESTORED"
	
	">ERR: MEM3 REMOVED"
	">MEM3 RESTORED"
	
	hp number removed:
	">ERR: INVALID/MISSING HP VALUE"
	">HP RESTORED"
	
	hp bar removed:
	">FATAL ERROR: HP NULL" // kill player afterword
	
	
	remove first B val, B? B0 B1 B2::
	">FATAL ERROR : BR NULL"
	
	remove rod:
	corrupts screen, also prints something behind it? 
	checking out gml_Object_obj_errormessages_Create_0 and gml_Object_obj_player_Step_0
	maybe gml_Object_obj_floor_hpn_Destroy_0
	
	">FATAL ERR: VR HAS BEEN SEVERED FROM SYSTEM\n"
	
	remove VO or ID:
	">FATAL ERROR: VOID BREACHED SHUTTING SYSTEM DOWN\n"
	
	the font?
	fnt_etext_12
	
	// also, dropoff tiles COVER this text. 
	// that,, may be a bit of an issue
	
	locust tile count valid position is DYNAMIC, TO WHERE THE LOCUST SPRITE IS
	
	gods i spent so much time on having locust/number tiles be able to update whereever they are on the board 
	before realizing how much pain this will be
	
	*/
	
	
	auto swap = [](WordTile*& a, WordTile*& b) {
		WordTile* s;
		s = a;
		a = b;
		b = s;
	};
	
	//FloorTile* bufferedTile = NULL; // tile to check at end of func
	const char* errorLine = NULL;
	Pos tilePos = testTile->tilePos;
	bool isVoided = game->entityManager.player->isVoided;
	bool doCarcusEnding = false;
	bool doCrashGame = false;
	
	// each time a number tile is placed, it needs to,
	// dynamically check the number to the LEFT of it, for what behaviour to do.
	// also, perform swaps in tileManager
	SaneSet<WordTile*, 4> numberTiles;
	numberTiles.insert(tileManager->locustCounterTile);
	numberTiles.insert(tileManager->floorTile2);
	if(!isVoided) {
		numberTiles.insert(tileManager->voidTile2);
	}
	
	if((void*)testTile == (void*)tileManager->memoryTile) {
		errorLine = isPickup ? ">ERR: MEM1 REMOVED" : ">MEM1 RESTORED";
	} else if((void*)testTile == (void*)tileManager->wingsTile) {
		errorLine = isPickup ? ">ERR: MEM2 REMOVED" : ">MEM2 RESTORED";
	} else if((void*)testTile == (void*)tileManager->swordTile) {
		errorLine = isPickup ? ">ERR: MEM3 REMOVED" : ">MEM3 RESTORED";
	} else if((void*)testTile == (void*)tileManager->locustTile) {
		errorLine = isPickup ? ">ERR: MEM3 REMOVED" : ">MEM3 RESTORED";
		// follow this up with a dynamic call to THIS SAME FUNC, 
		// WITH THE TILE TO THE RIGHT
		// BUT ONLY IF THAT TILE ISNT NULL.
		// moving the locust number tile does nothing. the only thing that does things, 
		// is the locust sprite tile.
		// wait am i fucking halucinating?
		// i am. 
		// wtf
			
		errorLine = isPickup ? ">ERR: LI NULL" : ">LI RESTORED";
	} else if((void*)testTile == (void*)tileManager->locustCounterTile) {
		if(tilePos.x != 0 && (void*)tileManager->floorMap[tilePos.x-1][tilePos.y] == (void*)tileManager->locustTile) {
			//bufferedTile = tileManager->locustTile;	
		}
	} else if((void*)testTile == (void*)tileManager->floorTile1) {
		// carcus ending
		if(tileManager->floorTile1->second == '?') {
			//crashGame("
		} else {
			doCarcusEnding = true;
			errorLine = ">FATAL ERROR : BR NULL";
		}
	} else if((void*)testTile == (void*)tileManager->rodTile) {
		// crash, but i havent implimented that yet
		errorLine = ">FATAL ERR: VR HAS BEEN SEVERED FROM SYSTEM";
		doCrashGame = true;
	} else if((void*)testTile == (void*)tileManager->voidTile1) {
		if(isVoided) {
			errorLine = ">FATAL ERROR: VOID BREACHED SHUTTING SYSTEM DOWN";
			doCrashGame = true;
		} else {
			errorLine = ">FATAL ERROR: HP NULL";
			//doCrashGame = true;
			game->entityManager.addKill(game->entityManager.player); // this makes them fall, not slump over, but its ok for now
		}
	} else if(isVoided && (void*)testTile == (void*)tileManager->voidTile2) {
		// crash (is this crash different if voided vs unvoided?)
		errorLine = ">FATAL ERROR: VOID BREACHED SHUTTING SYSTEM DOWN";
		doCrashGame = true;
	}
	
	// deal with numbertiles 
	
	if((void*)testTile == (void*)tileManager->floorTile2) {
		if(tileManager->floorTile2->first == '?' || tileManager->floorTile2->second == '?') {
			errorLine = ">FATAL ERROR: BR NULL";
			doCrashGame = true;
		}		
	}
	
	if(numberTiles.contains(static_cast<WordTile*>(testTile))) {
		// it rlly would be nice if the locust sprite tile was a wordtile. 
		// i could be hacky with it and have the locust tile be represented with chars, but idrk 
		// if its worth it for this one jank case
		
		BN_LOG("number tile");
		
		if(tilePos.x != 0 && tileManager->floorMap[tilePos.x-1][tilePos.y] != NULL) {
			
			FloorTile* leftTile = tileManager->floorMap[tilePos.x-1][tilePos.y];
			
			int leftTileIndex = -1;
			int currentTileIndex = -1;
			
			if(leftTile == (FloorTile*)tileManager->voidTile1) {
				// even being here implies being unvoided
				leftTileIndex = 0;
			} else if(leftTile == (FloorTile*)tileManager->locustTile) {
				leftTileIndex = 1;
			} else if(leftTile == (FloorTile*)tileManager->floorTile1) {
				leftTileIndex = 2;
			} else {
				BN_LOG("number tile left index not a modifier, returning");
				return; // IS THIS OK?
			}
			
			if(testTile == (FloorTile*)tileManager->voidTile2) {
				currentTileIndex = 0;
			} else if(testTile == (FloorTile*)tileManager->locustCounterTile) {
				currentTileIndex = 1;
			} else if(testTile == (FloorTile*)tileManager->floorTile2) {
				currentTileIndex = 2;
			} else {
				BN_ERROR("this should like, never, ever, happen");
			}
			
			BN_ASSERT(leftTileIndex != -1 && currentTileIndex != -1, "some really fucked shit occured in the dis os area");
			
	
			// there is totally a better way of doing this.
			// i remember in datastructurs hw6 i actually had a thing like this, where i was trying to swap pointers, and 
			// thinking of the differences between a pointer, a double pointer, and a refrence to a pointer
			
			// wtf
			//void** tilePointers[3] = {(void**)&tileManager->voidTile2, (void**)&tileManager->locustCounterTile, (void**)&tileManager->floorTile2};
			
			// since these two numbers are guarenteed to be unique, we can now do: jank shit
			// better than hardcoding
			if(leftTileIndex != currentTileIndex) {
				int switchVal = leftTileIndex + currentTileIndex;
				switch(switchVal) {
					case 1:
						swap(tileManager->voidTile2, tileManager->locustCounterTile);
						break;
					case 2:
						swap(tileManager->voidTile2, tileManager->floorTile2);
						break;
					case 3:
						swap(tileManager->locustCounterTile, tileManager->floorTile2);
						break;
					default:
						BN_ERROR("once again, the dis os code is shit :)");
						break;
				}
			}
		
			if(testTile == (FloorTile*)tileManager->voidTile2) {
				errorLine = isPickup ? ">ERR: INVALID/MISSING HP VALUE" : ">HP RESTORED";
			} else if(testTile == (FloorTile*)tileManager->locustCounterTile) {
				errorLine = isPickup ? ">ERR: INVALID/MISSING LI VALUE" : ">LI VALUE RESTORED";
			} else if(testTile == (FloorTile*)tileManager->floorTile2) {
				errorLine = isPickup ? ">ERR: INVALID/MISSING BR VALUE" : ">BR VALUE RESTORED";
			} else {
				BN_ERROR("this should like, never, ever, happen, omfg");
			}
			
			
			//BN_LOG("voidtile2         is at ", tileManager->voidTile2->tilePos);
			//BN_LOG("locustCounterTile is at ", tileManager->locustCounterTile->tilePos);
			//BN_LOG("floorTile2        is at ", tileManager->floorTile2->tilePos);
			
			if((testTile == (FloorTile*)tileManager->floorTile2) && tilePos == Pos(13, 8) && !isPickup) {
				
				// if the restored value is equal to our current room, we dont do shit.
				int testRoomIndex = tileManager->getRoomIndex();
				//BN_LOG("swaprooms! ", testRoomIndex, " ", game->roomManager.roomIndex);
				if(testRoomIndex != game->roomManager.roomIndex) {
					//BN_LOG("swaprooms!");
					game->entityManager.addKill(NULL);
				}
				
			}
		}
	}
	
	
	
	
	if(errorLine == NULL) {
		return;
	}
	
	if(disText.size() == disText.max_size()) {
		disText.pop_back();
	}
	disText.insert(disText.begin(), errorLine);
	
	
	if(disTextSprites.size() == 0) {
		disTextSprites.push_back(bn::vector<bn::sprite_ptr, MAXTEXTSPRITES>());
		
		char tempBuffer[32];
		memset(tempBuffer, 0, 32);

		strcpy(tempBuffer, "DIS OS REPORT \0");
		if(__DATE__[4] == ' ') {
			tempBuffer[14] = '0';
		} else {
			tempBuffer[14] = __DATE__[4];
		}
		
		if(__DATE__[5] == ' ') {
			tempBuffer[15] = '0';
		} else {
			tempBuffer[15] = __DATE__[5];
		}
		
		tempBuffer[16] = '/';
		tempBuffer[17] = '0' + (MONTH / 10);
		tempBuffer[18] = '0' + (MONTH % 10);
		tempBuffer[19] = '/';
		tempBuffer[20] = '1';			
		tempBuffer[21] = '1';
	
		// Nov  5 2023
		
		strcpy(tempBuffer+22, __DATE__+7);
		
		//strcpy(tempBuffer+14, __DATE__[4]);
		//"DIS OS REPORT 05/11/112023"
		//BN_LOG(tempBuffer);
		//BN_LOG(__DATE__);
		
		disTextGenerator.generate((bn::fixed)-120+8+8, (bn::fixed)-80+16, bn::string_view(tempBuffer), disOsTextSprites);
		for(int i=0; i<disOsTextSprites.size(); i++) {
			disOsTextSprites[i].set_palette(game->pal->getWhiteSpritePalette());
			disOsTextSprites[i].set_bg_priority(3);
		}
	}

		
	if(disTextSprites.size() == disTextSprites.max_size()) {
		disTextSprites.pop_back();
	}
	
	// move all previous sprites down 16 tiles.
	for(int j=0; j<disTextSprites.size(); j++) {
		for(int i=0; i<disTextSprites[j].size(); i++) {
			disTextSprites[j][i].set_y(-80+16+((j+2)*16));
			if(j == 0) {
				disTextSprites[j][i].set_palette(game->pal->getDarkGraySpritePalette());
			}
		}
	}
	
	disTextSprites.insert(disTextSprites.begin(), bn::vector<bn::sprite_ptr, MAXTEXTSPRITES>());

	BN_ASSERT(disTextSprites.size() >= 2, "in the dis text generator, there were not a minimum of two lines to be generated!");
	
	disTextGenerator.generate((bn::fixed)-120+8+8, (bn::fixed)-80+16+(1*16), bn::string_view(disText[0]), disTextSprites[0]);
	for(int i=0; i<disTextSprites[0].size(); i++) {
		disTextSprites[0][i].set_palette(game->pal->getLightGraySpritePalette());
		disTextSprites[0][i].set_bg_priority(3);
	}
	
	if(doCarcusEnding) {
		carcusEnding();
	}
	
	if(doCrashGame) {
		crashGame();
	}
	
	//if(bufferedTile != NULL) {
	//	disCrash(bufferedTile, isPickup);
	//
	
	
}

// -----

void CutsceneManager::freeLayer(int i) {
	maps[i]->bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_default_bg_tiles);
	maps[i]->bgPointer.set_map(mapBackup[i]);
	
	// causes some intial corrupion, but prevents fringe crashes
	// if possible, im only going to call this after ALL frees are called. hopefully that reduces buffer?
	// actually, i dont want to risk taht
	bn::bg_blocks_manager::update();
}

void CutsceneManager::backupLayer(int i) {

	mapBackup[i] = maps[i]->bgPointer.map();
	
	zIndexBackup[i] = maps[i]->bgPointer.z_order();
	priorityBackup[i] = maps[i]->bgPointer.priority();
	
	xPosBackup[i] = maps[i]->bgPointer.x();
	yPosBackup[i] = maps[i]->bgPointer.y();
	
	freeLayer(i);
}

void CutsceneManager::restoreLayer(int i) {
	// weirdly enough, the map MUST be set before the tiles here, why?

	if(i == 0) {
		globalGame->collision.rawMap.bgPointer.set_tiles(bn::regular_bg_tiles_ptr::allocate(512, bn::bpp_mode::BPP_4));
		globalGame->loadTiles();
	} else if(i == 1) {
		globalGame->tileManager.floorLayer.rawMap.bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_customfloortiles);
	}
	
	maps[i]->bgPointer.set_z_order(zIndexBackup[i]);
	maps[i]->bgPointer.set_priority(priorityBackup[i]);
	
	maps[i]->bgPointer.set_x(xPosBackup[i]);
	maps[i]->bgPointer.set_y(yPosBackup[i]);
	
	maps[i]->bgPointer.set_map(mapBackup[i]);
	
	maps[i]->reloadCells();
	
}

void CutsceneManager::backupAllButEffects() {
	backupLayer(0);
	backupLayer(1);
	backupLayer(3);
}

void CutsceneManager::restoreAllButEffects() {
	
	freeLayer(0);
	freeLayer(1);
	freeLayer(3);
	
	restoreLayer(1);
	restoreLayer(0);
	restoreLayer(3);

}

void CutsceneManager::backupAllButEffectsAndFloor() {
	backupLayer(0);
	backupLayer(3);
}

void CutsceneManager::restoreAllButEffectsAndFloor() {
	freeLayer(0);
	freeLayer(3);
	
	restoreLayer(0);
	restoreLayer(3);
}

void CutsceneManager::delay(int delayFrameCount) {
	// this function should of been made WAYYYY earlier
	for(int i=0; i<delayFrameCount; i++) {
		game->doButanoUpdate();
	}
}



