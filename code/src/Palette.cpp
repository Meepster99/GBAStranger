

#include "Palette.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define CLAMP(value, min_val, max_val) MAX(MIN((value), (max_val)), (min_val))

constexpr int CONVERT5BIT(int c, int maxVal) {
	(void)maxVal;

	bn::fixed ugh = (32.0 * c) / 256.0;

	int res = ugh.round_integer();

	res = CLAMP(res, 0, 31);

	return res;
};

constexpr bn::color MAKECOLOR(unsigned n) {

	int r = (n & 0xFF0000) >> 16;
	int g = (n & 0x00FF00) >> 8;
	int b = (n & 0x0000FF) >> 0;

	int maxVal = MAX(MAX(r, g), b);

	r = CONVERT5BIT(r, maxVal);
	g = CONVERT5BIT(g, maxVal);
	b = CONVERT5BIT(b, maxVal);

	return bn::color(r, g, b);
}

constexpr Palette PALETTEGEN(int a, int b, int c, int d) {
	return Palette(bn::color(0, 31, 31), MAKECOLOR(a), MAKECOLOR(d), MAKECOLOR(c), MAKECOLOR(b));
}

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

Palette DEPTHS = PALETTEGEN(0x000000, 0x373131, 0xa58e8e, 0xe0d0d0);

//Palette MORNING = PALETTEGEN(0xFFD8E3, 0xFF739A, 0xFFA4C6, 0xffc5da);
//Palette NOON = PALETTEGEN(0x9CCCEA, 0x68A2D8, 0x3D81C8, 0x1F5AA7);
//Palette EVENING = PALETTEGEN(0xd45915, 0xFF985F, 0xFF6F2F, 0xffd0b5);
Palette NIGHT = PALETTEGEN(0x121B38, 0x233759, 0x354F7B, 0x465B9D);

Palette MILK = PALETTEGEN(0x101021, 0x522942, 0xad3131, 0xefdec6);

Palette BOTHERSOME = PALETTEGEN(590336, 14715016, 16373809, 16448493);
Palette POCKET = PALETTEGEN(2633760, 5398858, 8622458, 10859421);
Palette PORTMASTER = PALETTEGEN(664637, 4427468, 15900474, 16777215);

Palette CUSTOM    = PALETTEGEN(0,8421504,12632256,16777215);

Palette* paletteList[17] = {&GRAYPALETTE,&REDPALETTE,&ORANGEPALETTE,&YELLOWPALETTE,&GREENPALETTE,&BLUEPALETTE,&INDIGOPALETTE,&VIOLETPALETTE,&BLUEBERRYJAMPALETTE,&ZERORANGERPALETTE,&DEPTHS,&NIGHT,&MILK,&BOTHERSOME,&POCKET,&PORTMASTER,&CUSTOM};

const char* paletteNameList[17] = {"GRAY\0", "RED\0", "ORANGE\0", "YELLOW\0", "GREEN\0", "BLUE\0", "INDIGO\0", "VIOLET\0", "BLUEBERRYJAM\0", "ZERORANGER\0", "DEPTHS\0", "NIGHT\0", "MILK\0", "BOTHERSOME\0", "POCKET\0", "PORTMASTER\0", "CUSTOM(PRESS B)\0"};


bn::array<bn::color, 16> colorArray;
bn::array<bn::color, 16> alternateColorArray;
bn::array<bn::color, 16> fontColorArray;
bn::array<bn::color, 16> tempColorArray;
bn::array<bn::color, 16> whiteOutlineArray;

bn::array<bn::color, 16> blackColorArray;
bn::array<bn::color, 16> darkGrayColorArray;
bn::array<bn::color, 16> lightGrayColorArray;
bn::array<bn::color, 16> whiteColorArray;
