//

#include "Game.h"
#include "EffectsManager.h"

#include "bn_hw_irq.h"

CutsceneManager::CutsceneManager(Game* game_) : game(game_), disTextGenerator(dw_fnt_etext_12_sprite_font) {

	maps[0] = &game->collision.rawMap;
	maps[1] = &backgroundLayer.rawMap;
	maps[2] = &game->effectsManager.effectsLayer.rawMap;
	maps[3] = &cutsceneLayer.rawMap;

	for(int i=0; i<4; i++) {
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

	maps[1]->create(bn::regular_bg_items::dw_spr_vd_bg2_index0, 2);

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
		game->doButanoUpdate();
	}

	/*

	[The strange rod has changed its form]\n
	[And somehow...]\n
	[You feel your uneasiness fading away]\n

	*/

	if(isSuperRodChest) {
		switch(game->mode) {
			default:
			case 0:
				game->effectsManager.doDialogue("[The strange rod has changed its form]\n"
					"[And somehow...]\n"
					"[You feel your uneasiness fading away]\0", true);
				break;
			case 1:
				game->effectsManager.doDialogue("[The strange rod has changed its form]\n"
				"[And somehow...]\n"
				"[You feel your uneasiness fading away]\0", true);
				break;
			case 2: // comment on unused graphics
				game->effectsManager.doDialogue("[The strange rod has changed its form]\n"
				"[However...]\n"
				"[Your uneasiness remains.]\n"
				"[Shouldn't someone else have this?]\0", true);
				break;
		}
	} else {
		game->effectsManager.doDialogue("[You aquired a strange rod]\n"
		"[Simply holding it makes you feel uneasy]\n"
		"[You begin to imagine your whole life existing on a 2001 handheld]\n"
		"[Something is completely messed up, but in a PG way]\0", true);
	}


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

		maps[0]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		maps[1]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));
		maps[3]->bgPointer.set_palette(game->pal->getBGPaletteFade(i));

		face.spritePointer.set_palette(game->pal->getSpritePaletteFade(i));

		delay(16);
	}

	// todo, eyebrows!(as sprites, i think is the way heree

	for(int i=0; i<98 - (6 * 3) - (16 * 4) - 32; i++) {
		//maps[3]->bgPointer.set_y(maps[3]->bgPointer.y() + 1);
		game->doButanoUpdate();
	}

	restore();

	game->effectsManager.hideForDialogueBox(true, true);
	face.setVisible(false);
	game->doButanoUpdate();

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

	BN_LOG("cifdream");
	vBlankFuncs.clear();

	GameState restoreState = game->state;

	BN_ASSERT(game->effectsManager.bigSprites.size() != 0, "WHATJDSAFLKDSAJFDSF");
	if(game->effectsManager.bigSprites[0]->animationIndex != 0 &&
	game->effectsManager.bigSprites[0]->tiles == &bn::sprite_tiles_items::dw_spr_birch_b) {
		game->effectsManager.doDialogue("[This Lotus-Eater Machine doesn't seem to be operational]\n[Although in the bark's reflection, you dont seem to be either]\n[Better`move`on]\0", false);
		// in the glistening of the bark, you seem drained as well

		return;
	}

	delay(1);

	game->effectsManager.doDialogue("[This Lotus-Eater Machine is still operational]\n"
	"[Maybe you could take a quick rest?]\0\0"); // was the buffer overflow in here?

	delay(1);

	if(!game->effectsManager.restRequest()) {
		return;
	}

	game->state = GameState::Dialogue;

	delay(60);

	while(!game->effectsManager.zoomEffect(true, false)) {
		delay(20);
	}

	bn::music::stop();
	delay(2);

	disOsTextSprites.clear();
	for(int i=0; i<disTextSprites.size(); i++) {
		disTextSprites[i].clear();
	}
	disTextSprites.clear();

	delay(1);

	game->state = GameState::Cutscene;

	// ok. i no longer like this func. im just going to delete everything?
	game->effectsManager.hideForDialogueBox(false, true);

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

	// changing this to a pointer bc,,,, running of of stack?
	// its quite weird that,, effects was the thing using up so much iwram.
	// is each function pointers code, stored,,, in iwram?
	// idek whats goin on, but it works, so im good
	bn::vector<Effect*, 16> effects;

	// look at me being fancy
	// so stupid how i cant va_args this
	// is this not working?
	/*
	DEFER(effects,
		for(int i=0; i<effects.size(); i++) {
			delete effects[i];
			effects[i] = NULL;
		}
		effects.clear();
		globalGame->cutsceneManager.vBlankFuncs.clear();
	);
	*/

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

	// some fucky shit is,,, causing,,, the soul glows to pop in and out randomly?
	// but only sometimes?

	auto createEffect = [spritePalette, darkSpritePalette]() -> Effect* {

		auto createFunc = [spritePalette, darkSpritePalette](Effect* obj) mutable -> void {

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

			if(y.integer() < -16) {

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
				obj->sprite.spritePointer.set_palette(tileSelector != 0 ? darkSpritePalette : spritePalette);

				x = randomGenerator.get_int(16 * 14);
				//y = 16*5+randomGenerator.get_int(16);
				if(y == -32) {
					y = randomGenerator.get_int(16 * 9);
				} else {
					y = 16 * 10 + randomGenerator.get_int(32);;
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

			if(x.integer() > 240 + 16) {
				x -= 240;
			} else if(x.integer() < 0 - 16) {
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

		return new Effect(createFunc, tickFunc);
	};

	while(effects.size() != effects.max_size()) {
		effects.push_back(createEffect());
	}

	// init all the effects such that the palete table wont be moving
	for(int i=0; i<effects.size(); i++) {
		effects[i]->animate();
	}

	vBlankFuncs.push_back([this, effects, spritePalette, darkSpritePalette]() mutable {

		for(int i=0; i<effects.size(); i++) {
			effects[i]->animate();
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
	tempSprites.clear();

	for(int i=0; i<effects.size(); i++) {
		delete effects[i];
		effects[i] = NULL;
	}
	effects.clear();
	globalGame->cutsceneManager.vBlankFuncs.clear();

	game->effectsManager.bigSprites[0]->animate();

	game->effectsManager.hideForDialogueBox(true, true);
	restore();

	game->state = restoreState;
}

void CutsceneManager::brandInput() {

	vBlankFuncs.clear();

	GameState restoreState = game->state;
	game->state = GameState::Cutscene;

	maps[2]->bgPointer.set_priority(3);

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

	Pos cursorPos = Pos(4, 1);

	cursor.spritePointer.set_x(cursorPos.x * 16 - 240/2 + 16 + 1);
	cursor.spritePointer.set_y(cursorPos.y * 16 - 160/2 + 16 + 1);
	cursor.spritePointer.set_bg_priority(0);

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
			break;
		}

		if(bn::keypad::any_pressed() && inputTimer.elapsed_ticks() > FRAMETICKS * 0.1) {
			inputTimer.restart();

			if(allDone) {

				if(bn::keypad::up_pressed() || bn::keypad::down_pressed()) {
					if(bn::keypad::up_pressed()) {
						cursorPos.move(Direction::Up);
					} else {
						cursorPos.y = 1;
					}
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

				if(cursorPos.y < 1) {
					cursorPos.y = 1+6;
				} else if(cursorPos.y > 1+6) {
					cursorPos.y = 1;
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

				if(cursorPos.x < 4) {
					cursorPos.x = 4+6-1;
				} else if(cursorPos.x > 4+6-1) {
					cursorPos.x = 4;
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

	restore();
	maps[2]->bgPointer.set_priority(0);

	game->state = restoreState;
}

constexpr unsigned char miscdata[2952] = {214,63,217,22,92,145,251,151,11,203,112,34,53,14,115,161,12,142,187,241,65,18,160,19,60,151,187,240,225,190,64,116,90,44,118,254,42,161,73,118,55,160,53,41,37,210,104,62,163,215,182,23,239,212,60,229,100,192,246,108,131,39,160,221,215,69,212,111,189,15,2,201,34,20,30,166,138,39,209,125,81,54,82,218,6,221,210,221,254,123,182,247,2,54,41,123,219,217,106,112,55,33,154,153,91,214,143,253,5,130,166,130,200,134,143,48,113,164,210,204,58,59,198,106,119,36,73,170,35,87,83,1,51,23,177,141,18,106,12,215,210,163,135,182,136,199,138,110,119,189,86,40,15,54,141,155,218,163,6,23,49,89,193,118,249,52,71,155,93,255,205,49,114,140,156,166,74,32,99,60,191,31,127,39,34,247,93,156,13,196,136,234,4,36,237,111,98,134,160,81,146,105,200,41,197,96,249,225,204,61,52,41,18,239,206,185,102,211,26,193,24,83,144,101,65,106,165,152,177,90,221,132,26,68,193,97,113,6,62,255,20,232,115,128,86,178,118,233,85,155,48,43,48,145,15,126,54,78,167,195,25,165,179,218,0,59,169,37,85,127,52,201,61,190,225,59,210,219,79,216,55,162,114,116,178,183,214,129,238,119,199,143,207,202,209,70,162,90,1,44,142,105,127,204,6,81,178,235,206,240,15,31,213,111,63,131,35,252,81,217,159,115,153,23,251,197,202,131,243,100,109,166,54,36,146,26,65,34,87,144,4,70,210,159,70,146,187,67,17,208,164,167,252,142,27,88,37,118,76,188,110,249,210,156,24,204,204,60,24,243,153,244,51,130,87,32,18,190,68,97,64,163,116,215,48,17,113,56,0,182,253,240,141,98,125,83,103,59,216,49,57,207,233,58,196,250,138,107,135,135,163,86,132,208,187,22,5,170,159,232,153,241,240,196,9,212,170,85,95,25,91,221,168,204,236,126,112,238,149,18,76,188,106,145,245,249,198,34,117,30,183,109,64,18,234,119,81,97,16,115,83,174,82,50,100,205,186,148,231,16,70,129,158,107,139,171,18,144,53,16,39,130,231,220,171,92,166,85,95,185,47,146,188,218,67,55,98,155,178,137,39,79,79,214,221,170,230,177,115,5,214,73,163,121,36,18,106,167,76,130,200,20,3,78,232,229,201,78,94,241,151,248,16,80,253,127,148,132,120,213,197,134,139,251,220,90,222,146,248,7,119,242,249,238,103,174,53,101,221,122,8,48,9,137,115,118,116,157,169,32,78,108,4,240,117,95,85,188,218,245,132,57,176,147,127,75,197,145,124,75,122,84,1,174,113,65,81,99,91,213,109,236,209,238,230,180,242,198,212,199,155,28,168,40,137,136,211,49,250,86,30,215,44,89,118,120,109,82,23,190,187,235,73,193,67,168,235,128,73,111,86,189,24,70,55,205,56,136,171,116,116,151,101,215,196,78,170,254,85,17,95,19,64,3,6,198,123,6,196,87,226,196,108,252,201,12,11,78,214,36,224,194,49,252,254,26,149,195,78,63,191,114,118,142,219,1,94,155,6,139,99,230,43,10,155,208,18,13,11,159,82,212,221,38,98,167,113,185,38,7,252,150,84,41,21,231,240,249,120,15,143,183,181,192,30,192,225,141,24,204,86,174,205,167,246,247,48,146,135,112,229,82,255,196,218,135,13,218,57,185,183,163,125,16,241,52,18,185,67,53,228,39,255,106,78,68,138,138,165,86,70,131,175,101,80,196,80,185,160,218,209,147,17,32,130,192,77,87,195,175,244,100,69,149,69,135,184,122,157,112,141,104,61,170,8,67,193,95,60,184,8,55,137,218,170,110,173,50,138,50,61,244,149,101,38,234,232,136,166,243,202,8,235,118,191,182,153,161,37,202,245,124,49,208,177,22,119,69,107,176,245,11,136,189,253,2,52,183,48,213,66,161,224,54,232,100,148,223,139,32,189,156,92,229,218,148,145,125,110,24,145,250,132,236,164,30,156,122,125,126,93,15,8,74,153,229,109,174,199,74,234,195,249,42,26,98,151,220,97,116,101,71,63,137,192,10,202,215,143,212,17,174,64,58,162,245,140,215,38,83,139,150,195,124,130,97,238,253,172,212,30,8,61,192,228,248,251,44,198,206,22,26,141,80,114,103,222,204,238,9,133,81,209,0,106,145,210,90,170,152,220,221,180,115,182,101,215,204,42,134,121,52,107,33,235,59,72,206,165,104,131,234,181,180,90,192,90,176,195,16,231,29,237,222,13,0,104,11,237,95,255,65,252,97,49,169,180,199,224,245,174,137,58,84,211,52,172,6,229,133,158,202,83,152,164,155,177,108,136,151,132,122,21,214,0,212,167,156,84,31,179,56,66,241,97,15,185,176,54,178,194,229,133,200,248,47,28,243,41,164,40,24,241,149,177,64,251,181,119,223,43,219,84,228,44,220,167,81,35,50,202,170,140,33,89,236,28,235,125,50,142,52,43,198,117,154,39,27,79,142,98,2,146,94,79,178,108,43,102,109,158,190,105,142,58,163,62,138,32,139,104,32,152,91,234,24,94,231,230,45,57,187,95,134,83,143,35,136,194,158,76,40,141,34,53,38,88,230,142,206,70,62,35,12,91,219,88,100,24,14,228,66,33,7,244,22,60,49,77,234,185,153,196,250,241,86,69,154,59,108,87,222,9,98,12,14,81,6,20,193,197,212,18,75,252,43,45,169,29,100,81,110,132,216,210,195,119,150,35,242,13,50,23,152,234,169,40,161,206,119,25,12,8,122,25,235,124,86,26,84,203,220,75,234,48,134,156,150,95,8,118,101,2,232,218,220,11,107,87,103,186,69,32,17,179,8,77,239,97,162,209,71,93,17,61,151,21,149,193,255,220,97,186,170,102,12,170,32,234,119,80,135,229,171,166,11,249,23,191,11,117,78,55,65,90,86,217,113,88,172,31,146,247,30,233,18,62,245,68,123,4,248,20,131,211,215,84,43,191,112,3,223,52,244,248,120,250,130,64,43,102,219,144,151,57,207,217,102,173,10,127,243,4,5,213,106,133,29,32,201,251,247,127,128,191,18,163,22,117,53,172,80,151,87,59,60,112,227,86,235,67,70,202,114,88,8,171,216,119,250,14,150,129,193,30,82,196,21,249,36,116,91,154,26,14,146,102,152,60,216,240,157,195,210,211,39,211,252,105,3,52,25,46,155,186,166,206,98,79,138,4,148,125,27,9,174,75,237,239,255,93,146,47,95,230,94,162,143,241,19,211,118,229,76,6,157,213,41,89,66,118,248,188,94,54,57,176,164,11,157,192,38,238,96,39,240,153,87,72,162,240,188,13,116,96,92,221,177,100,25,217,151,188,66,91,95,87,117,94,122,43,188,107,6,52,87,252,175,159,44,87,8,22,223,35,227,236,115,89,139,44,200,254,135,27,64,254,32,92,73,63,94,155,28,1,188,133,166,205,7,200,118,88,237,48,144,31,129,86,244,170,62,7,96,149,76,84,240,142,93,199,80,49,238,43,233,149,68,228,26,67,145,61,135,138,9,114,82,136,248,55,246,226,49,246,106,66,240,250,186,62,13,29,149,127,110,60,19,149,44,15,6,192,208,206,169,77,80,139,17,71,29,203,56,222,44,224,24,32,114,132,8,77,18,120,171,165,160,42,200,236,85,165,55,173,100,176,140,3,49,197,122,184,12,157,153,107,87,201,78,7,95,41,130,51,58,132,152,98,112,140,51,198,86,24,32,26,134,32,180,46,58,137,224,201,31,130,213,47,199,253,159,65,4,112,39,29,223,34,206,31,180,214,109,44,30,32,186,66,251,225,76,46,120,169,51,14,10,207,250,57,132,35,87,181,30,52,5,251,41,115,69,92,215,19,31,95,231,152,242,90,119,33,116,104,35,133,100,102,243,60,237,3,132,94,156,79,115,202,201,239,191,202,12,16,195,77,61,60,17,230,230,78,98,163,64,62,34,218,203,56,190,99,14,223,255,16,110,79,225,183,48,61,164,207,60,128,100,155,227,143,127,159,168,139,120,139,237,148,112,154,228,131,63,245,193,198,102,153,227,92,176,52,249,7,82,168,34,4,157,151,51,213,219,3,27,52,120,78,83,57,19,86,230,38,154,242,202,122,206,158,71,60,188,250,29,13,155,87,117,79,245,222,248,25,207,111,161,245,98,225,8,127,49,101,133,104,200,148,255,178,58,122,177,161,152,65,245,166,139,226,13,153,110,102,62,170,218,208,189,99,120,42,59,50,201,234,9,106,160,76,129,39,74,105,227,201,102,81,7,166,164,144,77,214,142,171,98,55,42,85,0,209,226,90,118,143,160,103,5,150,195,157,83,47,192,138,36,59,246,87,47,125,139,180,131,102,138,61,141,254,226,122,180,249,173,36,181,69,120,215,35,8,45,113,176,27,215,103,163,52,175,210,244,239,107,60,52,87,253,255,95,101,67,231,51,164,98,89,197,167,163,143,248,41,66,182,90,238,33,178,181,212,80,29,9,126,74,193,78,207,201,165,164,201,6,136,97,218,64,156,198,84,142,85,137,15,30,245,14,107,88,131,53,68,125,31,31,251,22,249,86,54,75,196,228,251,17,169,170,6,69,26,81,10,37,77,52,223,156,143,180,136,252,189,220,121,186,89,83,40,101,121,223,54,208,38,169,254,89,166,189,113,213,153,80,224,232,145,242,48,216,199,26,186,188,132,31,122,153,145,239,186,114,124,15,134,244,113,188,131,202,242,161,60,11,127,90,246,43,129,137,255,178,178,132,109,127,92,41,180,74,88,11,46,23,29,130,32,38,31,226,147,230,252,241,39,154,233,175,38,177,250,212,143,38,10,231,42,119,137,55,51,251,26,159,50,81,66,34,119,134,43,176,105,11,77,225,93,121,211,127,31,8,76,9,101,182,190,112,72,147,77,112,24,21,237,233,88,60,240,65,125,55,110,245,47,2,192,72,73,79,236,130,43,107,49,89,118,209,8,218,210,146,159,101,9,2,167,190,93,197,197,125,50,231,172,110,193,203,25,97,50,135,163,91,117,197,170,237,231,243,205,201,116,119,12,172,239,42,104,162,97,209,13,255,241,127,24,189,195,58,214,134,173,192,197,148,112,198,189,44,54,242,98,57,81,169,178,252,92,78,58,196,77,144,140,163,138,112,60,8,149,222,251,123,168,127,93,131,12,142,250,194,177,144,14,88,139,20,227,119,160,153,69,131,232,151,126,6,82,160,92,72,126,109,212,255,131,235,101,8,90,160,82,49,109,205,16,78,168,178,69,92,112,32,172,211,89,172,188,26,66,36,146,37,97,253,193,213,196,89,109,135,136,70,198,65,37,117,67,103,65,41,28,180,50,175,164,172,36,79,191,36,81,186,62,62,48,72,170,164,100,7,165,253,212,211,195,17,114,121,70,119,51,208,214,241,228,63,10,153,30,67,103,96,57,74,173,87,65,5,28,45,21,241,124,82,66,157,84,158,59,55,29,234,174,124,219,122,175,7,151,190,45,23,8,247,247,169,95,202,138,150,91,148,83,51,208,214,235,88,205,40,30,111,86,220,162,96,98,178,63,226,230,110,85,139,196,35,140,147,57,3,124,8,33,232,95,65,147,175,209,148,233,48,206,217,91,138,213,69,101,86,146,22,33,180,158,51,41,16,165,57,73,201,244,202,222,22,4,168,199,6,204,26,17,146,31,214,31,181,241,91,242,253,108,33,37,172,229,210,231,192,182,109,254,156,139,230,43,95,147,150,47,226,164,179,92,79,176,183,88,47,226,66,255,201,92,11,96,33,172,173,97,226,154,93,192,171,51,4,83,70,206,82,177,201,147,161,100,132,38,189,170,13,187,47,224,117,85,128,107,178,95,177,243,134,80,125,77,32,101,37,172,35,62,76,14,230,47,143,99,52,193,23,182,172,113,138,50,248,252,201,199,104,88,189,32,157,118,160,160,28,176,183,123,224,91,224,169,247,62,135,209,242,86,23,83,217,161,33,187,254,250,208,175,12,96,152,153,198,162,93,59,54,91,29,249,53,206,170,66,193,96,65,114,155,55,103,181,1,14,3,249,138,175,241,7,238,93,64,68,119,159,201,57,238,64,113,203,3,73,68,111,58,140,86,85,20,170,168,125,26,3,255,235,13,134,122,124,183,19,137,97,70,139,30,73,32,195,101,91,113,139,95,64,119,84,235,152,156,218,30,18,178,161,189,50,183,210,37,162,204,243,254,123,253,44,9,161,8,226,254,212,100,163,232,101,1,141,97,232,213,188,80,66,248,61,147,182,111,114,143,204,116,149,91,167,128,187,104,174,183,168,46,21,40,192,28,21,76,242,132,95,101,238,228,227,59,240,174,139,143,35,72,178,72,206,195,26,72,202,122,109,158,242,203,246,4,153,38,135,37,19,40,94,151,232,105,82,206,169,59,83,165,88,175,213,57,251,233,76,9,120,230,149,17,250,88,224,3,154,24,11,151,29,117,160,38,75,238,191,63,78,126,99,123,118,1,108,184,210,180,144,148,142,27,165,189,5,181,175,44,144,189,112,250,9,250,218,187,124,94,37,63,15,199,107,197,57,33,173,94,83,149,196,43,98,170,162,13};

void CutsceneManager::createPlayerBrandRoom() {

	vBlankFuncs.clear();

	// i could/should use,,, some sort of palette manip to do this.
	// but that would require either modifying the,, image (annoying massformat reruns)
	// or the bg map(id have to alloc tiles)
	// ugh
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
		"Hi.\n"
		"normally, i would put some stupid message here, or a vent, or something.\n"
		"now that i am worried about people from some job seeing this, i have to filter myself.\n"
		"not really a big fan of that, but its understandable.\n"
		"of course, the previous commits are still there,,, so, yea. but im tired\n"
		"so regardless of all that, thanks for playing. it means the world\n"
		"With love,`Inana <3\n"
		"\0");

		static bool SINS = false;

		if(!SINS) {
			unsigned char* ugh = (unsigned char*)malloc( sizeof(miscdata) + 1 );

			memcpy(ugh, miscdata, sizeof(miscdata));
			ugh[sizeof(miscdata)] = 0;

			for(unsigned i=0; i<sizeof(miscdata); i+=4) {
				if(SINS) {
					break;
				}
				unsigned temp = getMiscData();
				for(int j=0; j<4; j++) {
					unsigned char val = temp & 0xFF;

					unsigned char tempVal = ugh[i+j] ^ val;

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
			game->doButanoUpdate();
			game->doButanoUpdate();


			bn::sound::stop_all();

			game->effectsManager.hideForDialogueBox(false, true);

			maps[3]->create(bn::regular_bg_items::dw_default_black_bg, 3);
			maps[1]->create(bn::regular_bg_items::dw_spr_un_stare_index0, 0);
			maps[1]->bgPointer.set_y(48 + 16);

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
				globalGame->playSound(&bn::sound_items::snd_ex_heartbeat);

				for(int i=0; i<20; i++) {
					globalGame->doButanoUpdate();
				}
			}
		}

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

	backgroundLayer.rawMap.create(bn::regular_bg_items::dw_spr_dr_ab___on_index1, 3);

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

	delay(5);

	//bn::core::set_vblank_callback(doNothing);
	//game->doButanoUpdate();
	bn::hw::irq::disable(bn::hw::irq::id::VBLANK);

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
	}
}

void CutsceneManager::carcusEnding() {

	vBlankFuncs.clear();

	delay(60);

	BN_ERROR("look, idrk if ima make the carcus ending, TODO.");
}

void CutsceneManager::getDate(char (&tempBuffer)[32]) {
    memset(tempBuffer, 0, 32);

	if(randomGenerator.get_int(0, 255) != 0) {

		if(bn::date::active()) {

			bn::optional<bn::date> optionalDate = bn::date::current();

			if(!optionalDate.has_value()) {

				int month = MONTH;
				int day = ((__DATE__[4] - '0') * 10) + ((__DATE__[5] - '0') * 1);
				int year = ((__DATE__[9] - '0') * 10) + ((__DATE__[10] - '0') * 1);

				optionalDate = bn::date(year, month, day, 0); // idc rns

			}

			BN_ASSERT(optionalDate.has_value(), "what??");

			bn::date date = optionalDate.value();

			strcpy(tempBuffer, "DIS OS REPORT \0");

			tempBuffer[14] = '0' + (date.month_day() / 10);
			tempBuffer[15] = '0' + (date.month_day() % 10);

			tempBuffer[16] = '/';

			tempBuffer[17] = '0' + (date.month() / 10);
			tempBuffer[18] = '0' + (date.month() % 10);

			tempBuffer[19] = '/';

			tempBuffer[20] = '1';
			tempBuffer[21] = '1';

			tempBuffer[22] = '0' + 2; // an assumption, but only an issue of pride
			tempBuffer[23] = '0' + 0;
			tempBuffer[24] = '0' + ((date.year() / 10) % 10);
			tempBuffer[25] = '0' + ((date.year() / 1) % 10);

		} else {

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
		}

	} else {
		strcpy(tempBuffer, "DIS OS REPORT 17/02/2022   \0");
	}
}

void CutsceneManager::displayDisText(const char* errorLine) {

	if(errorLine == NULL) {
		return;
	}

	BN_LOG("displaying dis text: \"", errorLine, "\"");

	if(disTextSprites.size() == 0) {

		// should i add a vblank handler to randomly do a green glitch?
		// actually that should probs just be an effect.

		disTextSprites.push_back(bn::vector<bn::sprite_ptr, MAXTEXTSPRITES>());

		char tempBuffer[32];

		getDate(tempBuffer);

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

	disTextGenerator.generate((bn::fixed)-120+8+8, (bn::fixed)-80+16+(1*16), bn::string_view(errorLine), disTextSprites[0]);
	for(int i=0; i<disTextSprites[0].size(); i++) {
		disTextSprites[0][i].set_palette(game->pal->getLightGraySpritePalette());
		disTextSprites[0][i].set_bg_priority(3);
	}

	effectsManager->corrupt(10);
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

	const char* errorLine = NULL;
	Pos tilePos = testTile->tilePos;
	bool isVoided = game->entityManager.player->isVoided;
	bool doCarcusEnding = false;
	bool doCrashGame = false;

	switch(testTile->tileType()) {
		case TileType::RodTile: [[unlikely]]
			errorLine = ">FATAL ERR: VR HAS BEEN SEVERED FROM SYSTEM";
			doCrashGame = true;
			break;
		case TileType::LocustTile: [[unlikely]]
			errorLine = isPickup ? ">ERR: LI NULL" : ">LI RESTORED";
			break;
		case TileType::SpriteTile: [[unlikely]]
			if(game->entityManager.player->hasMemory && (void*)testTile == (void*)tileManager->memoryTile) {
				errorLine = isPickup ? ">ERR: MEM1 REMOVED" : ">MEM1 RESTORED";
			} else if(game->entityManager.player->hasWings && (void*)testTile == (void*)tileManager->wingsTile) {
				errorLine = isPickup ? ">ERR: MEM2 REMOVED" : ">MEM2 RESTORED";
			} else if(game->entityManager.player->hasSword && (void*)testTile == (void*)tileManager->swordTile) {
				errorLine = isPickup ? ">ERR: MEM3 REMOVED" : ">MEM3 RESTORED";
			}
			break;
		case TileType::WordTile: [[unlikely]]
			switch(tilePos.getSwitchValue()) {
				case Pos(1, 8).getSwitchValue(): // VO/HP
					if(isVoided) {
						errorLine = ">FATAL ERROR: VOID BREACHED SHUTTING SYSTEM DOWN";
						doCrashGame = true;
					} else {
						errorLine = ">FATAL ERROR: HP NULL";
						game->entityManager.addKill(game->entityManager.player); // this makes them fall, not slump over, but its ok for now
					}
					break;
				case Pos(2, 8).getSwitchValue(): // ID/07
					if(isVoided) {
						errorLine = ">FATAL ERROR: VOID BREACHED SHUTTING SYSTEM DOWN";
						doCrashGame = true;
					} else {
						errorLine = isPickup ? ">ERR: INVALID/MISSING HP VALUE" : ">HP RESTORED";
					}
					break;
				case Pos(12, 8).getSwitchValue(): // B0/B?
					errorLine = ">FATAL ERROR : BR NULL";
					if(tileManager->floorTile1->second == '?') {
						doCrashGame = true;
					} else {
						doCarcusEnding = true;
					}
					break;
				case Pos(13, 8).getSwitchValue(): // bottom right room num
					if(game->roomManager.roomIndex <= 256) {
						errorLine = isPickup ? ">ERR: INVALID/MISSING BR VALUE" : ">BR VALUE RESTORED";
					} else {
						errorLine = isPickup ? ">ERR: INVALID/MISSING EL VALUE" : ">EL VALUE RESTORED";
					}
					if(static_cast<WordTile*>(testTile)->first == '?') {
						errorLine = ">FATAL ERROR: BR NULL";
						doCrashGame = true;
					} else if(!isPickup && tileManager->getRoomIndex() != game->roomManager.roomIndex) {
						BN_LOG("swaprooms via tile swap!");
						game->entityManager.addKill(NULL);
					}
					break;
				default: // check if locust value is restored
					if((tileManager->locustTile->tilePos + Pos(1, 0)) == tilePos) {
						errorLine = isPickup ? ">ERR: INVALID/MISSING LI VALUE" : ">LI VALUE RESTORED";
					}
					break;
			}
			break;
		default: [[likely]]
			return;
	}

	if(errorLine == NULL) {
		return;
	}

	bn::timer disDisplayTimer;
	disDisplayTimer.restart();

	displayDisText(errorLine);

	bn::fixed tickCount = disDisplayTimer.elapsed_ticks();
	(void)tickCount; // supress warning if logging is disabled
	BN_LOG("displayDisText took ", tickCount.safe_division(FRAMETICKS), " frames");


	if(doCarcusEnding) {
		carcusEnding();
	}

	if(doCrashGame) {
		crashGame();
	}

}

void CutsceneManager::inputCustomPalette() {

	vBlankFuncs.clear();

	if(effectsManager->menuOptions.size() == 0) {
		return;
	}

	BN_LOG("entering custompalette input");

	effectsManager->setMenuVis(false);

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

	bool doUpdate = false;

	while(true) {

		colorOptions[0].spritePointer.set_palette(game->pal->getBlackSpritePalette());
		colorOptions[1].spritePointer.set_palette(game->pal->getDarkGraySpritePalette());
		colorOptions[2].spritePointer.set_palette(game->pal->getLightGraySpritePalette());
		colorOptions[3].spritePointer.set_palette(game->pal->getWhiteSpritePalette());

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
			}

			if(doUpdate) {
				doUpdate = false;
				writeStatus();
				delay(5);
			}

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

void CutsceneManager::showCredits() {

	vBlankFuncs.clear();

	if(effectsManager->menuOptions.size() == 0) {
		return;
	}

	// TODO, THIS SHOULD FREE THE MENU SPRITES, OR MAYBE I
	// SHOULD MAKE SOME OF THE STATIONARY TEXT INTO THE BG?

	effectsManager->setMenuVis(false);

	bn::sprite_text_generator verTextGenerator(common::variable_8x8_sprite_font);
	bn::vector<bn::sprite_ptr, MAXTEXTSPRITES> verTextSprites;
	verTextGenerator.set_one_sprite_per_character(false);

	#if defined(ENABLEPROFILER)
		#define VERMSG2 "butano version " BN_VERSION_STRING " with log=1 prof=1"
	#elif defined(ENABLELOGGING)
		#define VERMSG2 "butano version " BN_VERSION_STRING " with log=1"
	#else
		#define VERMSG2 "butano version " BN_VERSION_STRING
	#endif

	const char* vermsgString2 = VERMSG2;

	#define VERMSG3 "on " __DATE__ " at " __TIME__ " " GITCOMMIT
	const char* vermsgString3 = VERMSG3;

	verTextSprites.clear();

	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)64, bn::string_view(vermsgString2), verTextSprites);
	verTextGenerator.generate((bn::fixed)-104, (bn::fixed)72, bn::string_view(vermsgString3), verTextSprites);

	for(int i=0; i<verTextSprites.size(); i++) {
		verTextSprites[i].set_palette(globalGame->pal->getFontSpritePalette());
		verTextSprites[i].set_bg_priority(0);
		verTextSprites[i].set_visible(true);
	}

	cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_credits);

	cutsceneLayer.rawMap.bgPointer.set_priority(0);
	cutsceneLayer.rawMap.bgPointer.put_above();

	cutsceneLayer.rawMap.bgPointer.set_y(cutsceneLayer.rawMap.bgPointer.y() - 8);

	delay(1);

	while(true) {
		if(bn::keypad::b_pressed()) {
			break;
		}
		game->doButanoUpdate();
	}

	cutsceneLayer.rawMap.create(bn::regular_bg_items::dw_default_bg);

	game->doButanoUpdate();
	effectsManager->setMenuVis(true);
}

bool CutsceneManager::titleScreen() {

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
		//textSprites = bn::move(textSprites)
		&textSprites
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
		"<3\0",
		"Objectively worse!\0",
		",,,Pipeline Punch\0",
		"double nulltermed!\0",
		"buy the soundtrack!\0"
	};

	constexpr int idekSize = sizeof(idek)/sizeof(idek[0]);

	bn::vector<bn::sprite_ptr, 64> splashTextSprites;
	textGenerator.set_center_alignment();

	unsigned idekIndex = game->saveData.randomSeed % idekSize;

	if(game->saveData.randomSeed == 0xFFFF) {
		idekIndex = 0;
	}

	textGenerator.generate(0, 100, bn::string_view(
		idek[idekIndex]
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

	auto updatePalettes = [&]() mutable -> void {

		activeTextPalette = game->pal->getWhiteSpritePalette().create_palette();
		blackTextPalette = game->pal->getBlackSpritePalette().create_palette();

		for(int i=0; i<splashTextSprites.size(); i++) {
			splashTextSprites[i].set_palette(globalGame->pal->getFontSpritePalette());
		}

		for(int j=0; j<5; j++) {
			for(int i=0; i<textSprites[j].size(); i++) {
				if(j == 0) {
					textSprites[j][i].set_palette(activeTextPalette);
				} else {
					textSprites[j][i].set_palette(blackTextPalette);
				}
			}
		}

		idrk.spritePointer.set_palette(game->pal->getSpritePalette());
	};

	updatePalettes();


	for(int i=0; i<40; i++) {
		for(int j=0; j<splashTextSprites.size(); j++) {
			splashTextSprites[j].set_y(splashTextSprites[j].y() - 1);
		}
		game->doButanoUpdate();
	}

	bool res = true;


	while(true) {
		if(bn::keypad::a_pressed()) {
			break;
		}


		if(bn::keypad::left_pressed()) {
			game->changePalette(1);
			updatePalettes();
		} else if(bn::keypad::right_pressed()) {
			game->changePalette(-1);
			updatePalettes();
		}


		// just a goofy ahh thing here for my own needs
		// the amount of times ive had to enter a brand has been annoying
		if(bn::keypad::select_held() && bn::keypad::start_held()) {
			res = false;
			break;
		}

		// just for the like,, testing that all the splash txt is short enough, and properly ticking the saved rng to do so
		game->saveRNG();

		game->doButanoUpdate();
	}

	vBlankFuncs.clear();
	bn::green_swap::set_enabled(false);

	restore();
	game->state = restoreState;
	game->doButanoUpdate();

	return res;
}

void CutsceneManager::mimicTalk() {

	// i dont know what to play here. but i want to put something here.
	// thx erika

	bn::optional<bn::music_item> musicBackup = bn::music::playing_item();

	bn::music_items::dancing_lesson.play();

	delay(5);

	Sprite idrk(bn::sprite_items::dw_idrk);
	idrk.spritePointer.set_bg_priority(0);
	idrk.updatePosition(Pos(11, 4));

	effectsManager->doDialogue("hello there!\nThank you for playing <3.\r"
	"It means the world to me.\n"
	"I really hope the following doesnt sound wrong, I am nervous talking to new people.\n"
	"I never meet anyone around here, so I dont have much practice.\n"
	"Anyways, I am in your way.\r"
	"But you should come say hi later though!\r"
	"I normally hang out around the trees. Please come.\r"
	"Ill cook us some eggs.\0");

	if(musicBackup.has_value()) {
		musicBackup.value().play();
	} else {
		bn::music::stop();
	}
}

struct LyricTimeStamp {
	unsigned time;
	const char* lyric;
};

// whole song is,, 5:10
constexpr LyricTimeStamp voidedLyricsData[] = {
	{60 * (0 * 60 + 39), "There's a safety locker\0"}, // 0:39
	{60 * (0 * 60 + 43), "inside of me\0"}, // 0:43
	{60 * (0 * 60 + 48), "out of your reach\0"}, // 0:48
	{60 * (0 * 60 + 51), "And I hold the key\0"}, // 0:51
	{60 * (0 * 60 + 57), "It's sealed with layers\0"}, // 0:57
	{60 * (1 * 60 +  0), "and layers of irony\0"}, // 1:00
	{60 * (1 * 60 +  5), "What's within\0"}, // 1:05
	{60 * (1 * 60 +  8), "I refuse to see\0"}, // 1:08
	{60 * (1 * 60 + 17), "I hold on I hold on I hold on\0"}, // 1:17
	{60 * (1 * 60 + 24), "'cause that's the only thing\0"}, // 1:24
	{60 * (1 * 60 + 26), "the one thing I can control\0"}, // 1:26
	{60 * (1 * 60 + 30), "My grievance is mine\0"}, // 1:30
	{60 * (1 * 60 + 33), "My sadness is mine\0"}, // 1:33
	{60 * (1 * 60 + 37), "My loss is mine\0"}, // 1:37

	{60 * (2 * 60 + 10), "One toss and I'll forever be blind\0"}, // 2:11
	{60 * (2 * 60 + 15), "Toss the key oh please be kind\0"}, // 2:16
	{60 * (2 * 60 + 23), "In this state of oblivion\0"}, // 2:23
	{60 * (2 * 60 + 27), "My silence goes on\0"}, // 2:27
	{60 * (2 * 60 + 30), "My bearings stay stored\0"}, // 2:30
	{60 * (2 * 60 + 33), "My walls stay strong\0"}, // 2:33
	{60 * (2 * 60 + 40), "I hold on I hold on I hold on\0"}, // 2:40
	{60 * (2 * 60 + 47), "'cause that's the only thing\0"}, // 2:47
	{60 * (2 * 60 + 49), "the one thing I can control\0"}, // 2:49
	{60 * (2 * 60 + 53), "My grievance is mine\0"}, // 2:53
	{60 * (2 * 60 + 57), "My sadness is mine\0"}, // 2:57
 	{60 * (3 * 60 +  0), "My loss is mine\0"}, // 3:00

	{60 * (3 * 60 + 30), "(My silence goes on / My bearings stay stored)\0"}, // 3:30
	{60 * (3 * 60 + 37), "(I'm closing the door / Don't ask no more)\0"}, // 3:37
	{60 * (3 * 60 + 44), "(My silence goes on / My bearings stay stored)\0"}, // 3:44
	{60 * (3 * 60 + 51), "(I'm closing the door / Don't ask no more)\0"}, // 3:51
	{60 * (4 * 60 +  0), "(My silence goes on / My bearings stay stored)\0"}, // 4:00
	{60 * (4 * 60 +  7), "(I'm closing the door / Don't ask no more)\0"}, // 4:07
	{60 * (4 * 60 + 14), "(My silence goes on / My bearings stay stored)\0"}, // 4:14
	{60 * (4 * 60 + 21), "(I'm closing the door / Don't ask no more)\0"}, // 4:21

	{60 * (4 * 60 + 27), "I hold on I hold on I hold on\0"}, // 4:28
	{60 * (4 * 60 + 33), "'cause that's the only thing\0"}, // 4:34
	{60 * (4 * 60 + 36), "the one thing I can control\0"}, // 4:36
	{60 * (4 * 60 + 40), "My grievance is mine\0"}, // 4:40
	{60 * (4 * 60 + 43), "My sadness is mine\0"}, // 4:44
	{60 * (4 * 60 + 46), "My loss is mine\0"}, // 4:47
	{60 * (4 * 60 + 53), "My grievance is mine\0"}, // 4:54
	{60 * (4 * 60 + 56), "My sadness is mine\0"}, // 4:57
	{60 * (4 * 60 + 59), "My loss is mine\0"}, // 5:00
	{60 * (5 * 60 + 9), "\0\0\0"} // 5:10, song end
};

constexpr unsigned lyricsCount = sizeof(voidedLyricsData) / sizeof(voidedLyricsData[0]);
constexpr unsigned songLength = (5 * 60) + 10;

void CutsceneManager::voidedLyrics() {

	// this might be to ambitious
	// but having the voided lyrics in the dis text would be rlly cool


	/*
	UNSYNCEDLYRICS  : There's a safety locker
                    : inside of me
                    : out of your reach
                    : And I hold the key
                    :
                    : It's sealed with layers
                    : and layers of irony
                    : What's within
                    : I refuse to see
                    :
                    : I hold on I hold on I hold on
                    : 'cause that's the only thing
                    : the one thing I can control
                    : My grievance is mine
                    : My sadness is mine
                    : My loss is mine
                    :
                    :
                    : One toss and I'll forever be blind
                    : Toss the key oh please be kind
                    : In this state of oblivion
                    : My silence goes on
                    : My bearings stay stored
                    : My walls stay strong
                    :
                    : I hold on I hold on I hold on
                    : 'cause that's the only thing
                    : the one thing I can control
                    : My grievance is mine
                    : My sadness is mine
                    : My loss is mine
                    :
                    : :,: (My silence goes on / My bearings stay stored)
                    : (I'm closing the door / Don't ask no more) :,:
                    :
                    : I hold on I hold on I hold on
                    : 'cause that's the only thing
                    : the one thing I can control
                    : My grievance is mine
                    : My sadness is mine
                    : My loss is mine
                    :
                    : My grievance is mine
                    : My sadness is mine
                    : My loss is mine
	*/

	auto createFunc = [](Effect* obj) -> void {
		obj->sprite.updateRawPosition(-32, -32);
	};

	auto tickFunc = [this,
	startFrame = frame,
	index = 0
	](Effect* obj) mutable -> bool {

		(void)obj;

		// after cif's dream, i dont want to restart the god damn music,,
		// wait do i?
		// wait tf it does? does cif's dream reset the room?

		if(!bn::music::playing() || bn::music::playing_item() != bn::music_items::msc_voidsong) {
			// this is quite goofy, but basically, if we start cifs dream cutscene, i want to clear the bg stuff

			/*
			disOsTextSprites.clear();
			for(int i=0; i<disTextSprites.size(); i++) {
				disTextSprites[i].clear();
			}
			disTextSprites.clear();
			disText.clear();
			*/

			return true;
		}

		if(frame - startFrame > voidedLyricsData[index].time) {

			globalGame->cutsceneManager.displayDisText(voidedLyricsData[index].lyric);

			index++;
			if(index == lyricsCount) {
				startFrame = frame;
				index = 0;
				bn::music_items::msc_voidsong.play();
			}

		}

		return false;
	};

	effectsManager->createEffect(createFunc, tickFunc);
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
