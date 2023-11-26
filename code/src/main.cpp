
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
__attribute__((section(".iwram"))) unsigned short bruhRand() {
	const uint64_t a = 6364136223846793005;
	static uint64_t seed = 1;
    seed = a * seed + 1;
	
	//unsigned short res = (seed >> 48);
	//res ^= ((seed >> 32) & 0xFFFF);
	//return res;
    //return (unsigned short)(seed >> 32);
	unsigned res = 0;
	res ^= ((seed & 0xFFFF) >> 0);
	//res ^= ((seed & 0xFFFF0000) >> 16);
	res ^= ((seed & 0xFFFF00000000) >> 32);
	res ^= ((seed & 0xFFFF000000000000) >> 48);
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

unsigned biosHash = 0;

__attribute__((noinline, optimize("O3"), target("arm"), section(".iwram"))) unsigned getBiosHash() {
	
	
	// wee woo wee woo 
	// https://problemkaputt.de/gbabios.htm
	// x = (MidiKey2Freq(y-(((y AND 3)+1)OR 3), 168, 0) * 2) SHR 24
	// https://problemkaputt.de/gbatek-bios-sound-functions.htm
	// getbiosbyte used to be a function, but to be 100000% sure that,,, it wasnt (guarenteed) being inlined, its in here now
	
	// unsigned test = i-(((i & 3)+1) | 3);
	
	// ,,,,, dont ask.
	
	/*
	unsigned res = 0;
	
    //unsigned test = 4294967293;
	unsigned test = 0xFFFFFFFD;
	
    unsigned temp;
	
	//for(unsigned i=0; i<0x4000; i++) {
	for(unsigned i=0; i<0x4000; i++) {
		
		//u8 temp = getBiosByte(i);
		
		test = i-(((i & 3)+1) | 3);
		
		asm(
            "mov r0, %[addr];"
            "mov r1, #168;"
            "mov r2, #0;"
            "swi 0x1F << 16;"
            "mov %[result], r0;"
            : [result] "=r" (temp)
            : [addr] "r" (test)
        );
		
		res += temp;
		res += (temp << 10);
		res ^= (temp >> 6);
	}
	
	res += (res << 3);
	res ^= (res >> 11);
	res += (res << 15);
	*/
	
	/*
	"and r2, r0, #3                                   ;"
	"add r2, r2, #1                                   ;"
	"orr r2, r2, #3                                   ;"
	"sub r2, r0, r2                                   ;"
	"add r0, r0, #1                                   ;"
	"mov r0, r2                                       ;"
	"mov r1, #168                                     ;"
	"mov r2, #0                                       ;"
	"svc #2031616                                     ;"
		"mov r2, r0                                       ;"
		"add r1, r2, r1                                   ;"
		"add r1, r1, r2, lsl #10                          ;"
		"eor r1, r1, r2, lsr #6                           ;"
	
	
	
	*/
	
	asm(
	
	// PRAY
	// fucking O3 kept on trying to optimize the INSIDE of my gods forsaken asm, im talking it would slap an instruction 
	// inside the middle of it, or just flat out reorder the whole ass thing, even with volatile. 
	// so now i get to do this all in asm 
	// tbh tho, i rlly missed asm, so yay 
	
	// ok, now that i think abt it,,, this is 100% my fault lmao, i was fucking with registers that the for loop was using and pissing my pants when the for loop pissed its pants, when i was the one pissing its pants
	// computer science.
	
	// i dont have,,, as specific knoweledge of arm as i do with z80,,, ugh
	// how many registers do i even have to work with?
	//,,, oh my gods i have a ton  
	// are lower index registers faster
	// they all have the same speed.
	// yeaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	// i find it weird that,,, hmm 
	// idek 
	
	// ALSO, selecting skip bios anim, changes this??
	
	
	
	
	// r0 used as input(and output) of bios call 
	"mov r1, #168;" // const
	"mov r2, #0;" // const
	
	// why r7 you might be asking? bc 3,4,5,6, and 15 didnt work?
	"mov r7, #0;" // iterator
	
	"mov r6, #0;" // result
	
	"LOOP:;"
	
	
	// test = i-(((i & 3)+1) | 3);
	
	// can save a mov instr by just, anding r7 right into r0
	"and r0, r7, #3;"
	"add r0, r0, #1;"
	"orr r0, r0, #3;"
	"sub r0, r7, r0;"
	
	
	// these regs arent const, and are modified in the syscall 
	// however, counterpoint, who gives a fuck?
	// it will get me what i need regardless.
	//"mov r1, #168;" // const
	//"mov r2, #0;" // const
	
	// i have like,,, 0 guarentee that this wont fuck all my registers up. (it does, i just dont care)
	"swi 0x1F << 16;"


	// res += temp;
	"add r6, r6, r0;"
	
	// res += (temp << 10);
	"add r6, r6, r0, lsl #10;"
	
	// res ^= (temp >> 5);
	"add r6, r6, r0, lsr #5;"
	

	//"add r7, r7, #1;"
	"add r7, r7, #16;"
	"cmp r7, $0x4000;"
	"bne LOOP;"
	
	"add r6, r6, r6, lsl #3;"
	"eor r6, r6, r6, lsr #11;"
	"add r6, r6, r6, lsl #15;"
	
	"mov r0, r6;"
	
	"bx lr;"	
	);
	
	unsigned res = 0;
	return res;
}

int main() {
	
	bn::core::init(); 
	
	BN_LOG("butano inited");
	
	biosHash = getBiosHash();
	
	u8 tempBiosHash[4];
	
	memcpy(tempBiosHash, &biosHash, 4);
	
	for(int i=0; i<4; i++) {
		
		//BN_LOG((unsigned)tempBiosHash[i], " ");
		
		u8 idrk = 0x80;
		
		bn::string<64> string;
		bn::ostringstream string_stream(string);
		
		string_stream << (unsigned)tempBiosHash[i];
		string_stream << ' ';
		
		for(int j=0; j<8; j++) {
			if(tempBiosHash[i] & idrk) {
				string_stream << '1';
			} else {
				string_stream << '0';
			}
			idrk >>= 1;
		}
			
		BN_LOG(string);
	}
	
	//BN_LOG(biosHash[0], " ", biosHash[1], " ", biosHash[2], " ", biosHash[3], " ", biosHash[4], " ", biosHash[5], " ", biosHash[6], " ", biosHash[7]);
	
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


