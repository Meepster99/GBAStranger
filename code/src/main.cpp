

// ok so,, devkitARM-gdb 13.2-1

/*

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if __cplusplus != 202002L
	#pragma message ("c++ ver was: " STR(__cplusplus) " and should have been 202002L?")
#endif

#if __GNUC__ != 13
	#pragma message ("__GNUC__ was: " STR(__GNUC__) " and should have been 13?")
	
#endif

#if __GNUC_MINOR__ != 2
	#pragma message ("__GNUC_MINOR__ was: " STR(__GNUC_MINOR__) " and should have been 2?")
#endif

#if __GNUC_PATCHLEVEL__ != 0
	#pragma message ("__GNUC_PATCHLEVEL__ was: " STR(__GNUC_PATCHLEVEL__) " and should have been 0?")
#endif

*/

#include "SharedTypes.h"

#include "Game.h"

#include "bn_hw_irq.h"


// C:\devkitPro\devkitARM\arm-none-eabi\bin\objdump.exe -S -C -D .\build\main.o > idk.txt

// C:\devkitPro\devkitARM\bin\arm-none-eabi-gdb.exe
// file GBAStranger.elf
// target remote localhost:2345

// bash -c " git ls-files *.py *.cpp *.h | xargs cat | sed '/^\s*$/d' | wc -l"


//#include "ugba/ugba.h"

// https://github.com/GValiente/butano/blob/master/butano/hw/3rd_party/libugba/include/ugba/definitions.h

// 3 (curse)ing hours.
// the interrupt. 
// is commented out.
// in https://github.com/GValiente/butano/blob/8e0c20f9264fe2d3eebeb1e198ad02878a025ed8/butano/hw/3rd_party/libugba/src/irq_handler.s#L106
// i need to speak to the manager.
// I AM ACTUALLY SO (curse)ING PISSED.
// moving the func to game.cpp(actually maybe not)

// now that i think abt it,,, these things didnt have to be pointers,,, since globals are stored in ram.

unsigned* stareTiles = NULL;
unsigned short* stareMap = NULL;
int* stareTilesCount = 0;
int* stareMapCount = 0;

unsigned* glitchTiles = NULL;
int* glitchTilesCount = NULL;

unsigned short* col0 = NULL;
unsigned short* col1 = NULL;
unsigned short* col2 = NULL;
unsigned short* col3 = NULL;
unsigned short* col4 = NULL;

void doNothing() {
	
}

// i swear to the gods. this func was not supposed to become my default rand func. what am i smoking?
// should this be targeted to arm? unsure.

// WHY DOES THIS TARGET BEING ARM SOMEHOW MESS UP THE CARTPULL FUNC?????
__attribute__((section(".iwram"), target("thumb"))) unsigned short bruhRand() {
	// WHAT IS THIS NUMBER???
	const uint64_t a = 6364136223846793005ULL;
	
	// returning an unsigned short was dumb. thumb instrs are 16 bit, but in the actual ram/cpu i have 32 bit regs.
	
	//static uint64_t seed = 191847; // WHY THE HELL DID I HAVE THE SEED AT 1 FOR SO LONG????
	// actually,, most numbers dont give a good result, i was just lucky with this one?
	// maybe,,, bc it has only 2 prime factors,,, and the gcd between that and a is 3?
	// ill pick a big prime number
	// ok this thing fails to monobit, but passes everything else.
	// wait,,, oh no my testing output was only outputting unsigned short not unsigned
	//,,,,,,,, and now im bombing the tests. 
	// maybe i stick to returning unsigned shorts.
	
	static uint64_t seed = 4446193083169520393;
	
    seed = a * seed + 1;

	unsigned res = 0;
	res = seed;
	res ^= (seed >> 32);
	
	return res;
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
	// im about to commit a (curse)ing crime 
	// the thing is,,, why does like, 
	// i rlly dont want to have to mod that thing on the fly oh gods, esp since its compiled after massformater
	// i could maybe just copy it into iwram?
	// also,, oh gods 
	//those tiles being flip reduced,..
	// im about to commit so many crimes
	// it seems that, gods putting it in iwram may actually make this extremely easy, and 
	// im lazy as (curse).
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
	// this array seems to be declared on the (curse)ing rom. this is really (curse)ingbad
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
		if(tileIndex % 12 >= 6) { // one extra attempt
			temp = bruhRand() & 0x0FFF;
			tileIndex = (temp & 0x01FF) % maxTile;
		}
		temp = (temp & 0b0000110000000000) | tileIndex;
		
		mapPtr[i] = temp;
		
		temp = bruhRand() & 0x0FFF;
		
		tileIndex = (temp & 0x01FF) % maxTile;
		if(tileIndex % 12 >= 6) { // one extra attempt
			temp = bruhRand() & 0x0FFF;
			tileIndex = (temp & 0x01FF) % maxTile;
		}
		// why did i have to change this line? did something happen when i updated butano?
		temp = (temp & 0b0000110000000000) | tileIndex;
		
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

volatile unsigned miscPointer = 0;

__attribute__((noinline, optimize("O0"), target("arm"), section(".iwram"))) unsigned getMiscData() {
	
	// ,,,,, dont ask.
	// i could of, should of, and previously did this in a much easier way. but here i am now
	
	// also, this REALLY should of been written in thumb ugh
	
	unsigned res = 0;
	
	asm volatile(
   
	// PRAY
	
	"ldr r7, %[miscPointerAddr]\n" 
	"mov r6, #0\n"
	"mov r5, #0\n"
	
	"LOOP:\n"
	
	// can save a mov instr by just, anding r7 right into r0
	"and r0, r7, #3\n"
	"add r0, r0, #1\n"
	"orr r0, r0, #3\n"
	"sub r0, r7, r0\n"

	"mov r1, #168\n" 
	"mov r2, #0\n"
	"swi 0x1F << 16\n"

	"add r0, r0, #1\n"
	
	"add r6, r6, r0\n"
	
	"add r6, r6, r0, lsl #10\n"
	
	"add r6, r6, r0, lsr #5\n"
	

	"add r7, r7, #1\n"
	"add r5, r5, #1\n"

	"cmp r7, $0x4000\n"
	"bne CONT\n"
	
	"mov r7, #0\n"
	
	"CONT:\n"
	"cmp r5, #36\n" // <3
	"bne LOOP\n"

	"str r7, %[miscPointerAddr]\n"
	
	"add r6, r6, r6, lsl #3\n"
	"eor r6, r6, r6, lsr #11\n"
	"add r6, r6, r6, lsl #15\n"
	
	"str r6, %[result]\n"
	
	
    : // WHAT IS THIS???? THIS HAS TO BE THE DUMBEST SYNTAX OF ALL TIME?? // why does semicolon vs newline matter???
    : [result] "m" (res), [miscPointerAddr] "m" (miscPointer)
    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
	);
	
	return res;
}

int main() {
	
	bn::core::init(); 
	
	BN_LOG("butano inited");
	
	bn::fixed stackIWram = bn::memory::used_stack_iwram();
	bn::fixed staticIWram = bn::memory::used_static_iwram();
	bn::fixed totalIWram = stackIWram + staticIWram;
	bn::fixed totalEWram = bn::memory::used_static_ewram();
	
	BN_LOG("used_stack_iwram: ", stackIWram.safe_division(32 * 1024));
	BN_LOG("used_static_iwram: ", staticIWram.safe_division(32 * 1024));
	BN_LOG("total iwram: ", totalIWram.safe_division(32 * 1024));
	BN_LOG("total ewram: ", totalEWram.safe_division(256 * 1024));
	
	BN_ASSERT(totalIWram.safe_division(32 * 1024) < 1, "iwram overflow!!!");
	BN_ASSERT(totalEWram.safe_division(256 * 1024) < 1, "ewram overflow!!!");
	
	//BN_LOG("what the fuck ", 2[reinterpret_cast<unsigned char*>(0x02000050)]);
	
	/*
	
	[WARN] GBA Debug:	butano inited
	[WARN] GBA Debug:	used_stack_iwram: 0.01953
	[WARN] GBA Debug:	used_static_iwram: 0.81420
	[WARN] GBA Debug:	total iwram: 0.83374
		
	[WARN] GBA Debug:	butano inited
	[WARN] GBA Debug:	used_stack_iwram: 0.02709
	[WARN] GBA Debug:	used_static_iwram: 0.70678
	[WARN] GBA Debug:	total iwram: 0.73413
	
	
	*/
	
	bn::bg_tiles::set_allow_offset(true);

	bn::hw::irq::set_isr(bn::hw::irq::id::GAMEPAK, _cartPull);
	bn::hw::irq::enable(bn::hw::irq::id::GAMEPAK);

	// copy bs into memory 

	// copy colors into memory (5bitperchanel)
	// gba stores 
	//						  011111 11111 00000
	col0 = new unsigned short(0b0111111111100000);
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
	
	// dont cause a initial framedrop on boot.
	bn::core::update();
	

	Game* game = NULL;
	
	// this needs to be a pointer, or else i get scary stack errors.
	while(true) {
		game = new Game();
		
		// dont cause a initial framedrop on boot.
		bn::core::update();
	
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


