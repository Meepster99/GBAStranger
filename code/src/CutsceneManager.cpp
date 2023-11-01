

#include "EffectsManager.h"

#include "Game.h"


CutsceneManager::CutsceneManager(Game* game_) : game(game_) {
	
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
	
	//maps[i]->bgPointer.set_tiles(tilesBackup[i]);
	
	/*
	game->collision.rawMap.bgPointer.set_tiles(bn::regular_bg_tiles_items::dw_default_bg_tiles);
	//bn::core::update();
	
	
	game->needRedraw = true;
	
	int i = 0;
	
	maps[i]->bgPointer.set_map(mapBackup[i]);
	game->collision.rawMap.bgPointer.set_tiles(bn::regular_bg_tiles_ptr::allocate(512, bn::bpp_mode::BPP_4));
	
	maps[i]->bgPointer.set_z_order(zIndexBackup[i]);
	maps[i]->bgPointer.set_priority(priorityBackup[i]);
	
	maps[i]->bgPointer.set_x(xPosBackup[i]);
	maps[i]->bgPointer.set_y(yPosBackup[i]);
	
	maps[i]->reloadCells();
	*/
	
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

void CutsceneManager::delay(int delayFrameCount) {
	// this function should of been made WAYYYY earlier
	for(int i=0; i<delayFrameCount; i++) {
		game->doButanoUpdate();
	}
}



