#pragma once

//#include "SharedTypes.h"
#include "bn_color.h"
#include "bn_bg_palette_item.h"
#include "bn_sprite_palette_item.h"

#include <bn_assert.h>
#include "bn_fixed.h"

#define TRANSPARENT 0
#define BLACK 1 
#define WHITE 2
#define LIGHTGRAY 3
#define DARKGRAY 4
 
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
 
class Palette {
public:

	bn::color colorArray[16];
	bn::color alternateColorArray[16];
	bn::color fontColorArray[16];
	bn::color tempColorArray[16];
	
	constexpr Palette(bn::color a, bn::color b, bn::color c, bn::color d, bn::color e = bn::color(0, 0, 0)) :
	colorArray{a, b, c, d, e, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)},
	alternateColorArray{a, b, e, d, c, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)},
	fontColorArray{a, e, d, d, a, e, d, d, a, e, d, d, b, e, e, e},
	tempColorArray{a, b, c, d, e, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)}
	{}

	const bn::bg_palette_item getBGPalette() {
		
		bn::span<bn::color> spanthing(colorArray);
			
		return bn::bg_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getSpritePalette() {
		
		bn::span<bn::color> spanthing(colorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}

	void modifyTempColors(int index, bool toWhite) {
		tempColorArray[BLACK] = colorArray[BLACK];
		tempColorArray[WHITE] = colorArray[WHITE];
		tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
		tempColorArray[LIGHTGRAY] = colorArray[LIGHTGRAY];
		
		
		if(toWhite) {
			
			if(index >= 0) {
				tempColorArray[DARKGRAY] = colorArray[LIGHTGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[WHITE];
			}
			
			if(index >= 1) {
				tempColorArray[BLACK] = colorArray[DARKGRAY];
			}
			
			if(index >= 2) {
				tempColorArray[DARKGRAY] = colorArray[WHITE];
				tempColorArray[BLACK] = colorArray[LIGHTGRAY];
			}
			
			if(index >= 3) {
				tempColorArray[BLACK] = colorArray[WHITE];
			}

		} else {
			
			// 0 clue 
			if(index == 0) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[BLACK];
				tempColorArray[LIGHTGRAY] = colorArray[BLACK];
				tempColorArray[WHITE] = colorArray[BLACK];
			}
			
			if(index == 1) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[BLACK];
				tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
				tempColorArray[WHITE] = colorArray[DARKGRAY];
			}
			
			if(index == 2) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
				tempColorArray[WHITE] = colorArray[DARKGRAY];
			}
			
			if(index == 3) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[LIGHTGRAY];
				tempColorArray[WHITE] = colorArray[LIGHTGRAY];
			}
			
			if(index == 4) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[LIGHTGRAY];
				tempColorArray[WHITE] = colorArray[WHITE];
			}
		}
	}
	
	const bn::sprite_palette_item getDarkSpritePalette() {
		
		tempColorArray[BLACK] = colorArray[BLACK];
		tempColorArray[DARKGRAY] = colorArray[BLACK];
		tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
		tempColorArray[WHITE] = colorArray[LIGHTGRAY];
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getLightGraySpritePalette() {
		
		tempColorArray[BLACK] = colorArray[LIGHTGRAY];
		tempColorArray[DARKGRAY] = colorArray[LIGHTGRAY];
		tempColorArray[LIGHTGRAY] = colorArray[LIGHTGRAY];
		tempColorArray[WHITE] = colorArray[LIGHTGRAY];
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getWhiteSpritePalette() {
		
		tempColorArray[BLACK] = colorArray[WHITE];
		tempColorArray[DARKGRAY] = colorArray[WHITE];
		tempColorArray[LIGHTGRAY] = colorArray[WHITE];
		tempColorArray[WHITE] = colorArray[WHITE];
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getDarkGraySpritePalette() {
		
		tempColorArray[BLACK] = colorArray[DARKGRAY];
		tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
		tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
		tempColorArray[WHITE] = colorArray[DARKGRAY];
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getBlackSpritePalette() {
		
		tempColorArray[BLACK] = colorArray[BLACK];
		tempColorArray[DARKGRAY] = colorArray[BLACK];
		tempColorArray[LIGHTGRAY] = colorArray[BLACK];
		tempColorArray[WHITE] = colorArray[BLACK];
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::bg_palette_item getBlackBGPalette() {
		
		tempColorArray[BLACK] = colorArray[BLACK];
		tempColorArray[DARKGRAY] = colorArray[BLACK];
		tempColorArray[LIGHTGRAY] = colorArray[BLACK];
		tempColorArray[WHITE] = colorArray[BLACK];
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::bg_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::bg_palette_item getBGPaletteFade(int index = -1, bool toWhite = true) {
		(void)index;

		// ok something is (curse)ed with the color array 
		// copying it didnt work, assignment didnt work, im building from scratch now.
		// ok something is EXTREMELY (curse)ED with the colour class 
		// things arent making sense, blue is coming from nowhere?
		
		//,,, am i going to have to do all palets at compile time
		// or maybe if i make the tempthing an array like,, yea?
		// OK,
		//for some reason, tempcolarray HAD to be declared as a class var?? why??
		// ok honestly, (curse) it. 
		// i want memory level access to the palete table, and idc how i get it.
		// ive had so much trouble with this (curse), having to change all the palets of everything, when i can just access the god damn table.
		// but,, i already have a framework in place,, and i resolved this issue 
		// is there ever a case where 
		// actually no, i would have to manually change any sprite i have during a cutscene, thats annoying 
		
		modifyTempColors(index, toWhite);
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::bg_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getSpritePaletteFade(int index, bool toWhite = true) {
		(void)index;
		
		modifyTempColors(index, toWhite);
		
		bn::span<bn::color> spanthing(tempColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);	;
	}
	
	const bn::sprite_palette_item getAlternateSpritePalette() {
		// sets the palette specifically for when,,, im in the menu section??
		
		
		tempColorArray[0] = colorArray[A];
		tempColorArray[1] = colorArray[B];
		tempColorArray[2] = colorArray[E];
		tempColorArray[3] = colorArray[D];
		tempColorArray[4] = colorArray[C];
		
	
		//bn::span<bn::color> spanthing(alternateColorArray);
		bn::span<bn::color> spanthing(tempColorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getFontSpritePalette() {
		// sets the palette specifically for when,,, im in the menu section??
		// why the (curse) do the fonts take the lastc colors in the array??
		//bn::span<bn::color> spanthing(fontColorArray);
		
		
		//  {a, e, d, d, a, e, d, d, a, e, d, d, b, e, e, e},
		
		tempColorArray[0] = colorArray[A];
		tempColorArray[1] = colorArray[E];
		tempColorArray[2] = colorArray[D];
		tempColorArray[3] = colorArray[D];
		tempColorArray[4] = colorArray[A];
        tempColorArray[5] = colorArray[E];
        tempColorArray[6] = colorArray[D];
        tempColorArray[7] = colorArray[D];
		tempColorArray[8] = colorArray[A];
        tempColorArray[9] = colorArray[E];
        tempColorArray[10] = colorArray[D];
		tempColorArray[11] = colorArray[D];
		tempColorArray[12] = colorArray[B];
		tempColorArray[13] = colorArray[E];
		tempColorArray[14] = colorArray[E];
		tempColorArray[15] = colorArray[E];
		
		bn::span<bn::color> spanthing(tempColorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	bn::bpp_mode getBPP() {
		return bn::bpp_mode::BPP_4;
	}

	const bn::span<const bn::color> getColors() {
		bn::span<bn::color> spanthing(colorArray);
		return bn::bg_palette_item(spanthing, bn::bpp_mode::BPP_4).colors_ref();
	}
	
	
	
};

extern Palette defaultPalette;

extern Palette GRAYPALETTE;   
extern Palette REDPALETTE;
extern Palette ORANGEPALETTE;
extern Palette YELLOWPALETTE; 
extern Palette GREENPALETTE;  
extern Palette BLUEPALETTE;
extern Palette INDIGOPALETTE;
extern Palette VIOLETPALETTE;

extern Palette BLUEBERRYJAMPALETTE;

extern Palette ZERORANGERPALETTE;

extern Palette DEPTHS;

//extern Palette MORNING;
//extern Palette NOON;
//extern Palette EVENING;
extern Palette NIGHT;

extern Palette MILK;

extern Palette CUSTOM;

extern Palette* paletteList[14];
extern const char* paletteNameList[14];


//extern Palette redText;
//extern Palette greenText;
//extern Palette blueText;
//extern Palette whiteText;

