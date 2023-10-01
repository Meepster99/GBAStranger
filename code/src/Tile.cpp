

#include "Tile.h"
#include "EffectsManager.h"

EffectsManager* FloorTile::effectsManager = NULL;
BackgroundMap* FloorTile::rawMap = NULL;

int Switch::pressedCount = 0;
int Switch::totalCount = 0;

// the gods created constexpr related errors to test mankind, and mankind failed.
// i should: be able to oneline this, be able to do it anonymously, and be able to inline it
EffectTypeArray glassAnimation[] = {EffectType(bn::sprite_tiles_items::dw_spr_glassfloor, 8)};
//constexpr bn::span<const bn::pair<const bn::sprite_tiles_item, int>> bruh(glassAnimation);

void Glass::stepOff() {
	if(isAlive) {
		effectsManager->createEffect(tilePos, 
		EffectTypeCast(glassAnimation));
	}
	isAlive = false;
}

