

#include "Tile.h"
#include "EffectsManager.h"

EffectsManager* FloorTile::effectsManager = NULL;

int Switch::pressedCount = 0;
int Switch::totalCount = 0;

void Glass::stepOff() {
	if(isAlive) {
		effectsManager->newEffect(new GlassBreak(tilePos));
	}
	isAlive = false;
}

