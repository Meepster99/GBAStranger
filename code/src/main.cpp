
#include "SharedTypes.h"

#include "Game.h"

#include "bn_hw_irq.h"


//#include "ugba/ugba.h"

// https://github.com/GValiente/butano/blob/master/butano/hw/3rd_party/libugba/include/ugba/definitions.h

// 3 fucking hours.
// the interrupt. 
// is commented out.
// in https://github.com/GValiente/butano/blob/8e0c20f9264fe2d3eebeb1e198ad02878a025ed8/butano/hw/3rd_party/libugba/src/irq_handler.s#L106
// i need to speak to the manager.
// I AM ACTUALLY SO FUCKING PISSED.
// moving the func to game.cpp(actually maybe not)

unsigned* stareTiles = NULL;
unsigned short* stareMap = NULL;
int* stareTilesCount = 0;
int* stareMapCount = 0;

unsigned* glitchTiles = NULL;
//unsigned short* glitchMap = NULL; // we dont actually care abt this
int* glitchTilesCount;

unsigned short* col0 = NULL;
unsigned short* col1 = NULL;
unsigned short* col2 = NULL;
unsigned short* col3 = NULL;
unsigned short* col4 = NULL;


__attribute__((section(".iwram"))) unsigned short bruhRand() {
	const uint64_t a = 6364136223846793005;
	static uint64_t seed = 1;
    seed = a * seed + 1;
	
	unsigned short res = (seed >> 48);
	res ^= ((seed >> 32) & 0xFFFF);
	return res;
    //return (unsigned short)(seed >> 32);
}

// cheese and rice, at least i dont have to write asm
__attribute__((noinline, optimize("O0"), target("arm"), section(".iwram"))) void _cartPull() {
	
	// literally just copied from cutscenemanager 
	// i hope all these vars are stored in iwram
	
	// ok, i now see that i basically will have no access to butano for the duration of this.
	// this is gonna suck
	// ok,,, i cant even have two different things in a section??
	// thought i could maybe be weird with it and just put a bgtiles into ram, nope 
	
	// dw_spr_un_stare_index0_bn_gfx
	// i can change its .section to iwram.
	// im about to commit a fucking crime 
	// the thing is,,, why does like, 
	// i rlly dont want to have to mod that thing on the fly oh gods, esp since its compiled after massformater
	// i could maybe just copy it into iwram?
	// also,, oh gods 
	//those tiles being flip reduced,..
	// im about to commit so many crimes
	// it seems that, gods putting it in iwram may actually make this extremely easy, and 
	// im lazy as fuck.
	// you might have to compile twice on a clean compile lmao, to change the files
	// or, i could alloc them in iwram at runtime. 
	// llmao, we ball
	
	// https://problemkaputt.de/gbatek-gba-memory-map.htm
	/*
	VRAM, OAM, and Palette RAM Access
	These memory regions can be accessed during H-Blank or V-Blank only
	(unless display is disabled by Forced Blank bit in DISPCNT register).
	There is an additional restriction for OAM memory: Accesses during 
	H-Blank are allowed only if 'H-Blank Interval Free' in DISPCNT is set
	(which'd reduce number of display-able OBJs though).
	The CPU appears to be able to access VRAM/OAM/Palette at any
	time, a waitstate (one clock cycle) being inserted automatically in 
	case that the display controller was accessing memory simultaneously. 
	(Ie. unlike as in old 8bit gameboy, the data will not get lost.)
	*/
	*(reinterpret_cast<unsigned short*>(0x04000000)) |= 0b0000000010000000;
	
	//dw_spr_un_stare_index0_bn_gfxTiles
	//dw_spr_un_stare_index0_bn_gfxMap
	//dw_spr_un_stare_index0_bn_gfxPal
	
	// now that i think abt it, i could of done this in bitmap mode. 
	//omfg
	
	// setup palette 
	volatile unsigned short* palettePointer = reinterpret_cast<volatile unsigned short*>(0x05000000);
	// this array seems to be declared on the fucking rom. this is really fuckingbad
	// this needs to be redone in the future!
	
	//unsigned short col0 = palettePointer[512-16];
	//unsigned short col1 = palettePointer[512-15];
	//unsigned short col2 = palettePointer[512-14];
	//unsigned short col3 = palettePointer[512-13];
	//unsigned short col4 = palettePointer[512-12];
	
	for(int i=0; i<1024/2; i++) {
		switch(i % 16) {
			case 0:
				palettePointer[i] = *col0;
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
	
	// set up bg offsets 
	// https://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
	//                                                   FEDCBA9876543210
	*(reinterpret_cast<unsigned short*>(0x04000008)) = 0b0000001100000000; // add
	*(reinterpret_cast<unsigned short*>(0x0400000A)) = 0b0000010000000101; // glitch bg 1
	*(reinterpret_cast<unsigned short*>(0x0400000C)) = 0b0000010100000110; // glitch bg 2
	*(reinterpret_cast<unsigned short*>(0x0400000E)) = 0b0000011000001011; // black bg
	
	// https://problemkaputt.de/gbatek-lcd-i-o-bg-scrolling.htm
	*(reinterpret_cast<unsigned short*>(0x04000010)) = 0x01F8;
	*(reinterpret_cast<unsigned short*>(0x04000012)) = 0x01F0;
	
	*(reinterpret_cast<unsigned short*>(0x04000014)) = 0x0000;
	*(reinterpret_cast<unsigned short*>(0x04000016)) = 0x0000;
	
	*(reinterpret_cast<unsigned short*>(0x04000018)) = 0x0000;
	*(reinterpret_cast<unsigned short*>(0x0400001A)) = 0x0000;
	
	*(reinterpret_cast<unsigned short*>(0x0400001C)) = 0x0000;
	*(reinterpret_cast<unsigned short*>(0x0400001E)) = 0x0000;
	
	// fun fact, tile base addr isnt properly displayed in mgba.
	volatile unsigned short* mapPtr = reinterpret_cast<unsigned short*>(0x06000000 + (2 * 3 * 1024));
	volatile unsigned* tilesPtr = reinterpret_cast<unsigned*>(0x06000000 + (0 * 16 * 1024));

	// the iterate values should be gotten dynamically from pointer subtraction!!
	
	for(int i=0; i<*stareMapCount/2; i++) {
		mapPtr[i] = stareMap[i] | 0b0001000000000000;
	}
	
	for(int i=0; i<*stareTilesCount/4; i++) {
		tilesPtr[i] = stareTiles[i];
	}
	
	mapPtr = reinterpret_cast<unsigned short*>(0x06000000 + (2 * 4 * 1024));
	volatile unsigned short* mapPtr2 = reinterpret_cast<unsigned short*>(0x06000000 + (2 * 5 * 1024));
	tilesPtr = reinterpret_cast<unsigned*>(0x06000000 + (1 * 16 * 1024));
	
	unsigned short maxTile = *glitchTilesCount/(4*8);
	unsigned short temp;
	unsigned short tileIndex;
	for(int i=0; i<*stareMapCount/2; i++) {
		// https://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		temp = bruhRand() & 0x0FFF;
		
		tileIndex = (temp & 0x01FF) % maxTile;
		if(tileIndex == 0) { // one extra attempt
			temp = bruhRand() & 0x0FFF;
			tileIndex = (temp & 0x01FF) % maxTile;
		}
		temp = (temp & ~0x01FF) | tileIndex;
		
		mapPtr[i] = temp;
		
		temp = bruhRand() & 0x0FFF;
		
		tileIndex = (temp & 0x01FF) % maxTile;
		if(tileIndex == 0) { // one extra attempt
			temp = bruhRand() & 0x0FFF;
			tileIndex = (temp & 0x01FF) % maxTile;
		}
		temp = (temp & ~0x01FF) | tileIndex;
		
		mapPtr2[i] = temp;
	}
	
	for(int i=0; i<*glitchTilesCount/4; i++) {
		tilesPtr[i] = glitchTiles[i];
	}
	
	tilesPtr = reinterpret_cast<unsigned*>(0x06000000 + (2 * 16 * 1024));
	mapPtr = reinterpret_cast<unsigned short*>(0x06000000 + (2 * 6 * 1024));
	
	for(int i=0; i<8; i++) {
		tilesPtr[i] = 0x11111111;
	}
	
	for(int i=0; i<*stareMapCount/2; i++) {
		mapPtr[i] = 0xF000;
	}
	
	// instead of figureing out oam, im deadass just going to overwrite all sprite vram lmao
	// 06010000 - 06017FFF , 0x8000 bytes
	tilesPtr = reinterpret_cast<unsigned*>(0x06010000);
	
	for(int i=0; i<0x8000/4; i++) {
		tilesPtr[i] = 0;
	}
	
	*(reinterpret_cast<volatile unsigned short*>(0x04000000)) &= ~0b0000000010000000;
	
	unsigned short VCOUNT = 0;
	volatile unsigned short* greenswap = reinterpret_cast<volatile unsigned short*>(0x04000002);
	unsigned greenFrames = 0;
	bool frameStarted = false;
	unsigned short greenSwapState = 0xFFFF;
	while(true) {
		
		VCOUNT = *(reinterpret_cast<volatile unsigned short*>(0x04000006));
		
		if(VCOUNT == 0 && !frameStarted) {
			frameStarted = true;
			frame++;
			if(greenFrames > 0) {
				greenFrames--;
				palettePointer[16 + 2] = *col1;
			} else {
				*greenswap = 0;
				palettePointer[16 + 2] = *col2;
			}
		}
				
		if(greenFrames > 0) {
			if(VCOUNT == 0) {
				*greenswap = 1;
			}
			
			// dont move the eyes(now that the eyes are blinking, this aint rlly needed)
			if(VCOUNT == 36 && greenSwapState == 0xFFFF) {
				greenSwapState = *greenswap;
				*greenswap = 0;
			}
			
			if(VCOUNT == 37 && greenSwapState != 0xFFFF) {
				*greenswap = greenSwapState;
				greenSwapState = 0xFFFF;
			}
		}
		
		if(VCOUNT == 160) {
			*greenswap = 0;
			frameStarted = false;
			if(greenFrames == 0 && (bruhRand() & 0x7FF) == 0) {
				greenFrames = (bruhRand() & 0xF);
			}
		}
	}
}

int main() {
	
	bn::core::init(); 
	
	bn::bg_tiles::set_allow_offset(true);

	bn::hw::irq::set_isr(bn::hw::irq::id::GAMEPAK, _cartPull);
	bn::hw::irq::enable(bn::hw::irq::id::GAMEPAK);

	// copy bs into memory 

	col0 = new unsigned short(0x7FE0);
	col1 = new unsigned short(0x0000);
	col2 = new unsigned short(0x7FFF);
	col3 = new unsigned short(0x6318);
	col4 = new unsigned short(0x4210);
	
	stareTilesCount = new int(reinterpret_cast<int>(dw_spr_un_stare_index0_bn_gfxMap) - reinterpret_cast<int>(dw_spr_un_stare_index0_bn_gfxTiles));
	stareMapCount = new int(reinterpret_cast<int>(dw_spr_un_stare_index0_bn_gfxPal) - reinterpret_cast<int>(dw_spr_un_stare_index0_bn_gfxMap));
	glitchTilesCount = new int(dw_spr_glitchedsprites_bn_gfxTilesLen);
	
	stareTiles = (unsigned*)malloc(*stareTilesCount);
	stareMap = (unsigned short*)malloc(*stareMapCount);
	glitchTiles = (unsigned*)malloc(*glitchTilesCount);
	
	memcpy(stareTiles, dw_spr_un_stare_index0_bn_gfxTiles, *stareTilesCount);
	memcpy(stareMap, dw_spr_un_stare_index0_bn_gfxMap, *stareMapCount);
	memcpy(glitchTiles, dw_spr_glitchedsprites_bn_gfxTiles, *glitchTilesCount);
	
	Game* game = NULL;
	
	// this needs to be a pointer, or else i get scary stack errors.
	while(true) {
		game = new Game();
		game->run();
		delete game;
		game = NULL;
		bn::core::update();
		
		/*
		BN_LOG("bg_maps status");
		bn::bg_maps::log_status();
		BN_LOG("bg_tiles status");
		bn::bg_tiles::log_status();
		BN_LOG("sprite_tiles status");
		bn::sprite_tiles::log_status();
		BN_LOG("bg_palettes status");
		bn::bg_palettes::log_status();
		BN_LOG("sprite_palettes status");
		bn::sprite_palettes::log_status();
		BN_LOG("memory status");
		bn::memory::log_alloc_ewram_status();
		*/
	}
	
}
