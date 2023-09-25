
#include "DebugText.h"

#include "Game.h"

void DebugText::updateText() {
		
	// i had to go into the butano source to find out that 
	// for some reason, i need to also clear the sprite array???
	// this took up so much fucking time 
	text_sprites.clear();
	debugString.clear();
	
	// https://gvaliente.github.io/butano/namespacebn_1_1core.html#aed64f380e43b831c5b6b4602a508f811
	// im at 1.4,, this is kinda bad,,, 
	// ig it doesnt matter to much? but still
	
	bn::fixed cpuUsage = bn::core::current_cpu_usage();
	stringStream.append(cpuUsage);
	
	stringStream.append("  ");
	
	int skippedFrames = bn::core::last_missed_frames();
	stringStream.append(skippedFrames);
	
	stringStream.append("  ");
	stringStream.append(game->entityManager.entityList.size());
	
	stringStream.append(" ");
	stringStream.append(game->roomManager.currentRoomName());
	
	stringStream.append(" ");
	stringStream.append(game->roomManager.roomIndex);
	
	debugText.generate(bn::fixed(-120), bn::fixed(-80+4), 
		bn::string_view(debugString.data()),
	text_sprites);
	
	// spaces arent counted as sprites! good idea, but not communicated
	
	// colors 
	
	Palette* cpuColor = &redText;
	if(cpuUsage < 1) {
		cpuColor = &greenText;
	}
	for(int i = 0; i<MIN(7, text_sprites.size()); i++) {
		text_sprites[i].set_palette(cpuColor->getSpritePalette());
	}
	
	cpuColor = &greenText;
	if(skippedFrames > 0) {
		cpuColor = &redText;
	}
	
	text_sprites[7].set_palette(cpuColor->getSpritePalette());

	cpuColor = &whiteText;
	text_sprites[8].set_palette(cpuColor->getSpritePalette());

}