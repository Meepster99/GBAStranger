
#include "Palette.h"
 
// why didnt i just go dark to light omfg

//Palette defaultPalette(bn::color(0, 31, 31), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));
//Palette redPalette(bn::color({0, 0, 0}), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define CLAMP(value, min_val, max_val) MAX(MIN((value), (max_val)), (min_val))

// using fixed here was causing EXCESSIVE (curse)ed (curse) 
// but, since this all occurs in the preprocessor, i think we should be able to use doulbles
// nope :) and i cant even used fixed
// ok, i have 0 clue why i cannot use decimals here, but a solution is to like
// add 4 to the input value.
// why? idk
// well, i do k, but why i cannot use decimals is unknown to the gods
// wait adding 4 didnt work?? you have got to be (curse)ing kidding me 

/*

0 clue whats going on here
having TEST  be the thing to add 4, and call bruh works,
adding 4 in bruh doesnt.
wtf 

#define BRUH(n) CLAMP( (32 * n) / 256, 0, 0xFF)


//#define TEST(n) BRUH(n+4)
#define TEST(n) CLAMP( (32 * (n+4)) / 256, 0, 0xFF)

#define TEST2(n) bn::color( \
    TEST(((n & 0xFF0000) >> 16)), \
    TEST(((n & 0x00FF00) >> 8)), \
    TEST(((n & 0x0000FF) >> 0)) )

#define TEST3(a, b, c, d)  Palette(bn::color(0, 31, 31), TEST2(a), TEST2(d), TEST2(c), TEST2(b));

const Palette val    = TEST3(1310720,12717325,16284503,16777215);	


ok.
n+4 and n + 4 are different. in defines. for some (curse)ing reason .
omfg 
how many hours 
gods 

nope, that wasnt it????? 
bc for some reason when i copy the define into here it doesnt work?

tbh i could just do this in functions.
but i want it to be constexpr?

is it variable scoping with layered defines??
bc even with functions with noinline, its still (curse)ting itself

the defines i had leftover in main BETTER have not been causing this 

nope, they werent 

i found it :)) and i want to die 

bn::color calls bn_assert. doing this before bn::init is called, causes it to (curse) itself. 
i was clamping at a max of 0xFF instead of 31. this caused it to try to call assert, and crash.

im going to go do a pull request
like i should of for the unordered map bug

ugh but the more that i think abt it like,,, 
im not sure 
i could have it 

naw honestly, as much as i would like to blame someone other than myself
i wouldnt of made this mistake if i had eaten food today 

im now realizing, that after all this pain to change a rgb value from 
(16, 0, 0) to (24, 0, 0), is that 
16 looks better.

*/

//#define CONVERT5BIT(n) CLAMP( (((bn::fixed)32 * n) / 256).round_integer(), 0, 0xFF)
//#define CONVERT5BIT(n) CLAMP( ((32.0 * n) / 256.0), 0, 0xFF)
//#define CONVERT5BIT(n) CLAMP( (int)(0.5 + ((32.0 * n) / 256.0)), 0, 0xFF)
//#define CONVERT5BIT(n) CLAMP( (32 * (n+4)) / 256, 0, 0xFF)

// works:
//#define CONVERT5BIT(n) CLAMP( ( 32 * ( n  ) ) / 256 , 0 , 0xFF )

// doesnt work:
//#define CONVERT5BIT(n) CLAMP( ( 32 * ( n + 4) ) / 256 , 0 , 0xFF )

constexpr int CONVERT5BIT(int PLEASEHELP, int maxVal) {
	//return CLAMP( ( 32 * ( n + 4) ) / 256 , 0 , 0xFF );
	
	
	//int whatthe(curse) = ( 32 * ( n + 0) ) / 256;
	//int whatthe(curse) = ( 32 * ( n + 4) ) / 256;
	
	/*
	int dude = n;
	dude++;
	dude++;
	dude++;
	dude++;
	
	int whatthe(curse) = ( 32 * ( dude + 0) );
	whatthe(curse) = whatthe(curse) >> 8;
	*/
	//int whatthe(curse) = PLEASEHELP;
	//whatthe(curse) = ( 32 * ( whatthe(curse) + 0) );
	//whatthe(curse) = whatthe(curse) >> 8;
	
	//return PLEASEHELP >> 3;
	/*
	int result;
	asm volatile (
        "mov r1, #128\n\t"
        "add %0, r1, %1, lsl #5\n\t"
        "asr %0, %0, #8"
        : "=r" (result)
        : "r" (PLEASEHELP)
        : "r1"
    );
	return result;
	*/
	

	//int whatthe(curse) = 128;
	//whatthe(curse) = whatthe(curse) + (PLEASEHELP << 5);
	//whatthe(curse) = whatthe(curse) >> 8;

	// now, after spending 5 god damn hours on this (curse) 
	// ima do something fun and convert the rgb to the CIELAB color space, and then back to 15 bitset
	// but honestly, gods im tired
	// its been so long since ive talked to any of my friends
	
	
	bn::fixed ugh = (32.0 * PLEASEHELP) / 256.0;
	
	int whatthe = ugh.round_integer();
	
	if(maxVal > 172) {
		//whatthe(curse) = ugh.ceil_integer();
	//else if(maxVal < 86) {
	} else if(maxVal < 32) {
		//whatthe(curse) = ugh.floor_integer();
	}
	
	//int whatthe(curse) = PLEASEHELP >> 3;
	
	if(whatthe < 0) {
		whatthe = 0;
	}
	if(whatthe >= 32) {
		whatthe = 31;
	}
	
	return whatthe;
	
	
};

//static_assert(CONVERT5BIT(20) == 3, "jesus (curse). ");

/*
#define MAKECOLOR(n) bn::color( \
    CONVERT5BIT(((n & 0xFF0000) >> 16)), \
    CONVERT5BIT(((n & 0x00FF00) >> 8)), \
    CONVERT5BIT(((n & 0x0000FF) >> 0)) )
*/

constexpr bn::color MAKECOLOR(unsigned n) {
	//return bn::color(CONVERT5BIT(((n & 0xFF0000) >> 16)), CONVERT5BIT(((n & 0x00FF00) >> 8)), CONVERT5BIT(((n & 0x0000FF) >> 0)));
	
	int r = (n & 0xFF0000) >> 16;
	int g = (n & 0x00FF00) >> 8;
	int b = (n & 0x0000FF) >> 0;
	
	int maxVal = MAX(MAX(r, g), b);

	r = CONVERT5BIT(r, maxVal);
	g = CONVERT5BIT(g, maxVal);
	b = CONVERT5BIT(b, maxVal);
	
	return bn::color(r, g, b);
}
	
//#define PALETTEGEN(a, b, c, d)  Palette(bn::color(0, 31, 31), MAKECOLOR(a), MAKECOLOR(d), MAKECOLOR(c), MAKECOLOR(b));
constexpr Palette PALETTEGEN(int a, int b, int c, int d) {
	return Palette(bn::color(0, 31, 31), MAKECOLOR(a), MAKECOLOR(d), MAKECOLOR(c), MAKECOLOR(b));
}


	
// I SHOULD JUST HAVE A PALETTEMANAGER CLASS OMFG, OR INTEGRATE IT INTO GAME 
//int Palette::fadePalette = -1;
//bool Palette::toWhite = true;

Palette defaultPalette(bn::color(0, 31, 31), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));

//Palette defaultPalette = PALETTEGEN(0, 8421504, 12632256, 16777215);
Palette GRAYPALETTE    = PALETTEGEN(0,8421504,12632256,16777215);
Palette REDPALETTE     = PALETTEGEN(1310720,12717325,16284503,16777215);
Palette ORANGEPALETTE  = PALETTEGEN(1183493,12606012,16359488,16777215);
Palette YELLOWPALETTE  = PALETTEGEN(986880,12220314,15385344,16777215);
Palette GREENPALETTE   = PALETTEGEN(5140,815204,5823883,16777215);
Palette BLUEPALETTE    = PALETTEGEN(1387312,488851,6605050,16777215);
Palette INDIGOPALETTE  = PALETTEGEN(852000,7685745,10983884,16777215);
Palette VIOLETPALETTE  = PALETTEGEN(327710,6631840,12288456,16777215);

// credit to gooeyphantasm on disc for this one, titled blueberryjam
Palette BLUEBERRYJAMPALETTE = PALETTEGEN(0x24276c,0xb33598,0xff8f8e,0xfafdfc);

Palette ZERORANGERPALETTE = PALETTEGEN(1579812,806758,14325314,15851985);


Palette* paletteList[10] = {&GRAYPALETTE,&REDPALETTE,&ORANGEPALETTE,&YELLOWPALETTE,&GREENPALETTE,&BLUEPALETTE,&INDIGOPALETTE,&VIOLETPALETTE,&BLUEBERRYJAMPALETTE,&ZERORANGERPALETTE};

const char* paletteNameList[10] = {"GRAY\0", "RED\0", "ORANGE\0", "YELLOW\0", "GREEN\0", "BLUE\0", "INDIGO\0", "VIOLET\0", "BLUEBERRYJAM\0", "ZERORANGER\0"};
