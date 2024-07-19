#pragma once

#include "SharedTypes.h"
#include "EffectsManager.h"

//class TileManager;
class FloorTile;

#include <functional>

// decided this was big enough to make a whole new thing
// cutsceens, in order to be easily programable, will need to backup, and restore the all 4 bg layers
// and yea

class CutsceneLayer : public Layer {
public:

	// spr_vd_gray is a good start

	CutsceneLayer() : Layer(bn::regular_bg_items::dw_default_bg)
	{

	}

};

class BackgroundLayer : public Layer {
public:

	// spr_vd_gray is a good start

	BackgroundLayer() : Layer(bn::regular_bg_items::dw_default_bg, 3)
	//BackgroundLayer() : Layer(bn::regular_bg_items::dw_default_white_bg, 3)
	{

	}

};

namespace CutsceneManager {

	BN_DATA_EWRAM extern CutsceneLayer cutsceneLayer;
	BN_DATA_EWRAM extern BackgroundLayer backgroundLayer;
	BN_DATA_EWRAM extern BackgroundMap* maps[4];
	BN_DATA_EWRAM extern int zIndexBackup[4];
	BN_DATA_EWRAM extern int priorityBackup[4];
	BN_DATA_EWRAM extern bn::fixed xPosBackup[4];
	BN_DATA_EWRAM extern bn::fixed yPosBackup[4];
	BN_DATA_EWRAM extern bn::sprite_text_generator disTextGenerator;
	BN_DATA_EWRAM extern bn::vector<bn::sprite_ptr, 8> disOsTextSprites;
	BN_DATA_EWRAM extern bn::vector<bn::vector<bn::sprite_ptr, MAXTEXTSPRITES>, 7> disTextSprites;
	BN_DATA_EWRAM extern bn::vector<bn::regular_bg_map_ptr, 4> mapBackup;
	BN_DATA_EWRAM extern bn::vector<std::function<void()>, 8> vBlankFuncs;

	void CutsceneManager();

	void doVBlank();

	void resetRoom();

	__attribute__((optimize("O2"))) void introCutscene();

	__attribute__((optimize("O2"))) void cifDream();

	void brandInput();

	void createPlayerBrandRoom();

	void createResetRoom();

	void displayDisText(const char* errorLine);
	void disCrash(FloorTile* testTile, bool isPickup);
	void getDate(char (&tempBuffer)[32]);

	void crashGame();

	void carcusEnding();

	void inputCustomPalette();

	void showCredits();

	bool titleScreen();

	void mimicTalk();

	void voidedLyrics();

	// -----

	void backup(int i);

	void restore(int i);

	void backup();

	void restore();

	void delay(int delayFrameCount);

};
