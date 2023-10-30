#pragma once

//#include "SharedTypes.h"
#include "bn_color.h"
#include "bn_bg_palette_item.h"
#include "bn_sprite_palette_item.h"

#include <bn_assert.h>

#define TRANSPARENT 0
#define BLACK 1 
#define WHITE 2
#define LIGHTGRAY 3
#define DARKGRAY 4

class Palette {
public:

	bn::color colorArray[16];
	bn::color alternateColorArray[16];
	bn::color fontColorArray[16];
	bn::color tempColorArray[16];
	
	//static int fadePalette = 0;
	
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
				tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
				tempColorArray[WHITE] = colorArray[DARKGRAY];
			}
			
			if(index == 2) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
				tempColorArray[WHITE] = colorArray[LIGHTGRAY];
			}
			
			if(index == 3) {
				tempColorArray[BLACK] = colorArray[BLACK];
				tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
				tempColorArray[LIGHTGRAY] = colorArray[LIGHTGRAY];
				tempColorArray[WHITE] = colorArray[WHITE];
			}
		}
	}
	
	const bn::bg_palette_item getBGPaletteFade(int index, bool toWhite = true) {
		(void)index;

		// ok something is fucked with the color array 
		// copying it didnt work, assignment didnt work, im building from scratch now.
		// ok something is EXTREMELY FUCKED with the colour class 
		// things arent making sense, blue is coming from nowhere?
		
		//,,, am i going to have to do all palets at compile time
		// or maybe if i make the tempthing an array like,, yea?
		// OK,
		//for some reason, tempcolarray HAD to be declared as a class var?? why??
		// ok honestly, fuck it. 
		// i want memory level access to the palete table, and idc how i get it.
		// ive had so much trouble with this shit, having to change all the palets of everything, when i can just access the god damn table.
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
		
		bn::span<bn::color> spanthing(alternateColorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getFontSpritePalette() {
		// sets the palette specifically for when,,, im in the menu section??
		// why the fuck do the fonts take the lastc colors in the array??
		bn::span<bn::color> spanthing(fontColorArray);
			
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

extern Palette* paletteList[10];
extern const char* paletteNameList[10];


extern Palette redText;
extern Palette greenText;
extern Palette blueText;
extern Palette whiteText;
