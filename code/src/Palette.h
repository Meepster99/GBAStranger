#pragma once

//#include "SharedTypes.h"
#include "bn_color.h"
#include "bn_bg_palette_item.h"
#include "bn_sprite_palette_item.h"

class Palette {
public:

	bn::color colorArray[16];
	
	constexpr Palette(bn::color a, bn::color b, bn::color c, bn::color d, bn::color e = bn::color(0, 0, 0)) :
	colorArray{a, b, c, d, e, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)}
	{}

	const bn::bg_palette_item getBGPalette() {
		
		bn::span<bn::color> spanthing(colorArray);
			
		return bn::bg_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getSpritePalette() {
		
		bn::span<bn::color> spanthing(colorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
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

extern Palette redText;
extern Palette greenText;
extern Palette blueText;
extern Palette whiteText;
