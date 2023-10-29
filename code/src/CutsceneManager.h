#pragma once 

#include "SharedTypes.h"
#include "EffectsManager.h"

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

class CutsceneManager {
public:
	
	Game* game; 
	CutsceneLayer cutsceneLayer;
	EffectsManager* effectsManager;
	
	BackgroundMap* maps[4];
	int zIndexBackup[4];
	int priorityBackup[4];
	
	bn::fixed xPosBackup[4];
	bn::fixed yPosBackup[4];
	
	// these things cant be arrays bc i cant have a ref to the pointer, i need the pointer itself 
	//bn::vector<bn::regular_bg_tiles_ptr, 4> tilesBackup;
	bn::vector<bn::regular_bg_map_ptr, 4> mapBackup;
	
	CutsceneManager(Game* game_);

	bn::vector<std::function<void()>, 8> vBlankFuncs;
	
	void doVBlank() {
		for(int i=0; i<vBlankFuncs.size(); i++) {
			vBlankFuncs[i]();
		}
	}
	
	void introCutscene();
	
	void cifDream();
	
	
private:
	
	void freeLayer(int i);
	
	void backupLayer(int i);

	void restoreLayer(int i);
	
	void backupAllButEffects();
	
	void restoreAllButEffects();
	
	void delay(int delayFrameCount);

	
};


