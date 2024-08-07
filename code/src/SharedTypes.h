#pragma once

#include "dataWinIncludes.h"

#include "Profiler.h"

#include "bn_unordered_map.h"
#include "bn_regular_bg_tiles_ptr.h"
#include "bn_bg_tiles.h"
#include "bn_bg_maps.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_tiles.h"
#include "bn_sprite_palettes.h"
#include "bn_version.h"
#include "bn_music_items.h"
#include "bn_music.h"
#include "bn_sound.h"
#include "bn_sound_items.h"
#include "bn_sound_actions.h"
#include "common_variable_8x8_sprite_font.h"
#include "bn_blending.h"
#include "bn_window.h"
#include "bn_rect_window.h"
#include "bn_bg_palette_ptr.h"
#include "bn_istring_base.h"
#include "bn_random.h"
#include "bn_sram.h"
#include "bn_sprites_mosaic.h"
#include "bn_link.h"
#include "bn_link_state.h"
#include "bn_link_player.h"
#include "bn_bg_blocks_manager.h"
#include "bn_green_swap.h"

#ifdef ENABLELOGGING

#include "bn_log.h"

/*
#undef BN_LOG

#define BN_LOG_MOD(...) \
        do \
        { \
            char _bn_string[BN_CFG_LOG_MAX_SIZE]; \
            bn::istring_base _bn_istring(_bn_string); \
            bn::ostringstream _bn_string_stream(_bn_istring); \
            _bn_string_stream.append_args(__VA_ARGS__); \
            bn::log(_bn_istring); \
        } while(false)

#define BN_LOG(...) do { BN_LOG_MOD(__FILE__, " ", __LINE__);  BN_LOG_MOD(__VA_ARGS__); } while(false)
*/

#else
#define BN_LOG(...) do {} while (false)
#endif


#include "Palette.h"
#include <bn_deque.h>
#include "fontData.h"
#include "bn_time.h"
#include "bn_date.h"
#include "bn_timers.h"

// memory addresses
// https://gbadev.net/gbadoc/registers.html#REG_DISPCNT
#define REG_DISPCNT 0x04000000
#define REG_VCOUNT	0x04000006

#define REG_BG0CNT	0x04000008
#define REG_BG1CNT	0x0400000A
#define REG_BG2CNT	0x0400000C
#define REG_BG3CNT	0x0400000E

#define REG_BG0HOFS	0x04000010
#define REG_BG0VOFS	0x04000012
#define REG_BG1HOFS	0x04000014
#define REG_BG1VOFS	0x04000016
#define REG_BG2HOFS	0x04000018
#define REG_BG2VOFS	0x0400001A
#define REG_BG3HOFS	0x0400001C
#define REG_BG3VOFS	0x0400001E

#define REG_GRNSWP	0x04000002

#define REG_IE		0x04000200
#define REG_IF		0x04000202
#define REG_IME		0x04000208

// -----

#ifdef DISABLEASSERTS

#warning asserts are disabled! you might have a bad time

#undef BN_ASSERT
#define BN_ASSERT(...) do {} while (false)

#endif

// https://stackoverflow.com/questions/33050620/golang-style-defer-in-c
#define DEFER(captures, code) std::shared_ptr<void> _(nullptr, [captures](...) mutable { code });

#define USEEWRAM __attribute__((section(".ewram")))
#define USEIWRAM __attribute__((section(".iwram")))
#define USEARM __attribute__((noinline, target("arm"), section(".iwram"), long_call)) // unsure if noinline and long_call are the way to go

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

//#define ABS(v) ( v < 0 ? -v : v )
// gods this is dumb, and one of the most ANNOYING bugs ive had to find recently
#define ABS(v) ( (v) < 0 ? -(v) : (v) )

#define CLAMP(value, min_val, max_val) MAX(MIN((value), (max_val)), (min_val))

typedef unsigned char u8;

// unsure of accuracy, but will make things slightly easier, hopefully
// https://github.com/GValiente/butano/blob/master/butano/hw/include/bn_hw_timer_constants.h
// bn::timers::ticks_per_frame()
#define FRAMETICKS 4389

#define VBLANKTICKS 1309

#define MAXSPRITES 128

#define MAXTEXTSPRITES 128
#define MAXEFFECTSPRITES 112

#define MAXENTITYSPRITES 64

extern unsigned int frame;
extern int playerIdleFrame;
extern bool isVblank;
extern unsigned boobaCount;
extern unsigned playerMoveCount;

extern bool debugToggle;

extern bn::random randomGenerator;

extern void delay(int delayFrameCount);

extern unsigned short* col0;
extern unsigned short* col1;
extern unsigned short* col2;
extern unsigned short* col3;
extern unsigned short* col4;

extern unsigned* stareTiles;
extern unsigned short* stareMap;
extern int* stareTilesCount;
extern int* stareMapCount;

extern unsigned* glitchTiles;
extern int* glitchTilesCount;

#define MONTH (\
  __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
: __DATE__ [2] == 'b' ? 2 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
: __DATE__ [2] == 'y' ? 5 \
: __DATE__ [2] == 'l' ? 7 \
: __DATE__ [2] == 'g' ? 8 \
: __DATE__ [2] == 'p' ? 9 \
: __DATE__ [2] == 't' ? 10 \
: __DATE__ [2] == 'v' ? 11 \
: 12)

// these funcs are long call here and not in main.cpp. is that ok?
__attribute__((section(".iwram"), target("thumb"), long_call)) unsigned short bruhRand();

__attribute__((noinline, optimize("O0"), target("arm"), section(".iwram"), long_call)) unsigned getMiscData();

__attribute__((noinline, target("arm"), section(".iwram"), long_call)) void uncompressData(u8 res[126], u8* input);

void _fullReset();

#define getInput(key) ((globalGame->saveData.delay == -1) ? bn::keypad::pressed(key) : (bn::keypad::pressed(key) || bn::keypad::held(key)))

void logRamStatus();

// this should probs be in ram?
void doNothing();

enum class GameState {
	Normal, // normal gameplay
	Exiting, // we either just completed a level, or just died
	Entering, // we just exited the exiting state and are now reloading stuffs.
	Loading, // loading in new data, do nothing for now.
	Paused, // either actually paused(which i havent even thought about making) or in dialogue.
	Dialogue,
	Cutscene,
	Sleep,
};

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const GameState& e) {
static const char *GameStateToString[] ={
	"Normal",
	"Exiting",
	"Entering",
	"Loading",
};

	stream << GameStateToString[static_cast<int>(e)];
	return stream;
}

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

// all these ostreams should of been added days ago omfg.
// nothing i googled mentioned i could overload ostream for enums
inline bn::ostringstream& operator<<(bn::ostringstream& stream, const Direction& e) {
static const char *DirectionToString[] ={
  	"Up",
	"Down",
	"Left",
	"Right",
};

	stream << DirectionToString[static_cast<int>(e)];
	return stream;
}

enum class EntityType {
	Entity,

	Player,

	Leech,
	Maggot,
	Eye,
	Bull,
	Chester,

	Mimic,
	WhiteMimic,
	GrayMimic,
	BlackMimic,

	Diamond,
	Shadow,

	Boulder,
	Chest,

	AddStatue,
	EusStatue,
	BeeStatue,
	MonStatue,
	TanStatue,
	GorStatue,
	LevStatue,
	CifStatue,
	JukeBox,

	Interactable,

	// i am not sure if this is the best way to do this, but I am going with it
	EmptyChest
};

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const EntityType& e) {
static const char *EntityTypeToString[] ={
  	"Entity",
	"Player",
	"Leech",
	"Maggot",
	"Eye",
	"Bull",
	"Chester",
	"Mimic",
	"WhiteMimic",
	"GrayMimic",
	"BlackMimic",
	"Diamond",
	"Shadow",
	"Boulder",
	"Chest",
	"AddStatue",
	"EusStatue",
	"BeeStatue",
	"MonStatue",
	"TanStatue",
	"GorStatue",
	"LevStatue",
	"CifStatue",
	"Interactable",
	"EmptyChest"
};

	stream << EntityTypeToString[static_cast<int>(e)];
	return stream;

}

enum class TileType {
	Pit,
	Floor,
	Glass,
	Bomb,
	Death,
	Copy,
	Exit,
	Switch,
	WordTile,
	RodTile,
	LocustTile,
	SpriteTile,
	HalfBomb,
};

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const TileType& e) {
static const char *TileTypeToString[] ={
	"Pit",
	"Floor",
	"Glass",
	"Bomb",
	"Death",
	"Copy",
	"Exit",
	"Switch",
	"WordTile",
	"RodTile",
	"LocustTile",
	"SpriteTile",
};

	stream << TileTypeToString[static_cast<int>(e)];
	return stream;

}

#define BACKGROUNDMAPATTRIBUTES __attribute__((section(".iwram")))

class BackgroundMap {
public:

	bn::regular_bg_map_cell cells[32 * 32];
	bn::regular_bg_map_item mapItem;
	bn::regular_bg_ptr bgPointer;
	bn::regular_bg_map_ptr bgMap;

	static Palette* backgroundPalette;

	BackgroundMap(bn::regular_bg_tiles_item& tileset, int zIndex) :
		mapItem(cells[0], bn::size(32, 32)),
		bgPointer(
			bn::regular_bg_item(tileset, backgroundPalette->getBGPalette(), mapItem).create_bg(8, 48)
		),
		bgMap(bgPointer.map())
		{
			init(zIndex);
		}

	// alternate constructor for when (trying) to use an allocated tileset instead of a normal one(for vram modifications)
	BackgroundMap(bn::regular_bg_tiles_ptr& tilesPointer, int zIndex) :
		mapItem(cells[0], bn::size(32, 32)),

		bgPointer(bn::regular_bg_item(
			tilesPointer.vram().value(),
			backgroundPalette->getColors(),
			backgroundPalette->getBPP(),
			*cells,
			bn::size(32, 32)
		).create_bg(8, 48)),

		bgMap(bgPointer.map())
	{
		init(zIndex);
	}

	BackgroundMap(const bn::regular_bg_item& bgItem, int zIndex = 0) :
	mapItem(cells[0], bn::size(32, 32)),
	bgPointer(bgItem.create_bg(8, 48)),
	bgMap(bgPointer.map())
	{
		bgPointer.set_priority(zIndex);
	}

	void create(const bn::regular_bg_item& bgItem, int zIndex = 0) {
		bgPointer.set_priority(zIndex);

		bgPointer.set_tiles(bgItem.tiles_item());
		bgPointer.set_map(bgItem.map_item());

		bgPointer.set_x(8 + 8);
		bgPointer.set_y(8 + 48);

		reloadCells();
	}

	void init(int zIndex) {
		bgPointer.set_priority(zIndex);

		setTile(1,21,4);
		setTile(1,22,4);
		setTile(1,23,4);

		setTile(4,21,4);
		setTile(4,22,4);
		setTile(4,23,4);

		setTile(2,24,4);
		setTile(3,24,4);

		// -----

		setTile(7,21,4);
		setTile(7,22,4);
		setTile(7,23,4);

		setTile(9,21,4);
		setTile(9,22,4);
		setTile(9,23,4);

		setTile(11,21,4);
		setTile(11,22,4);
		setTile(11,23,4);

		setTile(8,24,4);
		setTile(10,24,4);

		// -----

		setTile(14,21,4);
		setTile(14,22,4);
		setTile(14,23,4);

		setTile(17,21,4);
		setTile(17,22,4);
		setTile(17,23,4);

		setTile(15,24,4);
		setTile(16,24,4);
	}

	BACKGROUNDMAPATTRIBUTES void setTile(int x, int y, int tileIndex) {
		bn::regular_bg_map_cell& current_cell = cells[ (y * 32) + x ];

		bn::regular_bg_map_cell_info current_cell_info(current_cell);

		current_cell_info.set_tile_index(tileIndex);
		current_cell = current_cell_info.cell();
	}

	BACKGROUNDMAPATTRIBUTES void setTile(int x, int y, int tileIndex, bool flipX, bool flipY) {
		//bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
		// removing butano's overhead saved a regretably large amount of cpu.
		bn::regular_bg_map_cell& current_cell = cells[ (y * 32) + x ];

		bn::regular_bg_map_cell_info current_cell_info(current_cell);

		current_cell_info.set_tile_index(tileIndex);
		current_cell_info.set_horizontal_flip(flipX);
		current_cell_info.set_vertical_flip(flipY);

		current_cell = current_cell_info.cell();
	}

	void reloadCells() {
		bgMap.reload_cells_ref();
	}

};

class Layer {
public:

	BackgroundMap rawMap;
	Layer(bn::regular_bg_tiles_item tileset, int zIndex, int fillIndex = 0) :
		rawMap(tileset, zIndex)
		{
			init(fillIndex);
		}

	Layer(bn::regular_bg_tiles_ptr tilesPointer, int zIndex, int fillIndex = 0) :
		rawMap(tilesPointer, zIndex)
		{
			init(fillIndex);
		}

	Layer(bn::regular_bg_item bgItem, int zIndex = 0) :
		rawMap(bgItem, zIndex)
		{
			rawMap.reloadCells();
		}

	void init(int fillIndex) {
		//setup black border., just black the whole screen
		for(int i=0; i<30; i++) {
			for(int j=0; j<20; j++) {
				rawMap.setTile(i, j, fillIndex);
			}
		}
		rawMap.reloadCells();
	}

	virtual ~Layer() = default;

	virtual void draw(u8 (&gameMap)[14][9]) {
		BN_ERROR("DONT CALL THE ACTUAL DRAW METHOD IN COLLISION");
		(void)gameMap;
	}

	BACKGROUNDMAPATTRIBUTES void setTile(int x, int y, int tileIndex) {
		rawMap.setTile(x, y, tileIndex);
	}

	BACKGROUNDMAPATTRIBUTES void setTile(int x, int y, int tileIndex, bool flipX, bool flipY) {
		rawMap.setTile(x, y, tileIndex, flipX, flipY);
	}

	u8 tempTileIndicies[4];

	BACKGROUNDMAPATTRIBUTES void setBigTile(int x, int y, int tile, bool flipX = false, bool flipY = false) {
		// this func actually being able to flip (curse) properly is UNCONFIRMED bc I AM SLEEPY
		// coming back to this code, a solid 3 months later, what the fuck?
		// coming back to this code another ~6? months later, what the fuck?
		tempTileIndicies[0] = 4 * tile + ((flipY << 1) | flipX);
		tempTileIndicies[1] = 4 * tile + ((flipY << 1) | !flipX);
		tempTileIndicies[2] = 4 * tile + ((!flipY << 1) | flipX);
		tempTileIndicies[3] = 4 * tile + ((!flipY << 1) | !flipX);

		rawMap.setTile(x * 2 + 1, y * 2 + 1, tempTileIndicies[0], flipX, flipY);
		rawMap.setTile(x * 2 + 2, y * 2 + 1, tempTileIndicies[1], flipX, flipY);
		rawMap.setTile(x * 2 + 1, y * 2 + 2, tempTileIndicies[2], flipX, flipY);
		rawMap.setTile(x * 2 + 2, y * 2 + 2, tempTileIndicies[3], flipX, flipY);
	}

	// goofy
	void update() {
		rawMap.reloadCells();
	}

	void reloadCells() {
		rawMap.reloadCells();
	}

};

#define POSATTRIBUTES
//#define POSATTRIBUTES __attribute__((target("arm"), section(".iwram")))

typedef int_fast8_t POSTYPE;
//typedef int16_t POSTYPE;

class Pos {
public:

	POSTYPE x;
	POSTYPE y;

	constexpr Pos(POSTYPE x_, POSTYPE y_) : x(x_), y(y_) {
		// this assert almost definitely causes a considerable amount of lag.
		BN_ASSERT(x >= 0 && y >= 0 && x < 14 && y < 9, "invalid pos created at ", x, " ", y);
	}

	constexpr Pos(const Pos& other) : x(other.x), y(other.y) {}

	constexpr Pos& operator=(const Pos& other) {
        if (this != &other) {
            x = other.x;
			y = other.y;
        }
        return *this;
    }

	POSATTRIBUTES constexpr bool operator<(const Pos& other) const {
		//return (x + 14 * y) < (other.x + 14 * y);
		// x and y both,, are restricted to 4 bits max per num,,,, i canoptimize this
		// this is only rlly being used for SaneSet anyway
		return ((x << 4) | y) < ((other.x << 4) | other.y);
	}

	POSATTRIBUTES Pos operator+(const Pos& other) {

		POSTYPE tempX = x + other.x;
		POSTYPE tempY = y + other.y;

		if(tempX < 0) { tempX = 0; }
		if(tempY < 0) { tempY = 0; }
		if(tempX >= 14) { tempX = 13; }
		if(tempY >= 9) { tempY = 8; }

		return Pos(tempX, tempY);
	}

	POSATTRIBUTES Pos operator-(const Pos& other) {

		POSTYPE tempX = x - other.x;
		POSTYPE tempY = y - other.y;

		if(tempX < 0) { tempX = 0; }
		if(tempY < 0) { tempY = 0; }
		if(tempX >= 14) { tempX = 13; }
		if(tempY >= 9) { tempY = 8; }

		return Pos(tempX, tempY);
	}

	POSATTRIBUTES bool move(Direction moveDir) {

		switch (moveDir) {
			case Direction::Up:
				if(y == 0) {
					return false;
				}
				y -= 1;
				break;
			case Direction::Down:
				if(y == 8) {
					return false;
				}
				y += 1;
				break;
			case Direction::Left:
				if(x == 0) {
					return false;
				}
				x -= 1;
				break;
			case Direction::Right:
				if(x == 13) {
					return false;
				}
				x += 1;
				break;
			default:
				break;
		}

		return true;
	}

	POSATTRIBUTES bool moveInvert(Direction moveDir, bool invertHorizontal, bool invertVertical) {

		if(invertHorizontal) {
			if(moveDir == Direction::Left) {
				moveDir = Direction::Right;
			} else if(moveDir == Direction::Right) {
				moveDir = Direction::Left;
			}
		}

		if(invertVertical) {
			if(moveDir == Direction::Up) {
				moveDir = Direction::Down;
			} else if(moveDir == Direction::Down) {
				moveDir = Direction::Up;
			}
		}

		return move(moveDir);
	}

	POSATTRIBUTES bool operator==(Pos const& rhs) const { return x == rhs.x && y == rhs.y; }

	POSATTRIBUTES constexpr unsigned getSwitchValue() { // used to get a unsigned value for use in switch statements
		return (x << 4) | y;
	}

	POSATTRIBUTES bool sanity() const {
		// checks if im going insane.
		return x >= 0 && y >= 0 && x < 14 && y < 9;
	}

};

inline Pos safePos(POSTYPE x, POSTYPE y) {

	if(x < 0) { x = 0; }
	if(y < 0) { y = 0; }
	if(x >= 14) { x = 13; }
	if(y >= 9) { y = 8; }

	return Pos(x, y);
}

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const Pos& p) {

	stream.append("(");
	stream.append(p.x);
	stream.append(", ");
	stream.append(p.y);
	stream.append(")");

	return stream;
}

struct EntityHolder {
	const EntityType t;
	const unsigned short x;
	const unsigned short y;
};

struct EffectHolder {
	const bn::sprite_tiles_item* tiles;
	const int x;
	const int y;
	const int width;
	const int height;
	const bool collide = true;
	const int priority = 2;
	const bool autoAnimate = false;
};

struct SecretHolder {

	// holds the,,, yea secrets
	// if this secret is on top of an exit,, how do i track that?
	// ill just set a flag on load.
	// as for the 0 array issue, first thing will have a pos of -1, -1
	// if dest is null, just send to the next thing in the list? i suppose
	// ordering will have to be done manually, but tbh that is fine

	const int x;
	const int y;

	const char* dest;
	const char sanity = '\0'; // i dont trust this program.
};

class Room {
public:
	const void* collision;
	const void* floor;
	const void* details;

	const void* entities;
	const int entityCount;

	const void* effects;
	const int effectsCount;

	const void* secrets;
	const int secretsCount;

	const void* exitDest;

	const void* collisionTiles;
	const void* detailsTiles;

	constexpr Room(
		const void* collision_, const void* floor_, const void* details_,
		const void* entities_, const int entityCount_,
		const void* effects_, const int effectsCount_,
		const void* secrets_, const int secretsCount_,
		const void* exitDest_,
		const void* collisionTiles_, const void* detailsTiles_
	) :
	collision(collision_), floor(floor_), details(details_),
	entities(entities_), entityCount(entityCount_),
	effects(effects_), effectsCount(effectsCount_),
	secrets(secrets_), secretsCount(secretsCount_),
	exitDest(exitDest_),
	collisionTiles(collisionTiles_), detailsTiles(detailsTiles_)
	{ }

};

//template <size_t Size>
//using EntitySetIterator = EntitySet<Size>::iterator;
//using EntitySetIterator = bn::unordered_set::iterator;

// set without insanity. slower? def, but im so (curse)ing done
// ill haeve the backend be a vector.
// resizing on every insert,,, gods
// i however, cannot give a (curse) anymore :)
// spent way to long on this only to stop being a (curse) and understand what pointer stability is
// i could maybe use,, unique pointer? but tbh i just dont want to
// actually, we only have unique pointers! no shared, so im doing this
// insertion will be slow, but at least lookup will be fast.

// looking back on everything, the fact that this set exists is/was an objectively stupid oversight.
// actually maybe not?
// idek
// its not like many entities are being inserted/removed in a frame but
// well except during loading, but having them be inserted in there makes no difference
// the fact that most of saneset's use is in the gamemap, and that i have the insane "if constexpr" line
// thing means this was just premature optimization

#define SANESETATTRIBUTES
//#define SANESETATTRIBUTES __attribute__((section(".iwram")))
//#define SANESETATTRIBUTES __attribute__((target("arm"), section(".iwram")))

template <typename T, int maxVecSize>
class SaneSet {
private:

	bn::vector<T, maxVecSize> data;

	SANESETATTRIBUTES int binarySearch(const T& elem) const {
		int left = 0;
        int right = data.size() - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (data[mid] == elem) {
                return mid;
            } else if (data[mid] < elem) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
		return -1;
	}

	SANESETATTRIBUTES int getInsertIndex(const T& elem) const {
        int left = 0;
        int right = data.size();

        while (left < right) {
            int mid = left + (right - left) / 2;

            if (data[mid] < elem) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        return left;
    }

	SANESETATTRIBUTES int binarySearchOrInsertIndex(const T& elem) {
		int low = 0;
		int high = data.size() - 1;

		while (low <= high) {
			int mid = low + (high - low) / 2;

			if (data[mid] == elem) {
				return -1;
			} else if (data[mid] < elem) {
				low = mid + 1;
			} else {
				high = mid - 1;
			}
		}

		return low;
	}

public:

	SANESETATTRIBUTES SaneSet() {}

	SANESETATTRIBUTES SaneSet(const SaneSet& other) : data(other.data) {}

	SANESETATTRIBUTES SaneSet& operator=(const SaneSet& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }

	SANESETATTRIBUTES void insert(const T& elem) {

		// i doubt my code even ever actually,,, does a duplicate??

		if constexpr (maxVecSize == 4) {

			for(auto it = data.begin(); it != data.end(); ++it) {
				if(*it == elem) {
					return;
				}
			}

			data.push_back(elem);

		} else {

			int index = binarySearchOrInsertIndex(elem);

			if(index != -1) {
				data.insert(data.begin() + index, elem);
			}
		}
	}

	SANESETATTRIBUTES bn::vector<T, maxVecSize>::iterator erase(const T& elem) {

		if constexpr (maxVecSize == 4) {
			for(auto it = data.begin(); it != data.end(); ++it) {
				if(*it == elem) {
					return data.erase(it);

				}
			}
			return data.end();
		} else {

			int index = binarySearch(elem);

			if (index != -1) {
				return data.erase(data.begin() + index);
			}

			return data.end();
		}
    }

	SANESETATTRIBUTES bn::vector<T, maxVecSize>::iterator insert(const bn::vector<T, maxVecSize>::iterator it) {

		if constexpr (maxVecSize == 4) {

			for(auto temp = data.cbegin(); temp != data.cend(); ++temp) {
				if(*temp == *it) {
					return;
				}
			}

			return data.insert(data.end(), *it);
		} else {

			int index = binarySearchOrInsertIndex(*it);

			if(index == -1) {
				return data.begin() + index;
			}

			return data.insert(data.begin() + index, *it);
		}
	}

	SANESETATTRIBUTES bn::vector<T, maxVecSize>::iterator erase(const bn::vector<T, maxVecSize>::iterator it) {
		return data.erase(it);
	}

	SANESETATTRIBUTES bool contains(const T& elem) const {

		if(data.size() == 0) {
			return false;
		}

		if constexpr (maxVecSize == 4) {

			for(auto it = data.cbegin(); it != data.cend(); ++it) {
				if(*it == elem) {
					return true;
				}
			}

			return false;
		} else {
			return binarySearch(elem) != -1;
		}
	}

	SANESETATTRIBUTES int size() const {
		return data.size();
	}

	SANESETATTRIBUTES auto begin() {
		return data.begin();
	}

	SANESETATTRIBUTES auto end() {
		return data.end();
	}

	SANESETATTRIBUTES auto cbegin() const {
		return data.cbegin();
	}

	SANESETATTRIBUTES auto cend() const {
		return data.cend();
	}

	SANESETATTRIBUTES void clear() {
		data.clear();
	}

	SANESETATTRIBUTES int maxSize() const {
		return maxVecSize;
	}

};

template <typename T, int maxVecSize>
class SaneVector : public bn::vector<T, maxVecSize> { // war crime
public:

	SaneVector(std::initializer_list<T> l) {
		for(auto it = l.begin(); it != l.end(); ++it) {
			this->push_back(*it);
		}
	}

};

inline char* strcpy(char* dest, const char* src) {
	char *original_dest = dest;

	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}

	*dest = '\0';

	return original_dest;
}

inline char* strncpy_custom(char* dest, const char* src, size_t n) {
    char *dest_start = dest;

    while (*src != '\0' && n > 0) {
        *dest++ = *src++;
        n--;
    }

    // If n is greater than the length of src, fill the remaining with null characters.
    while (n > 0) {
        *dest++ = '\0';
        n--;
    }

    return dest_start;
}

inline void* memset(void* ptr, int value, size_t num) {
	unsigned char *byte_ptr = (unsigned char *)ptr;
	unsigned char byte_value = (unsigned char)value;

	for (size_t i = 0; i < num; i++) {
		byte_ptr[i] = byte_value;
	}

	return ptr;
}

inline int strcmp(const char *str1, const char *str2) {

	if(WTF(str1) != WTF(str2)) {
		return -1;
	}

	while (*str1 != '\0' && *str2 != '\0') {
		if (*str1 != *str2) {
			return (*str1 - *str2);
		}
		str1++;
		str2++;
	}

	return 0;
}

constexpr unsigned hashString(const char* str) {
	// the majority of hashString calls are at compile time.
	// a (slightly) more expensive hash func will benifit sanity

	unsigned hash = 0;

	while(*str) {
		hash = (hash * 31) + (*str);
		//hash = (hash << 3) | (hash >> 29);
		str++;
	}

	return hash;
}

inline char* strstrCustom(const char* haystack, const char* needle) {
	if (*needle == '\0') {
		return (char*)haystack;
	}

	while (*haystack != '\0') {
		const char* h = haystack;
		const char* n = needle;

		while (*n != '\0' && *h == *n) {
			h++;
			n++;
		}


		if (*n == '\0') {
			return (char*)haystack;
		}

		haystack++;
	}

	return NULL;
}
