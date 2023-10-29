

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
	
	game->effectsManager.doDialogue("still programming this in its actually pretty close to being finished, but just in case of crashes i commented it out.\0");
	
	return;
	
	BN_LOG("cifdream");
	vBlankFuncs.clear();
	
	if(game->effectsManager.bigSprites[0]->animationIndex != 0) {
		return;
	}
	
	/*
	game->effectsManager.doDialogue(""
	"[This Lotus-Eater Machine is still operational]\n"
	"[Maybe you could take a quick rest?]\n"
	"\0", false);
	
	bool res = game->effectsManager.restRequest();
	
	if(!res) {
		return;
	}
	
	
	delay(60);
	*/
	
	while(!game->effectsManager.zoomEffect(true, false)) {
		delay(1);
		//delay(20);
		BN_LOG("HEY FOOL, CHANGE THIS DELAY BACK TO 20");
	}
	
	GameState restoreState = game->state;
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
	
	game->effectsManager.doDialogue("Wake up.\0", true);
	
	effectsManager->effectsLayer.clear();
	
	maps[1]->create(bn::regular_bg_items::dw_default_black_bg, 3);
	
	// gml_Object_obj_cifdream_eyecatch_Create_0 contains the spawn indexes. (what is their origin?)
	Sprite mon(bn::sprite_tiles_items::dw_spr_cdream_mon, bn::sprite_shape_size(64, 32), 37  , 107  );
	Sprite bee(bn::sprite_tiles_items::dw_spr_cdream_bee, bn::sprite_shape_size(16, 16), 140 , 32 - 8  );
	Sprite gor(bn::sprite_tiles_items::dw_spr_cdream_gor, bn::sprite_shape_size(64, 32), 148 , 127  );
	Sprite tan(bn::sprite_tiles_items::dw_spr_cdream_tan, bn::sprite_shape_size(32, 32), 134 , 96   );
	Sprite lev(bn::sprite_tiles_items::dw_spr_cdream_lev, bn::sprite_shape_size(32, 32), 32  , 92   );
	Sprite cif(bn::sprite_tiles_items::dw_spr_cdream_cif, bn::sprite_shape_size(16, 16), 119 , 37 - 4   );
	cif.spritePointer.set_bg_priority(3);
	
	//Sprite cifGlow(
	
	// the more that i think of the positions of the lords, the more symbolism i see. its insane. i love this game sm
	
	maps[0]->create(bn::regular_bg_items::dw_spr_cdream_add_eus_b_index0, 2);
	maps[0]->bgPointer.set_x(74 + (64 / 2) - 8);
	maps[0]->bgPointer.set_y(10 + (132 / 2) - 16);
	
	vBlankFuncs.push_back([this, cif]() mutable {
		const bn::regular_bg_item* addBackgrounds[4] = {&bn::regular_bg_items::dw_spr_cdream_add_eus_b_index0, &bn::regular_bg_items::dw_spr_cdream_add_eus_b_index1, &bn::regular_bg_items::dw_spr_cdream_add_eus_b_index2, &bn::regular_bg_items::dw_spr_cdream_add_eus_b_index3};
		static int addBackgroundsIndex = 0;
		static int x = 74 + (64 / 2) - 8;
		static bn::fixed y = 10 + (132 / 2) - 16;
		const static bn::fixed yStart = 10 + (132 / 2) - 16;
		static int degree = 0;
		
		if(frame % 18 == 0) {
			maps[0]->create(*addBackgrounds[addBackgroundsIndex], 2);
			addBackgroundsIndex = (addBackgroundsIndex + 1) % 4;
		}
		
		bn::fixed temp = y + 0.15 * sinTable[degree % 360];
		
		bn::fixed tempDif = temp - yStart;
		if(tempDif > -3.0 && tempDif < 3.0) {
			y = temp;
			cif.setRawY(37 - 4 + tempDif);
		} else {
			degree+=3;
		}
		
		degree = (degree + 1) % 360;
		
		
		
		maps[0]->bgPointer.set_x(x);
		maps[0]->bgPointer.set_y(y);
		
		
		return;
	});
	
	
	// fade should be done here, i believe
	

	
	
	while(true) {
		delay(60*5);
	}

	/*
	maps[1]->create(bn::regular_bg_items::dw_spr_cdream_add_eus_b_index1, 3);
	maps[3]->create(bn::regular_bg_items::dw_spr_cdream_add_eus_b_index2, 3);
	//maps[1]->bgPointer.set_y(maps[1]->bgPointer.y() + 16 + 32);
	delay(60*5);
	*/
	
	// 
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



