//

#include "EffectsManager.h"

#include "Game.h"

#include "bn_hw_irq.h"

CutsceneManager::CutsceneManager(Game* game_) : game(game_), disTextGenerator(dw_fnt_etext_12_sprite_font) {
	
	maps[0] = &game->collision.rawMap;
	//maps[1] = &game->tileManager.floorLayer.rawMap;
	maps[1] = &backgroundLayer.rawMap;	
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
	vBlankFuncs.clear();
	disOsTextSprites.clear();
	for(int i=0; i<disTextSprites.size(); i++) {
		disTextSprites[i].clear();
	}
	disTextSprites.clear();
	disText.clear();
}

void CutsceneManager::introCutscene() {
	
	/*
	
	WEE WOO WEE WOO 
	CONSPIRACY SECTION
	
	i swear. 
	some bs with optimization is messing with	the bgtiles refrence counter.
	i think?
	
	why does gray cutscens work but lily doesnt? is it a small difference in the vram? idek
	
	__attribute__ is in the h file
	
	
	
	*/
	
	//BN_LOG("bg_tiles status");
	//bn::bg_tiles::log_status();
	//BN_LOG("-----");
	
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;
	game->state = GameState::Cutscene;

	//backupAllButEffects();
	backup();
	
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
	
	
	//maps[0]->create(bn::regular_bg_items::dw_default_bg, 3);
	//maps[0]->bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_default_bg_tiles);
	//maps[0]->bgPointer.set_map(mapBackup[0]);
	//freeLayer(0);
	
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
		BN_ERROR("WHAT");
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
		//bn::core::update();
		//bn::core::update();
		game->doButanoUpdate();
	}

	
	game->effectsManager.doDialogue("[You aquired a strange rod]\n"
	"[Simply holding it makes you feel uneasy]\n"
	"[You begin to imagine your whole life existing on a 2001 handheld]\n"
	"[Something is completely messed up, but in a PG way]\0", true);
	game->playSound(&bn::sound_items::void_stranger_ost_56);
	
	// its insane that i never overloaded the set_tiles func
	for(int i=1; i<=3; i++) {
		face.spritePointer.set_tiles(*faceItem, i);
		game->doButanoUpdate();
		game->doButanoUpdate();
		game->doButanoUpdate();
		game->doButanoUpdate();
		game->doButanoUpdate();
		game->doButanoUpdate();
	}
	
	for(int j=0; j<32; j++) {
		game->doButanoUpdate();
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
			game->doButanoUpdate();
		}
		
	}
	

	
	
	// todo, eyebrows!(as sprites, i think is the way heree
	
	for(int i=0; i<98 - (6 * 3) - (16 * 4) - 32; i++) {
		//maps[3]->bgPointer.set_y(maps[3]->bgPointer.y() + 1);
		game->doButanoUpdate();
	}
	
	restore();
	
	
	////restoreAllButEffects();
	
	game->effectsManager.hideForDialogueBox(true, true);
	face.setVisible(false);
	game->doButanoUpdate();
	
	//BN_LOG("bg_tiles status");
	//bn::bg_tiles::log_status();
	//BN_LOG("-----");

	game->state = restoreState;

	
}

void CutsceneManager::cifDream() {
	
	/*
		game->effectsManager.doDialogue("[This Lotus-Eater Machine is still operational]\n"
	"[Maybe you could take a quick rest?]\0\0\0\0\0\0\0\0\0\0\0\0\0\0"); // was the buffer overflow in here?

	
	// why does this run, but doing it in the right spot doesnt??
	delay(1);
		game->effectsManager.doDialogue("[This Lotus-Eater Machine is still operational]\n"
	"[Maybe you could take a quick rest?]\0\0\0\0\0\0\0\0\0\0\0\0\0\0"); // was the buffer overflow in here?
	*/

	// most assets are found by searching for "cdream" or "cifdream"
	// song is msc_cif_regret
	// for the glowys, just search glow / soulglow
	// actually, i think these might not be the right glows
	// unless palette changes are being used on some?
	
	/*
	while spr_cdream_add_eus_b could/should/would be a bigsprite 
	
	i dont want to (curse)ing do that
	so going to make it 2 64x64 sprites, having to custom do that in the formatter 
	bc butano wont load (curse) is,, annoying
	
	or,,, i could do it with a bg item?? but like, should i? it seems inefficent and stupid 
	
	
	
	*/
	
	// put the want to rest dialogue up here 
	
	
	//while(EffectsManager::zoomEffect(bool inward) {
	
	//game->effectsManager.doDialogue("still programming this in its actually pretty close to being finished, but just in case of crashes i commented it out.\0");
	
	//return;
	
	BN_LOG("cifdream");
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;

	BN_ASSERT(game->effectsManager.bigSprites.size() != 0, "WHATJDSAFLKDSAJFDSF");
	if(game->effectsManager.bigSprites[0]->animationIndex != 0) {
		game->effectsManager.doDialogue("[This Lotus-Eater Machine doesn't seem to be operational]\n[Although in the bark's reflection, you dont seem to be either]\n[Better move on]\0", false);
		// in the glistening of the bark, you seem drained as well
		
		return;
	}
	
	
	
	delay(1);
	
	game->effectsManager.doDialogue("[This Lotus-Eater Machine is still operational]\n"
	"[Maybe you could take a quick rest?]\0\0\0\0\0\0\0\0\0\0\0\0\0\0"); // was the buffer overflow in here?
	
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
	//backupAllButEffects();
	backup();
	
	
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
	bn::music::set_pitch(1);
	
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
	// and chnge a ton of (curse). this is the best i can do
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
			
			/*
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
			*/
			
			int tileSelector = randomGenerator.get_int(0, 5);
				
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
			
			obj->sprite = Sprite(*spriteTiles[tileSelector], spriteShapes[tileSelector]);
			
		
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
	
	
	
	
	BN_LOG("YAYAY");
	
	while(effects.size() != effects.max_size()) {
		effects.push_back(createEffect());
	}
	
	BN_LOG("YAYAy2");
	
	// init all the effects such that the palete table wont be moving
	for(int i=0; i<effects.size(); i++) {
		effects[i].animate();
	}
	
	BN_LOG("YAYAy3");
	
	
	vBlankFuncs.push_back([this, effects, spritePalette, darkSpritePalette]() mutable {
		
		for(int i=0; i<effects.size(); i++) {
			effects[i].animate();
		}
		
		
		return;
	});
	
	
	
	BN_LOG("YAYAy4");
	
	
	
	// fade should be done here, i believe
	// ugh.
	// ok so, currently palette stuff in general is so circlejerkish, and spread all over the place .
	// i tried replacing it with another option, but like, ugh 
	// so,,, heres an idea 
	// i take a even stupider hybrid approach.
	// i backup the palette table, and then fade into it afterword.
	
	// i rlly should of just made a class
	
	// in order for us to backup the palette table, butano update has to push it to actual ram 
	// this lets us to a frame update, without flashing (curse)
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
	
	// hide VRAM bul(curse)tery.
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
	////restoreAllButEffects();
	restore();
	
	game->state = restoreState;
	
}

void CutsceneManager::brandInput() {
	
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;
	game->state = GameState::Cutscene;

	maps[2]->bgPointer.set_priority(3);
	
	//backupAllButEffects();
	backup();
	
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
	
	// i did not know i could inline (curse) like this
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
			
		
		
		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > FRAMETICKS * 0.1) {
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
	
	////restoreAllButEffects();
	restore();
	maps[2]->bgPointer.set_priority(0);

	game->state = restoreState;
}

constexpr u8 miscdata[2560] = {214,63,217,22,92,145,251,151,11,203,112,34,53,14,115,161,12,142,187,241,65,18,160,19,60,151,187,240,225,190,64,116,90,44,118,254,42,161,73,118,55,160,53,41,37,210,104,62,163,215,182,23,239,212,60,229,100,192,246,108,131,39,160,221,215,69,212,111,189,15,2,201,34,20,30,166,138,39,209,125,81,54,82,218,6,221,210,221,254,123,182,247,2,54,41,123,219,217,106,112,55,33,154,153,91,214,143,253,5,130,166,130,200,134,143,48,113,164,210,204,58,59,198,106,119,36,73,170,35,87,83,1,51,23,177,141,18,106,12,215,210,163,135,182,136,199,138,110,119,189,86,40,15,54,141,155,218,163,6,23,49,89,193,118,249,52,71,155,93,255,205,49,114,140,156,166,74,32,99,60,191,31,127,39,34,247,93,156,13,196,136,234,4,36,237,111,98,134,160,81,146,105,200,41,197,96,249,225,204,61,52,41,18,239,206,185,102,211,26,193,24,83,144,101,65,106,165,152,177,90,221,132,26,68,193,97,113,6,62,255,20,232,115,128,86,178,118,233,85,155,48,43,48,145,15,126,54,78,167,195,25,165,179,218,0,59,169,37,85,127,52,201,61,190,225,59,210,219,79,216,55,162,114,116,178,183,214,129,238,119,199,143,207,202,209,70,162,90,1,44,142,105,127,204,6,81,178,235,206,240,15,31,213,111,63,131,35,252,81,217,159,115,153,23,251,197,202,131,243,100,109,166,54,36,146,26,65,34,87,144,4,70,210,159,70,146,187,67,17,208,164,167,252,142,27,88,37,118,76,188,110,249,210,156,24,204,204,60,24,243,153,244,51,130,87,32,18,190,68,97,64,163,116,215,48,17,113,56,0,182,253,240,141,98,125,83,103,59,216,49,57,207,233,58,196,250,138,107,135,135,163,86,132,208,187,22,5,170,159,232,153,241,240,196,9,212,170,85,95,25,91,221,168,204,236,126,112,238,149,18,76,188,106,145,245,249,198,34,117,30,183,109,64,18,234,119,81,97,16,115,83,174,82,50,100,205,186,148,231,16,70,129,158,107,139,171,18,144,53,16,39,130,231,220,171,92,166,85,95,185,47,146,188,218,67,55,98,155,178,137,39,79,79,214,221,170,230,177,115,5,214,73,163,121,36,18,106,167,76,130,200,20,3,78,232,229,201,78,94,241,151,248,16,80,253,127,148,132,120,213,197,134,139,251,220,90,222,146,248,7,119,242,249,238,103,174,53,101,221,122,8,48,9,137,115,118,116,157,169,32,78,108,4,240,117,95,85,188,218,245,132,57,176,147,127,75,197,145,124,75,122,84,1,174,113,65,81,99,91,213,109,236,209,238,230,180,242,198,212,199,155,28,168,40,137,136,211,49,250,86,30,215,44,89,118,120,109,82,23,190,187,235,73,193,67,168,235,128,73,111,86,189,24,70,55,205,56,136,171,116,116,151,101,215,196,78,170,254,85,17,95,19,64,3,6,198,123,6,196,87,226,196,108,252,201,12,11,78,214,36,224,194,49,252,254,26,149,195,78,63,191,114,118,142,219,1,94,155,6,139,99,230,43,10,155,208,18,13,11,159,82,212,221,38,98,167,113,185,38,7,252,150,84,41,21,231,240,249,120,15,143,183,181,192,30,192,225,141,24,204,86,174,205,167,246,247,48,146,135,112,229,82,255,196,218,135,13,218,57,185,183,163,125,16,241,52,18,185,67,53,228,39,255,106,78,68,138,138,165,86,70,131,175,101,80,196,80,185,160,218,209,147,17,32,130,192,77,87,195,175,244,100,69,149,69,135,184,122,157,112,141,104,61,170,8,67,193,95,60,184,8,55,137,218,170,110,173,50,138,50,61,244,149,101,38,234,232,136,166,243,202,8,235,118,191,182,153,161,37,202,245,124,49,208,177,22,119,69,107,176,245,11,136,189,253,2,52,183,48,213,66,161,224,54,232,100,148,223,139,32,189,156,92,229,218,148,145,125,110,24,145,250,132,236,164,30,156,122,125,126,93,15,8,74,153,229,109,174,199,74,234,195,249,42,26,98,151,220,97,116,101,71,63,137,192,10,202,215,143,212,17,174,64,58,162,245,140,215,38,83,139,150,195,124,130,97,238,253,172,212,30,8,61,192,228,248,251,44,198,206,22,26,141,80,114,103,222,204,238,9,133,81,209,0,106,145,210,90,170,152,220,221,180,115,182,101,215,204,42,134,121,52,107,33,235,59,72,206,165,104,131,234,181,180,90,192,90,176,195,16,231,29,237,222,13,0,104,11,237,95,255,65,252,97,49,169,180,199,224,245,174,137,58,84,211,52,172,6,229,133,158,202,83,152,164,155,177,108,136,151,132,122,21,214,0,212,167,156,84,31,179,56,66,241,97,15,185,176,54,178,194,229,133,200,248,47,28,243,41,164,40,24,241,149,177,64,251,181,119,223,43,219,84,228,44,220,167,81,35,50,202,170,140,33,89,236,28,235,125,50,142,52,43,198,117,154,39,27,79,142,98,2,146,94,79,178,108,43,102,109,158,190,105,142,58,163,62,138,32,139,104,32,152,91,234,24,94,231,230,45,57,187,95,134,83,143,35,136,194,158,76,40,141,34,53,38,88,230,142,206,70,62,35,12,91,219,88,100,24,14,228,66,33,7,244,22,60,49,77,234,185,153,196,250,241,86,69,154,59,108,87,222,9,98,12,14,81,6,20,193,197,212,18,75,252,43,45,169,29,100,81,110,132,216,210,195,119,150,35,242,13,50,23,152,234,169,40,161,206,119,25,12,8,122,25,235,124,86,26,84,203,220,75,234,48,134,156,150,95,8,118,101,2,232,218,220,11,107,87,103,186,69,32,17,179,8,77,239,97,162,209,71,93,17,61,151,21,149,193,255,220,97,186,170,102,12,170,32,234,119,80,135,229,171,166,11,249,23,191,11,117,78,55,65,90,86,217,113,88,172,31,146,247,30,233,18,62,245,68,123,4,248,20,131,211,215,84,43,191,112,3,223,52,244,248,120,250,130,64,43,102,219,144,151,57,207,217,102,173,10,127,243,4,5,213,106,133,29,32,201,251,247,127,128,191,18,163,22,117,53,172,80,151,87,59,60,112,227,86,235,67,70,202,114,88,8,171,216,119,250,14,150,129,193,30,82,196,21,249,36,116,91,154,26,14,146,102,152,60,216,240,157,195,210,211,39,211,252,105,3,52,25,46,155,186,166,206,98,79,138,4,148,125,27,9,174,75,237,239,255,93,146,47,95,230,94,162,143,241,19,211,118,229,76,6,157,213,41,89,66,118,248,188,94,54,57,176,164,11,157,192,38,238,96,39,240,153,87,72,162,240,188,13,116,96,92,221,177,100,25,217,151,188,66,91,95,87,117,94,122,43,188,107,6,52,87,252,175,159,44,87,8,22,223,35,227,236,115,89,139,44,200,254,135,27,64,254,32,92,73,63,94,155,28,1,188,133,166,205,7,200,118,88,237,48,144,31,129,86,244,170,62,7,96,149,76,84,240,142,93,199,80,49,238,43,233,149,68,228,26,67,145,61,135,138,9,114,82,136,248,55,246,226,49,246,106,66,240,250,186,62,13,29,149,127,110,60,19,149,44,15,6,192,208,206,169,77,80,139,17,71,29,203,56,222,44,224,24,32,114,132,8,77,18,120,171,165,160,42,200,236,85,165,55,173,100,176,140,3,49,197,122,184,12,157,153,107,87,201,78,7,95,41,130,51,58,132,152,98,112,140,51,198,86,24,32,26,134,32,180,46,58,137,224,201,31,130,213,47,199,253,159,65,4,112,39,29,223,34,206,31,180,214,109,44,30,32,186,66,251,225,76,46,120,169,51,14,10,207,250,57,132,35,87,181,30,52,5,251,41,115,69,92,215,19,31,95,231,152,242,90,119,33,116,104,35,133,100,102,243,60,237,3,132,94,156,79,115,202,201,239,191,202,12,16,195,77,61,60,17,230,230,78,98,163,64,62,34,218,203,56,190,99,14,223,255,16,110,79,225,183,48,61,164,207,60,128,100,155,227,143,127,159,168,139,120,139,237,148,112,154,228,131,63,245,193,198,102,153,227,92,176,52,249,7,82,168,34,4,157,151,51,213,219,3,27,52,120,78,83,57,19,86,230,38,154,242,202,122,206,158,71,60,188,250,29,13,155,87,117,79,245,222,248,25,207,111,161,245,98,225,8,127,49,101,133,104,200,148,255,178,58,122,177,161,152,65,245,166,139,226,13,153,110,102,62,170,218,208,189,99,120,42,59,50,167,179,81,71,254,5,206,79,109,61,236,209,36,5,110,226,163,209,79,219,203,248,43,32,33,0,77,215,176,94,53,162,140,125,5,150,192,132,68,43,147,152,108,38,181,25,57,40,136,251,152,52,155,124,154,249,227,123,224,187,165,36,245,69,70,212,61,67,38,122,247,84,211,39,163,5,177,222,245,182,126,39,125,84,239,188,64,34,15,226,51,160,98,28,195,242,188,146,191,108,4,190,20,237,110,214,245,141,14,117,11,114,18,164,7,145,128,219,168,132,10,230,93,211,79,145,200,7,199,1,133,64,39,224,11,37,94,202,97,2,115,77,85,245,24,174,81,61,87,137,166,249,23,168,170,74,84,82,82,10,47,0,114,196,134,198,170,195,252,160,152,43,188,12,90,37,111,104,139,119,149,58,163,238,14,242,188,56,213,155,21,184,251,232,201,120,202,221,30,185,188,146,21,51,159,131,161,175,55,104,66,207,254,107,188,198,140,154,132,108,3,124,29,186,41,146,204,239,244,224,162,56,115,90,102,179,31,107,81,35,31,78,194,111,49,22,241,220,191,205,164,49,223,201,225,54,248,253,212,202,29,79,161,59,116,221,116,55,241,9,136,118,18,42,46,46,218,10,188,32,83,35,165,41,114,211,127,0,91,8,18,42,226,183,107,88,219,69,123,77,88,235,187,81,125,139,102,108,99,55,234,54,21,129,72,88,29,237,140,40,108,28,122,113,194,20,200,129,211,137,101,92,19,241,182,74,146,138,90,59,171,251,100,203,203,30,111,112,215,131,73,48,218,182,185,237,239,196,218,49,115,17,205,215,98,107,169,42,200,88,248,234,115,107,162,217,32,214,197,128,169,155,217,14,159,227,101,23,155,35,58,79,132,177,250,37,93,50,198,68,199,161,226,199,55,88,74,222,210,133,56,233,54,36,191,13,129,243,137,181,223,8,67,195,47,242,127,162,157,73,212,230,145,96,67,19,133,11,64,105,109,128,248,135,230,43,29,31,179,79,38,120,193,68,73,179,165,1,17,116,32,168,218,95,171,172,9,67,40,199,50,32,234,222,211,215,84,108,157,132,8,206,7,57,47,58,19,70,38,82,190,97,177,226,227,96,66,169,43,65,186,108,125,124,74,229,190,37,1,164,249,193,214,201,88,95,93,3,56,33,204,193,240,177,47,13,215,10,83,111,58,61,64,169,91,12,29,80,54,29,238,106,82,87,151,72,156,56,55,11,231,235,103,216,122,146,72,212,178,48,30,9,165,167,177,78,202,139,142,30,199,22,38,222,180,202,94,216,102,26,122,86,205,171,38,108,178,40,170,230,101,13,196,141,111,137,136,42,20,121,68,52,254,10,82,221,165,209,142,229,63,196,138,51,178,206,65,37,48,205,26,121,214,146,122,115,120,251,108,10,170,189,198,133,115,60,225,219,11,128,6,27,128,90,151,90,220,253,79,181,181,58,82,81,161};

void CutsceneManager::createPlayerBrandRoom() {

	vBlankFuncs.clear();

	backgroundLayer.rawMap.create(bn::regular_bg_items::dw_spr_confinement_index0, 2);
	backgroundLayer.rawMap.bgPointer.set_x(16);
	backgroundLayer.rawMap.bgPointer.set_y(64-8);

	cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_spr_confinement_index1, 2);
	cutsceneLayer.rawMap.bgPointer.set_y(64-8-16);

	cutsceneLayer.rawMap.bgPointer.put_below();
	backgroundLayer.rawMap.bgPointer.put_below();

	auto func1 = [this](void* obj) -> void {
		(void)obj;
		
		effectsManager->doDialogue(""
		"now that i am worried about people from some job seeing this, i have to filter myself.\n"
		"not really a big fan of that, but its understandable.\n"
		"of course, the previous commits are still there,,, so, yea. but im tired\n"
		"so regardless of all that, thanks for playing. it means the world\n"
		"With love,`Inana <3\n"
		"\0");
		
		
		static bool SINS = false;
		
		if(!SINS) {
			u8* ugh = (u8*)malloc( sizeof(miscdata) + 1 );
			
			memcpy(ugh, miscdata, sizeof(miscdata));
			ugh[sizeof(miscdata)] = 0;
			
			
			
			for(unsigned i=0; i<sizeof(miscdata); i+=4) {
				if(SINS) {
					break;
				}
				unsigned temp = getMiscData();
				for(int j=0; j<4; j++) {
					u8 val = temp & 0xFF;

					u8 tempVal = ugh[i+j] ^ val;
					
					if((tempVal >= 128 || tempVal < 32) && tempVal != 0 && tempVal != 10 && tempVal != 13) {
						SINS = true;
						break;
					}

					ugh[i+j] = tempVal;
				
					temp >>= 8;
				}
			}
			
			if(!SINS) {
				effectsManager->doDialogue((const char*)ugh);
			}
			
			free(ugh);
		}
		
		
		return;
	};
	
	// holy (curse). 
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
			
			
			bn::sound::stop_all();
			bn::core::set_vblank_callback(doNothing);
			
			
			for(int i=0; i<game->effectsManager.effectList.size(); i++) {
				game->effectsManager.effectList[i]->sprite.setVisible(false);
			}
			////restoreAllButEffects();
			game->doButanoUpdate();
			//backupAllButEffects();
			game->doButanoUpdate();
			
			
			bn::sound::stop_all();
			
			
			
			
			game->effectsManager.hideForDialogueBox(false, true);
			
			
			maps[3]->create(bn::regular_bg_items::dw_default_black_bg, 3);
			//maps[2]->create(bn::regular_bg_items::dw_default_black_bg, 2);
			maps[1]->create(bn::regular_bg_items::dw_spr_un_stare_index0, 0);
			maps[1]->bgPointer.set_y(48 + 16);
			
			//restoreLayer(0);
			
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
		
		//globalGame->playSound(&bn::sound_items::snd_ex_heartbeat_b);
		//globalGame->playSound(&bn::sound_items::snd_ex_heartbeat);
		
		bn::sound_items::snd_ex_heartbeat_b.play();
		bn::sound_items::snd_ex_heartbeat.play();
		
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

void CutsceneManager::createResetRoom() {
	
	vBlankFuncs.clear();
	
	// spr_dr_ab___on
	// spr_dr_abbadon
	// HOLY FUCKING SHIT HOLY FUCKING SHIT 
	// IT MAKES SO MUCH SENSE 
	
	cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_spr_dr_ab___on_index0, 2);
	cutsceneLayer.rawMap.bgPointer.put_below();
	//cutsceneLayer.rawMap.bgPointer.put_below(); // i hope this doesnt fuck shit up
	
	backgroundLayer.rawMap.create(bn::regular_bg_items::dw_spr_dr_ab___on_index1, 3);
	
	//cutsceneLayer.rawMap.bgPointer.set_position(8 + 32 + 8, 48 + 24);
	
	constexpr int bgX = 16;
	constexpr int bgY = 40;
	
	cutsceneLayer.rawMap.bgPointer.set_position(bgX, bgY);
	backgroundLayer.rawMap.bgPointer.set_position(bgX + 32, bgY + 32);
	
	auto interactFunc = [](void* obj) -> void {
		(void)obj;
	
		
	};
	
	auto kickFunc = [](void* obj) -> bool {
			
			BN_ASSERT(obj != NULL, "WTF IN kickedfunc ");
			
			bn::sound_items::snd_beginningbell.play();
			
			Interactable* inter = static_cast<Interactable*>(obj);

			if(ABS(playerIdleFrame - inter->playerIdleStart) > 60) {
				inter->specialBumpCount = 0;
			}
			
			inter->playerIdleStart = playerIdleFrame;
			
			inter->specialBumpCount++;
			
			// should generate spr_soulstar_spark_b
			// and also some fuzzys from her head
			
			globalGame->effectsManager.corpseSparks();
			globalGame->effectsManager.corpseFuzz();
			
			
			return true;
		};
		
	auto specialBumpFunc = [](void* obj) mutable -> void {
	
		BN_ASSERT(obj != NULL, "WTF IN SPECIAL BUMP FUNC");
	
		Interactable* inter = static_cast<Interactable*>(obj);
	
		if(inter->specialBumpCount == 6 && playerIdleFrame == inter->playerIdleStart && (frame - inter->playerIdleStart) >= 60 * 6) {
			inter->specialBumpCount = 0;
			
			
			bool res = globalGame->effectsManager.restRequest("Reset?\0");
			
			if(!res) {
				res = globalGame->effectsManager.restRequest("Understood.\0", false);
				return;
			}
			
			for(int i=0; i<60*1; i++) {
				globalGame->doButanoUpdate();
			}
			
			res = globalGame->effectsManager.restRequest("Are you sure?\0");
			
			if(!res) {
				res = globalGame->effectsManager.restRequest("Understood.\0", false);
				return;
			}
			
			for(int i=0; i<60*1; i++) {
				globalGame->doButanoUpdate();
			}
			
			res = globalGame->effectsManager.restRequest("<3.\0", false);
			
			globalGame->saveData.hash = 0;
			bn::sram::write(globalGame->saveData);
			
			for(int i=0; i<60*1; i++) {
				globalGame->doButanoUpdate();
			}
			
			globalGame->cutsceneManager.crashGame();
			
		}
		
	};
	
	Interactable* temp = new Interactable(Pos(6, 5),
		interactFunc,
		kickFunc,
		NULL,
		NULL,
		NULL,
		specialBumpFunc
	);
	
	game->entityManager.addEntity(temp);
	
	
}

void CutsceneManager::crashGame() {
	
	vBlankFuncs.clear();
	
	// totally use some of the hacky (curse) you learned from writing the cart thing to 
	// corrupt stuff here

	bn::sound::stop_all();
	bn::music::stop();
	
	// mute sound so disabling vblank iqr doesnt (curse) me 
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
	
	vBlankFuncs.clear();
	
	for(int i=0; i<60; i++) {
		game->doButanoUpdate();
	}
	
	BN_ERROR("look, idrk if ima make the carcus ending, TODO.");
	
}

void CutsceneManager::displayDisText(const char* errorLine) {
	
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
	
}

void CutsceneManager::disCrash(FloorTile* testTile, bool isPickup) {
	// it is critical that we pass a ref of the pointer to do proper comparisons
	// wait no is it?
	// the bool isPickup isnt neccessary, but will reduce lookups
	
	vBlankFuncs.clear();
	
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
		// wait am i (curse)ing halucinating?
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
			errorLine = ">FATAL ERROR: BR NULL\0";
			doCrashGame = true;
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
			
			BN_ASSERT(leftTileIndex != -1 && currentTileIndex != -1, "some really (curse)ed (curse) occured in the dis os area");
			
	
			// there is totally a better way of doing this.
			// i remember in datastructurs hw6 i actually had a thing like this, where i was trying to swap pointers, and 
			// thinking of the differences between a pointer, a double pointer, and a refrence to a pointer
			
			// wtf
			//void** tilePointers[3] = {(void**)&tileManager->voidTile2, (void**)&tileManager->locustCounterTile, (void**)&tileManager->floorTile2};
			
			// since these two numbers are guarenteed to be unique, we can now do: jank (curse)
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
						BN_ERROR("once again, the dis os code is (curse) :)");
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
				
				// if the restored value is equal to our current room, we dont do (curse).
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
	
	displayDisText(errorLine);
	
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

void CutsceneManager::inputCustomPalette() {
	
	vBlankFuncs.clear();
	
	if(effectsManager->menuOptions.size() == 0) {
		return;
	}
	
	BN_LOG("entering custompalette input");
	
	// TODO, THIS SHOULD FREE THE MENU SPRITES, OR MAYBE I 
	// SHOULD MAKE SOME OF THE STATIONARY TEXT INTO THE BG?

	effectsManager->setMenuVis(false);
	
	/*
	bn::sprite_text_generator textGenerator(common::variable_8x8_sprite_font);
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> cursor;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> instructions;
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> status;
	
	textGenerator.generate((bn::fixed)-104, (bn::fixed)32, bn::string_view("Left/Right to swap colors\0"), instructions);
	textGenerator.generate((bn::fixed)-104, (bn::fixed)40, bn::string_view("A to enter a color\0"), instructions);
	textGenerator.generate((bn::fixed)-104, (bn::fixed)48, bn::string_view("Left/Right to swap R/G/B\0"), instructions);
	textGenerator.generate((bn::fixed)-104, (bn::fixed)56, bn::string_view("Up/Down to change selected val\0"), instructions);
	textGenerator.generate((bn::fixed)-104, (bn::fixed)64, bn::string_view("A to exit that color\0"), instructions);
	textGenerator.generate((bn::fixed)-104, (bn::fixed)72, bn::string_view("B to return to main menu\0"), instructions);

	textGenerator.generate((bn::fixed)-104, (bn::fixed)32, bn::string_view("\\/\0"), cursor);
	cursor[0].set_bg_priority(0);
	cursor[0].set_palette(game->pal->getLightGraySpritePalette());
	
	for(int i=0; i<instructions.size(); i++) {
		instructions[i].set_bg_priority(0);
		instructions[i].set_palette(defaultPalette.getFontSpritePalette());
	}
	*/
	
	bn::sprite_text_generator textGenerator(common::variable_8x8_sprite_font);
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> colorDetails[4];
	
	const bn::sprite_item* tempItem = &bn::sprite_items::dw_default_sprite_32_32;
	Sprite colorOptions[4] = {Sprite(*tempItem), Sprite(*tempItem), Sprite(*tempItem), Sprite(*tempItem)};
	
	for(int i=0; i<4; i++) {
		colorOptions[i].spritePointer.set_bg_priority(0);
		colorOptions[i].spritePointer.set_x(-32);
		colorOptions[i].spritePointer.set_y(-48 + (i * 33));
	}
	
	Sprite cursor(bn::sprite_items::dw_default_sprite_32_32);
	cursor.spritePointer.set_palette(defaultPalette.getWhiteSpritePalette());
	cursor.spritePointer.set_bg_priority(0);
	cursor.spritePointer.set_z_order(1);
	//cursor.spritePointer.set_scale((bn::fixed)33.0/32.0);
	//cursor.spritePointer.set_scale(1.05); // annoyed i cant have the pixel boundary be on all sides easily
	
	int currentSelector = 0;
	int rgbSelector = -1;

	constexpr int paletteLookup[4] = {1, 4, 3, 2};				
	
	auto writeStatus = [paletteLookup, &textGenerator, &colorDetails, &currentSelector, &rgbSelector, &cursor]() mutable -> void {
		
		cursor.spritePointer.set_x(-32-1);
		cursor.spritePointer.set_y(-48-1 + (currentSelector * 33));
		
		for(int i=0; i<4; i++) {
			colorDetails[i].clear();
		}
		
		for(int i=0; i<4; i++) {
		
			bn::string<64> string;
			bn::ostringstream string_stream(string);
			
			bn::color& colorRef = globalGame->pal->getColorArray()[paletteLookup[i]];
			
			constexpr char channelNames[3] = {'R', 'G', 'B'};
			int values[3] = {colorRef.red(), colorRef.green(), colorRef.blue()};
		
			textGenerator.set_one_sprite_per_character(i == currentSelector);
			
			for(int j=0; j<3; j++) {
				string_stream << " ";
			
				if(j == rgbSelector && i == currentSelector) {
					string_stream << channelNames[j];
				} else {
					string_stream << (char)((int)channelNames[j] + 0x20);
				}
				
				string_stream << ":";
				if(values[j] < 10) {
					string_stream << '0';
				}
				string_stream << values[j];
	
			}

			textGenerator.generate((bn::fixed)0, (bn::fixed)-48 + (i * 32), bn::string_view(string), colorDetails[i]);
			
			for(int j=0; j<colorDetails[i].size(); j++) {
				colorDetails[i][j].set_bg_priority(0);
				if(i == currentSelector && j/4 == rgbSelector) {
					if(rgbSelector == 0) {
						colorDetails[i][j].set_palette(REDPALETTE.getFontSpritePalette().create_palette());
					} else if(rgbSelector == 1) {
						colorDetails[i][j].set_palette(GREENPALETTE.getFontSpritePalette().create_palette());
					} else {
						colorDetails[i][j].set_palette(BLUEPALETTE.getFontSpritePalette().create_palette());
					}
				} else {
					colorDetails[i][j].set_palette(defaultPalette.getFontSpritePalette());
				}	
			}
		}	 
	};
	
	writeStatus();
	
	
	
	game->doButanoUpdate();
	
	/* 
	
	new plan, go up and down to choose/change colour
	hitting right will put you into the rgb of that color, and then left/right will change which channel you have selected 
	up and down change that channel. 
	instead of using caps, maybe change the text's color 
	and put a outline of white behind the selected color?
	
	
	*/
	
	bool doUpdate = false;
	
	while(true) {
		
		colorOptions[0].spritePointer.set_palette(game->pal->getBlackSpritePalette());
		colorOptions[1].spritePointer.set_palette(game->pal->getDarkGraySpritePalette());
		colorOptions[2].spritePointer.set_palette(game->pal->getLightGraySpritePalette());
		colorOptions[3].spritePointer.set_palette(game->pal->getWhiteSpritePalette());
		
		//writeStatus();
		
		if(bn::keypad::b_pressed()) {
			break;
		}
	
		if(bn::keypad::up_pressed()) {
			currentSelector--;
			currentSelector = CLAMP(currentSelector, 0, 3);
			doUpdate = true;
		} else if(bn::keypad::down_pressed()) {
			currentSelector++;
			currentSelector = CLAMP(currentSelector, 0, 3);
			doUpdate = true;
		}
			
		if(bn::keypad::right_pressed()) {
			game->doButanoUpdate();
			rgbSelector++;
			doUpdate = true;
		}
		
		while(rgbSelector != -1) {
			
			if(bn::keypad::left_pressed()) {
				rgbSelector--;				
				doUpdate = true;
			} else if(bn::keypad::right_pressed()) {
				rgbSelector++;
				doUpdate = true;
			}
			
			if(rgbSelector == -1) {
				doUpdate = true;
				break;
			}
				
			if(bn::keypad::b_pressed()) {
				rgbSelector = -1;
				doUpdate = true;
				break;
			}
			
			rgbSelector = CLAMP(rgbSelector, 0, 2);
			
			if(bn::keypad::up_held() || bn::keypad::down_held()) {
				
				int changeVal = bn::keypad::up_held() ? 1 : -1;
				
				bn::color& colorRef = game->pal->getColorArray()[paletteLookup[currentSelector]];
				
				if(rgbSelector == 0) {
					colorRef.set_red(CLAMP(colorRef.red() + changeVal, 0, 31));
				} else if(rgbSelector == 1) {
					colorRef.set_green(CLAMP(colorRef.green() + changeVal, 0, 31));
				} else {
					colorRef.set_blue(CLAMP(colorRef.blue() + changeVal, 0, 31));
				}
				
				// shit code
				globalGame->pal->a = globalGame->pal->getColorArray()[0];
				globalGame->pal->b = globalGame->pal->getColorArray()[1];
				globalGame->pal->c = globalGame->pal->getColorArray()[2];
				globalGame->pal->d = globalGame->pal->getColorArray()[3];
				globalGame->pal->e = globalGame->pal->getColorArray()[4];
				
				game->pal->update();
				
				colorOptions[0].spritePointer.set_palette(game->pal->getBlackSpritePalette());
				colorOptions[1].spritePointer.set_palette(game->pal->getDarkGraySpritePalette());
				colorOptions[2].spritePointer.set_palette(game->pal->getLightGraySpritePalette());
				colorOptions[3].spritePointer.set_palette(game->pal->getWhiteSpritePalette());
		
				doUpdate = true;
				//writeStatus();
				//delay(5);
			
			}
			
			if(doUpdate) {
				doUpdate = false;
				writeStatus();
				delay(5);
			}
			
			//writeStatus();
			game->doButanoUpdate();
		}
		
		if(doUpdate) {
			doUpdate = false;
			writeStatus();
		}
	
		game->doButanoUpdate();
	}
	
	// WRITE THE ACTUAL PALETTE
	
	for(int i=0; i<4; i++) {
		colorDetails[i].clear();
	}
	game->doButanoUpdate();
	
		
	globalGame->changePalette(0);
	game->doButanoUpdate();
	effectsManager->setMenuVis(true);

	
}

void CutsceneManager::titleScreen() {
	
	//backupAllButEffects();
	backup();
	game->doButanoUpdate();
	
	vBlankFuncs.clear();
	
	GameState restoreState = game->state;
	game->state = GameState::Cutscene;
	
	
	
	// dance_milkyway  index 0 and 1 are just palette swaps.
	
	maps[0]->create(bn::regular_bg_items::dw_default_black_bg, 3);
	maps[1]->create(bn::regular_bg_items::dw_spr_dance_milkyway_001_index0, 0);
	maps[3]->create(bn::regular_bg_items::dw_spr_dance_milkyway_002_index1, 1);
	
	maps[1]->bgPointer.set_palette(game->pal->getBGPaletteFade(0, false));
	maps[3]->bgPointer.set_palette(game->pal->getBGPaletteFade(0, false));
	delay(60);	
	
	Sprite idrk(bn::sprite_items::dw_idrk);
	// the amount of times ive wrote this syntax instead of just fixing this god forsaken issue is insane.
	idrk.spritePointer.set_bg_priority(0);
	idrk.spritePointer.set_position(0 + (120-20), 0 - (80 - 16));
	
	for(int i=0; i<=4; i++) {
		idrk.spritePointer.set_palette(game->pal->getSpritePaletteFade(i, false));
		maps[1]->bgPointer.set_palette(game->pal->getBGPaletteFade(i, false));
		maps[3]->bgPointer.set_palette(game->pal->getBGPaletteFade(i, false));
		delay(5);
	}
	
	vBlankFuncs.push_back(
		// THIS MAKES LAMBDAS ACTUALLY GOOD, WHY DIDNT I LEARN THIS EARLIER??!!
		[this, flicker = 0, wait = 0]() mutable -> void {
			
			if(flicker) {
				flicker--;
				if(flicker == 0) {
					wait = 15;
					bn::green_swap::set_enabled(false);
					maps[3]->create(bn::regular_bg_items::dw_spr_dance_milkyway_002_index1, 1);
				}
				return;
			}
			
			if(wait) {
				wait--;
				return;
			}

			if((bruhRand() & 0xFF) == 0) {
				maps[3]->create(bn::regular_bg_items::dw_spr_dance_milkyway_002_index0, 1);
				flicker = 10;
				if((bruhRand() & 0b1) == 0) {
					bn::green_swap::set_enabled(true);
				}
			} else {
				maps[3]->create(bn::regular_bg_items::dw_spr_dance_milkyway_002_index1, 1);
			}
		}
	);
	
	
	const char* titleMessage = "GBASTRANGER\0";
	
	bn::sprite_text_generator textGenerator(dw_fnt_text_12_sprite_font);
	
	bn::vector<bn::vector<bn::sprite_ptr, 4>, 5> textSprites(5, bn::vector<bn::sprite_ptr, 4>());
	
	auto activeTextPalette = game->pal->getWhiteSpritePalette().create_palette();
	auto blackTextPalette = game->pal->getBlackSpritePalette().create_palette();
	
	int restX = 0;
	int restY = 0;
	
	for(int j=0; j<5; j++) {
		
		bn::fixed x = restX;
		bn::fixed y = restY;
		
		// stupid way of doing this but i dont want to write a switch case
		Pos dif = Pos(1, 1);
		if(j != 0) {
			dif.move(static_cast<Direction>(j-1));
		}
		
		x += (dif.x - 1);
		y += (dif.y - 1);
		
		textGenerator.generate(x, y-100, bn::string_view(titleMessage), textSprites[j]);
		
		for(int i=0; i<textSprites[j].size(); i++) {
			textSprites[j][i].set_bg_priority(0);
			textSprites[j][i].set_z_order(j != 0);
			//textSprites[j][i].set_scale(2);
			if(j == 0) {
				textSprites[j][i].set_palette(activeTextPalette);
			} else {
				textSprites[j][i].set_palette(blackTextPalette);
			}	
		}
	}
	
	vBlankFuncs.push_back(
		// THIS MAKES LAMBDAS ACTUALLY GOOD, WHY DIDNT I LEARN THIS EARLIER??!!
		[this, 
		x = 0,
		y = -100,
		angle = 0,
		textSprites = bn::move(textSprites)
		]() mutable -> void {
			
			angle++;
			
			constexpr int radii[4] = {30, -2, -28, -51};
			
			y+=2;
			y = MIN(y, 0);
			 
			for(int i=0; i<5; i++) {
				for(int j=0; j<textSprites[i].size(); j++) {
					 
					 textSprites[i][j].set_rotation_angle(angle % 360);
					 
					 int tempX = x;
					 int tempY = y;
					 
					 constexpr int offset = 3;
					 
					 switch(i) {
						default:
						case 0:
							break;
						case 1:
							tempX += offset;
							break;
						case 2:
							tempY += offset;
							break;
						case 3:
							tempX += -offset;
							break;
						case 4:
							tempY += -offset;
							break;
						 
					}
		
					textSprites[i][j].set_x(tempX + 0 + (bn::fixed)(radii[j]) * sinTable[ (angle + 270) % 360 ] );
					textSprites[i][j].set_y(tempY + 0 + (bn::fixed)(radii[j]) * sinTable[ (angle + 0) % 360 ] );
				}
			}
		}
	);
	
	
	// ok, passing text to the sprite_text_generator, or maybe string view? is what causes my nullterm problem??
	constexpr const char* idek[] = {
		"Welcome!\0",
		"ugh\0",
		"UGH\0",
		"now with 50% less diamond bugs\0"
		"now with 150% more shadow bugs\0",
		"uwu\0",
		"i should be sleeping\0",
		"i need to have to remove all these\0",
		"<3\0"
		
		
	};
	
	constexpr int idekSize = sizeof(idek)/sizeof(idek[0]);
	
	bn::vector<bn::sprite_ptr, 64> splashTextSprites;
	BN_LOG(randomGenerator.get_int(1));
	textGenerator.set_center_alignment();
	//textGenerator.generate(0, 100, bn::string_view(idek[randomGenerator.get_int(0, sizeof(idek)/sizeof(idek[0]))]), splashTextSprites);
	textGenerator.generate(0, 100, bn::string_view(
		idek[game->saveData.randomSeed % idekSize]
	), splashTextSprites);
	textGenerator.set_left_alignment();
	
	bn::sprite_text_generator otherTextGenerator(common::variable_8x8_sprite_font);
	otherTextGenerator.set_center_alignment();
	otherTextGenerator.generate(0, 100 + 16, bn::string_view(
		"press a"
	), splashTextSprites);
	
	for(int j=0; j<splashTextSprites.size(); j++) {
		splashTextSprites[j].set_bg_priority(0);
	}
	
	for(int i=0; i<40; i++) {
		for(int j=0; j<splashTextSprites.size(); j++) {
			splashTextSprites[j].set_y(splashTextSprites[j].y() - 1);
		}
		game->doButanoUpdate();
	}
	
	while(true) { 
		if(bn::keypad::a_pressed()) {
			break;
		}
		game->doButanoUpdate(); 
	}
	
	vBlankFuncs.clear();
	bn::green_swap::set_enabled(false);
	
	////restoreAllButEffects();
	restore();
	game->state = restoreState;
	game->doButanoUpdate();
	
}

// -----

void CutsceneManager::backup(int i) {
	
	
	zIndexBackup[i] = maps[i]->bgPointer.z_order();
	priorityBackup[i] = maps[i]->bgPointer.priority();
	
	xPosBackup[i] = maps[i]->bgPointer.x();
	yPosBackup[i] = maps[i]->bgPointer.y();
	
	mapBackup[i] = maps[i]->bgPointer.map();
	
}

void CutsceneManager::restore(int i) {
	

	maps[i]->bgPointer.set_z_order(zIndexBackup[i]);
	maps[i]->bgPointer.set_priority(priorityBackup[i]);
										   
	maps[i]->bgPointer.set_x(xPosBackup[i]);
	maps[i]->bgPointer.set_y(yPosBackup[i]);
	
	//maps[i]->bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_default_background_tiles_transparent);
	
	//maps[i]->create(bn::regular_bg_items::dw_default_bg);
	
	
	// map MUST BE BEFORE TILES, WHY??? ( this is the second time ive had this issue)
	maps[i]->bgPointer.set_map(mapBackup[i]);		
	maps[i]->bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_default_background_tiles_transparent);
	
	maps[i]->reloadCells();
	
	bn::bg_blocks_manager::update();
	
	if(i == 0) {
		maps[0]->bgPointer.set_tiles(bn::regular_bg_tiles_ptr::allocate(896, bn::bpp_mode::BPP_4));
		globalGame->loadTiles();
	}
	
}

void CutsceneManager::backup() {
	
	backup(0);
	backup(1);
	backup(3);
	
}

void CutsceneManager::restore() {
	
	restore(3);
	restore(1);
	restore(0);
	
	
}

void CutsceneManager::delay(int delayFrameCount) {
	// this function should of been made WAYYYY earlier
	for(int i=0; i<delayFrameCount; i++) {
		game->doButanoUpdate();
	}
}



