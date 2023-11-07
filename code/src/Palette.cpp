
#include "Palette.h"
 
// why didnt i just go dark to light omfg

//Palette defaultPalette(bn::color(0, 31, 31), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));
//Palette redPalette(bn::color({0, 0, 0}), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define CLAMP(value, min_val, max_val) MAX(MIN((value), (max_val)), (min_val))

// using fixed here was causing EXCESSIVE fucked shit 
// but, since this all occurs in the preprocessor, i think we should be able to use doulbles
// nope :) and i cant even used fixed
// ok, i have 0 clue why i cannot use decimals here, but a solution is to like
// add 4 to the input value.
// why? idk
// well, i do k, but why i cannot use decimals is unknown to the gods
// wait adding 4 didnt work?? you have got to be fucking kidding me 

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
n+4 and n + 4 are different. in defines. for some fucking reason .
omfg 
how many hours 
gods 

nope, that wasnt it????? 
bc for some reason when i copy the define into here it doesnt work?

tbh i could just do this in functions.
but i want it to be constexpr?


*/

//#define CONVERT5BIT(n) CLAMP( (((bn::fixed)32 * n) / 256).round_integer(), 0, 0xFF)
//#define CONVERT5BIT(n) CLAMP( ((32.0 * n) / 256.0), 0, 0xFF)
//#define CONVERT5BIT(n) CLAMP( (int)(0.5 + ((32.0 * n) / 256.0)), 0, 0xFF)
//#define CONVERT5BIT(n) CLAMP( (32 * (n+4)) / 256, 0, 0xFF)

// works:
//#define CONVERT5BIT(n) CLAMP( ( 32 * ( n  ) ) / 256 , 0 , 0xFF )

// doesnt work:
//#define CONVERT5BIT(n) CLAMP( ( 32 * ( n + 4) ) / 256 , 0 , 0xFF )

int CONVERT5BIT(int n) {
	return CLAMP( ( 32 * ( n + 4) ) / 256 , 0 , 0xFF );
};

#define MAKECOLOR(n) bn::color( \
    CONVERT5BIT(((n & 0xFF0000) >> 16)), \
    CONVERT5BIT(((n & 0x00FF00) >> 8)), \
    CONVERT5BIT(((n & 0x0000FF) >> 0)) )
	
#define PALETTEGEN(a, b, c, d)  Palette(bn::color(0, 31, 31), MAKECOLOR(a), MAKECOLOR(d), MAKECOLOR(c), MAKECOLOR(b));

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

/*
from the disc, but like,, idk they dont seem right?
colarray[0] = 2369387
colarray[1] = 11744917
colarray[2] = 16748431
colarray[3] = 16777215
colarray[4] = 16448767
*/

Palette ZERORANGERPALETTE = PALETTEGEN(1579812,806758,14325314,15851985);

Palette* paletteList[10] = {&GRAYPALETTE,&REDPALETTE,&ORANGEPALETTE,&YELLOWPALETTE,&GREENPALETTE,&BLUEPALETTE,&INDIGOPALETTE,&VIOLETPALETTE,&BLUEBERRYJAMPALETTE,&ZERORANGERPALETTE};

const char* paletteNameList[10] = {"GRAY\0", "RED\0", "ORANGE\0", "YELLOW\0", "GREEN\0", "BLUE\0", "INDIGO\0", "VIOLET\0", "BLUEBERRYJAM\0", "ZERORANGER\0"};

Palette redText(bn::color(0, 0, 0), bn::color(31, 0, 0), bn::color(0, 0, 0),bn::color(31, 0, 0), bn::color(31, 0, 0));
Palette greenText(bn::color(0, 0, 0), bn::color(0, 31, 0), bn::color(0, 0, 0),bn::color(31, 0, 0), bn::color(31, 0, 0));
Palette blueText(bn::color(0, 0, 0), bn::color(0, 0, 31), bn::color(0, 0, 0),bn::color(31, 0, 0), bn::color(31, 0, 0));
Palette whiteText(bn::color(0, 0, 0), bn::color(31, 31, 31), bn::color(0, 0, 0),bn::color(31, 0, 0), bn::color(31, 0, 0));
