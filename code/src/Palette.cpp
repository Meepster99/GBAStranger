
#include "Palette.h"
 
// why didnt i just go dark to light omfg

//Palette defaultPalette(bn::color(0, 31, 31), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));
//Palette redPalette(bn::color({0, 0, 0}), bn::color(0, 0, 0), bn::color(31, 31, 31),bn::color(24, 24, 24), bn::color(16, 16, 16));

#define MAKECOLOR(n) bn::color( \
    (32 * ((n & 0xFF0000) >> 16)) / 256, \
    (32 * ((n & 0x00FF00) >> 8)) / 256, \
    (32 * ((n & 0x0000FF) >> 0)) / 256) 
	
#define PALETTEGEN(a, b, c, d)  Palette(bn::color(0, 31, 31), MAKECOLOR(a), MAKECOLOR(d), MAKECOLOR(c), MAKECOLOR(b));

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
