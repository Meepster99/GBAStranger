#pragma once

//#include "SharedTypes.h"
#include "bn_color.h"
#include "bn_bg_palette_item.h"
#include "bn_sprite_palette_item.h"
#include "bn_compression_type.h"

#include <bn_assert.h>
#include "bn_fixed.h"

#include "bn_array.h"

#define TRANSPARENT 0
#define BLACK 1 
#define WHITE 2
#define LIGHTGRAY 3
#define DARKGRAY 4
 
#define ACOL 0
#define BCOL 1
#define CCOL 2
#define DCOL 3
#define ECOL 4

/*

ohnobro.
it seems like ive done a major fucky wucky 
sprite palette item's spans,, hold a refrence to the color. 
meaning that my "tempcolorarray" solution was actually "fucked"
and also that doing palette manip is also WAYYYY easier than i made it be for myself.

and now ive somehow corrupted every single palette. 

i suppose,,, im going to need to have,,, a fucking individual array foreach thing again 
unless i want to deal with memory hell. 
ugh
annnnnnd IM almost definitely going to fuck up all the fancy fades ive made

godsssss this whole class is fucked to all hell

no wait, i know how to get around this. 
i just need to avoid EVER updating the ACTUAL palette cache.
i did this other times.

you wont see this in the commit logs, but like 
gods trying to get restrequest working again like, WHAT THE FUCK IS GOING ON 
i ended up having to reset back to head

this whole implimentation is scuffed. why dont i have global funcs for getting the palettes of things??? omfg 

was it caused by me moving random shit out of rom into ram?
theres no fucking way though, like theres no fucking way 

ok aparently there is a way?????
thats,,, that not ok. thats just not ok. 
how.

i moved game vblank, and the effects manager vblank, enter and exit funcs from arm and iwram, to ewram and thumb. 
and the bug went away??? what the fuck 
i just spent 3 hours on this?!?!?!?!

gods the issue is,,, i have certain things that like, need to be in arm, else i lag ( floor draw code)

is this going to hurt performance?

ok now i,,, 
i give up. i do not get it 

let it be known, if shit just gets weird, its arm/something else being fucked
or maybe i just needed a make clean???? 

no! putting the game classes vblank func into arm and iwram, causes the text when saying no to cif's dream in 254 to corrupt like, fucking everything.
i rlly hope that like, swapping from thumb to arm isnt expensive.
bc im trying to keep other things in arm 
or maybe its bc im not putting the attributes in the cpp and header???

nvm even with vblank in ewram, shits still fucked, just on the second time talking to the tree

gods having an array per thing is just horrid for mem management, and idek if it will actually help

ok i have no clue anymore 

i think its an issue with sections,,, does,,, does putting something in overwrite my palette data?? as in my funcs are overwriteing other data?
putting my shit onto ewram like,, i need it in arm to be fast enough!

is it,, swaping to arm instrs without, actually swapping to arm code mode??? theres no way, it would crash
how much of my code is actually in thumb vs arm?? 
i have -mthumb and mthumb interwork
does putting something into iwram automatically compile it as arm?? what the fuck is happening
would noinline give it the push needed to, not be a bitch and switch to arm? 
what the fuck is happening?
is it because these funcs arent global???

gods i dont want to rewrite evrything with namespaceslkjadakhfdkjalfs

also gods im probs just placeboing myself, but i swear the code is slower now.

am i,, running out of iwram?

i,, think i am?? 
the fact that i didnt get an error though,,, is extremely concerning

*/
 
 
class Palette {
public:

	//bn::color colorArray[16];
	//bn::color alternateColorArray[16];
	//bn::color fontColorArray[16];
	//bn::color tempColorArray[16];
	
	bn::array<bn::color, 16> colorArray;
	bn::array<bn::color, 16> alternateColorArray;
	bn::array<bn::color, 16> fontColorArray;
	bn::array<bn::color, 16> tempColorArray;
	
	bn::array<bn::color, 16> blackColorArray;
	bn::array<bn::color, 16> darkGrayColorArray;
	bn::array<bn::color, 16> lightGrayColorArray;
	bn::array<bn::color, 16> whiteColorArray;
	
	constexpr Palette(bn::color a, bn::color b, bn::color c, bn::color d, bn::color e = bn::color(0, 0, 0)) :
	colorArray{a, b, c, d, e, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)},
	alternateColorArray{a, b, e, d, c, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)},
	fontColorArray{a, e, d, d, a, e, d, d, a, e, d, d, b, e, e, e},
	tempColorArray{a, b, c, d, e, bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)}
	{
		
		blackColorArray = {
			colorArray[TRANSPARENT], colorArray[BLACK], colorArray[BLACK], colorArray[BLACK], colorArray[BLACK],
			bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)
		};
		
		darkGrayColorArray = {
			colorArray[TRANSPARENT], colorArray[DARKGRAY], colorArray[DARKGRAY], colorArray[DARKGRAY], colorArray[DARKGRAY],
			bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)
		};
		
		lightGrayColorArray = {
			colorArray[TRANSPARENT], colorArray[LIGHTGRAY], colorArray[LIGHTGRAY], colorArray[LIGHTGRAY], colorArray[LIGHTGRAY],
			bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)
		};
		
		whiteColorArray = {
			colorArray[TRANSPARENT], colorArray[WHITE], colorArray[WHITE], colorArray[WHITE], colorArray[WHITE],
			bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0), bn::color(0, 0, 0)
		};
		
	}

	const bn::bg_palette_item getBGPalette() {
		
		bn::span<bn::color> spanthing(colorArray);
			
		return bn::bg_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getSpritePalette() {
		
		bn::span<bn::color> spanthing(colorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4, bn::compression_type::NONE);
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
		//bn::span<bn::color> spanthing(darkGrayColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getLightGraySpritePalette() {
		
		/*
		tempColorArray[BLACK] = colorArray[LIGHTGRAY];
		tempColorArray[DARKGRAY] = colorArray[LIGHTGRAY];
		tempColorArray[LIGHTGRAY] = colorArray[LIGHTGRAY];
		tempColorArray[WHITE] = colorArray[LIGHTGRAY];
		*/
		
		//bn::span<bn::color> spanthing(tempColorArray);
		bn::span<bn::color> spanthing(lightGrayColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getWhiteSpritePalette() {
		
		/*
		tempColorArray[BLACK] = colorArray[WHITE];
		tempColorArray[DARKGRAY] = colorArray[WHITE];
		tempColorArray[LIGHTGRAY] = colorArray[WHITE];
		tempColorArray[WHITE] = colorArray[WHITE];
		*/
		
		//bn::span<bn::color> spanthing(tempColorArray);
		bn::span<bn::color> spanthing(whiteColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getDarkGraySpritePalette() {
		
		/*
		tempColorArray[BLACK] = colorArray[DARKGRAY];
		tempColorArray[DARKGRAY] = colorArray[DARKGRAY];
		tempColorArray[LIGHTGRAY] = colorArray[DARKGRAY];
		tempColorArray[WHITE] = colorArray[DARKGRAY];
		*/
		
		bn::span<bn::color> spanthing(darkGrayColorArray);

		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getBlackSpritePalette() {
		
		/*
		tempColorArray[BLACK] = colorArray[BLACK];
		tempColorArray[DARKGRAY] = colorArray[BLACK];
		tempColorArray[LIGHTGRAY] = colorArray[BLACK];
		tempColorArray[WHITE] = colorArray[BLACK];
		*/
		
		//bn::span<bn::color> spanthing(tempColorArray);
		bn::span<bn::color> spanthing(blackColorArray);
	
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
		
		/*
		tempColorArray[0] = colorArray[ACOL];
		tempColorArray[1] = colorArray[BCOL];
		tempColorArray[2] = colorArray[ECOL];
		tempColorArray[3] = colorArray[DCOL];
		tempColorArray[4] = colorArray[CCOL];
		*/
	
		bn::span<bn::color> spanthing(alternateColorArray);
		//bn::span<bn::color> spanthing(tempColorArray);
			
		return bn::sprite_palette_item(spanthing, bn::bpp_mode::BPP_4);
	}
	
	const bn::sprite_palette_item getFontSpritePalette() {
		// sets the palette specifically for when,,, im in the menu section??
		// why the (curse) do the fonts take the lastc colors in the array??
		//bn::span<bn::color> spanthing(fontColorArray);
		
		
		//  {a, e, d, d, a, e, d, d, a, e, d, d, b, e, e, e},
		
		/*
		tempColorArray[0]  = colorArray[ACOL];
		tempColorArray[1]  = colorArray[ECOL];
		tempColorArray[2]  = colorArray[DCOL];
		tempColorArray[3]  = colorArray[DCOL];
		tempColorArray[4]  = colorArray[ACOL];
        tempColorArray[5]  = colorArray[ECOL];
        tempColorArray[6]  = colorArray[DCOL];
        tempColorArray[7]  = colorArray[DCOL];
		tempColorArray[8]  = colorArray[ACOL];
        tempColorArray[9]  = colorArray[ECOL];
        tempColorArray[10] = colorArray[DCOL];
		tempColorArray[11] = colorArray[DCOL];
		tempColorArray[12] = colorArray[BCOL];
		tempColorArray[13] = colorArray[ECOL];
		tempColorArray[14] = colorArray[ECOL];
		tempColorArray[15] = colorArray[ECOL];
		*/
		
		//bn::span<bn::color> spanthing(tempColorArray);
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

