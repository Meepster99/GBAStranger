#pragma once

#include "dataWinIncludes.h"

#include "bn_sound.h"

//#include "bn_music_items.h"
//#include "bn_music_actions.h"

#include "bn_sound_items.h"
#include "bn_sound_actions.h"

#include "bn_log.h"
#include "bn_config_log.h"
#include "bn_log_backend.h"
#include "Palette.h"

// getting this include to work was such a painful process for reasons i still dont get. a make clean fixed them(i think)?
#include "bn_profiler.h"
//#include "bn_config_profiler.h"

#include <bn_deque.h>

#include "fontData.h"

 
// i rlly wish that profiling could just be done 
// for all funcs 

// THE PROFILER DOESNT ALLOW FOR NESTED PROFILING??? WHY????
// at this point i should just make my own profiler 
// wait,,, 
// thats not the worst idea.
// or i could use their profiler with a stack based system?
// the only annoying thing would be,, having to write my own display system.
// so ill try writing a stack system.
// actually, if we have the profiler keep track of the id it should swap back to,,
// hehe we are going to have to access a underscore namespace hehehehehe
// im doing a bad

// ok so, because of them using "good c++ practices" i cannot rawdog the _profiler to get what i want
// maybe a static variable in here to keep track of it instead?
// this has got to be one of the weirdest things i have ever wrote.
// should i have written a stack/queue? maybe? i have like,, 
// gods i wonder if all the extra allocation from this thing will also cause slowdown
// but tbh, it works, and ill take it

// if the func name was to long, it was fucking overflowing butano(i think)
// wonderful.
// i could have this be fixed in the define, but the issue is then that time 
// that string funcs were occuring would be added to the timers.
// gods is there a way to do this during compile time? there really should
// of course, if the profiler had actual bounds checking




class Profiler {
public:

	const char* prevID;
	static const char* currentID;
	
	static int WTF(const char* str) {
		
		// look.
		// i think, that when the compiler saw this func, it saw it was basically strlen, and tried 
		// to just sub that in, only for strlen to not exist for some unknown ungodly reasons
		// this is horrid.
		
		int length = 0;
		
		while (*str != '\0') {
			++length;
			++length;
			++str;
		} 
		return length >> 1;
	}

    Profiler(const char* ID) { 
	
		BN_ASSERT(WTF(ID) < BN_CFG_ASSERT_BUFFER_SIZE, "you are fucked");
	
		BN_PROFILER_STOP();
		prevID = currentID;
		BN_PROFILER_START(ID); 
		currentID = ID;
		}
		
    ~Profiler() { 
		BN_PROFILER_STOP();
		currentID = prevID;
		BN_PROFILER_START(prevID); 
	}
};

/*
#define profileFunction() \
	volatile Profiler PROFILEROBJ(__PRETTY_FUNCTION__);
*/

template <std::size_t N>
constexpr const char* extractFunctionName(const char (&str)[N], std::size_t i = 0) {
    return (str[i] == ' ') ? (str + i + 1) : extractFunctionName(str, i + 1);
}

#define profileFunction() \
    static constexpr const char* function_name = extractFunctionName(__PRETTY_FUNCTION__); \
    Profiler<function_name> profiler;
	
typedef unsigned char u8;

// unsure of accuracy, but will make things slightly easier, hopefully
#define FRAMETICKS 4372

// TODO, GO OVER ALL FUNCS, AND DEFINE WHAT CAN BE AS CONST REF
// idrk if c++ optimization does that for me? but regardless its a good idea

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define MAXSPRITES 128

#define MAXDEBUGSPRITES 48

#define MAXENTITYSPRITES MAXSPRITES - MAXDEBUGSPRITES

extern unsigned int frame;

enum class GameState {
	Normal, // normal gameplay
	Exiting, // we either just completed a level, or just died
	Entering, // we just exited the exiting state and are now reloading stuffs. 
	Loading, // loading in new data, do nothing for now.
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
	"WordTile,"
};

	stream << TileTypeToString[static_cast<int>(e)];
	return stream;

}

class BackgroundMap {
public:

	bn::regular_bg_map_cell cells[32 * 32];
	bn::regular_bg_map_item mapItem;
	bn::regular_bg_item bg_item;
	bn::regular_bg_ptr bgPointer;
	bn::regular_bg_map_ptr bgMap;

	static Palette* backgroundPalette;
	
	// bn::bg_palette_item palette
	// how in tarnation can butano look you in the face, and say "yea do this for a background map"
	BackgroundMap(bn::regular_bg_tiles_item tileset, int zIndex) :
		mapItem(cells[0], bn::size(32, 32)),
		
		bg_item(tileset, 
		
		backgroundPalette->getBGPalette()
		
		,mapItem),
		bgPointer(bg_item.create_bg(8, 48)),
		bgMap(bgPointer.map())
		{
			
			// for all tilesets from the game, tile 0 
			// is just a checkerboard. if possible, i rlly, and i mean really, 
			// should overwrite that with transparent here.
			// actually, fuck it, ill just do it in preprocessing.
			
			
			//bgPointer.set_z_order(zIndex);
			bgPointer.set_priority(zIndex);
			
			//setTile(0,21,4);
			//setTile(31,31,4);

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
	
	void setTile(int x, int y, int tileIndex) {
		
		bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
		bn::regular_bg_map_cell_info current_cell_info(current_cell);

		current_cell_info.set_tile_index(tileIndex);
		current_cell = current_cell_info.cell(); 
	
		// if lag happens to occur, we could not update this until a frame is done
		// yep, lag occured.
		// sorta fixed it, but to be safe, im ballin here
		//bgMap.reload_cells_ref();
	}
	
	void setTile(int x, int y, int tileIndex, bool flipX, bool flipY) {
		
		bn::regular_bg_map_cell& current_cell = cells[mapItem.cell_index(x, y)];
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

	// dont ask
	Layer(bn::regular_bg_tiles_item tileset, int zIndex, int fillIndex = 0) : rawMap(tileset, zIndex)
		{
			

			//setup black border., just black the whole screen
			for(int i=0; i<30; i++) {
				for(int j=0; j<20; j++) {
			//for(int i=0; i<32; i++) {
			//	for(int j=0; j<32; j++) {
					rawMap.setTile(i, j, fillIndex);
				}
			}
			rawMap.reloadCells();
		}
	virtual ~Layer() = default;
	
	virtual void draw(u8 (&gameMap)[14][9]) {

		for(int x=0; x<14; x++) {
			for(int y=0; y<9; y++) {
				
				u8 tile = gameMap[x][y];
			
				rawMap.setTile(x * 2 + 1, y * 2 + 1, 4 * tile); 
				rawMap.setTile(x * 2 + 2, y * 2 + 1, 4 * tile + 1); 
				rawMap.setTile(x * 2 + 1, y * 2 + 2, 4 * tile + 2); 
				rawMap.setTile(x * 2 + 2, y * 2 + 2, 4 * tile + 3); 
			}
		}
		
		rawMap.reloadCells();
	}
	
	void setTile(int x, int y, int tileIndex) { rawMap.setTile(x, y, tileIndex); }
	void setTile(int x, int y, int tileIndex, bool flipX, bool flipY) { rawMap.setTile(x, y, tileIndex, flipX, flipY); }
	
};

class Pos {
public:

	signed char x;
	signed char y;

	Pos(signed char x_, signed char y_) : x(x_), y(y_) { 
		BN_ASSERT(x >= 0 && y >= 0 && x < 14 && y < 9, "invalid pos created at ", x, " ", y);
	}
	
	Pos(const Pos& other) : x(other.x), y(other.y) {}
	
	Pos& operator=(const Pos& other) {
        if (this != &other) {
            x = other.x;
			y = other.y;
        }
        return *this;
    }
	
	bool operator<(const Pos& other) const {
		return (x + 14 * y) < (other.x + 14 * y);
	}
	
	Pos operator+(const Pos& other) {

		int tempX = x + other.x;
		int tempY = y + other.y;
		
		if(tempX < 0) { tempX = 0; }
		if(tempY < 0) { tempY = 0; }
		if(tempX >= 14) { tempX = 13; }
		if(tempY >= 9) { tempY = 8; }
		
		return Pos(tempX, tempY);
	}
	
	Pos operator-(const Pos& other) {

		int tempX = x - other.x;
		int tempY = y - other.y;
		
		if(tempX < 0) { tempX = 0; }
		if(tempY < 0) { tempY = 0; }
		if(tempX >= 14) { tempX = 13; }
		if(tempY >= 9) { tempY = 8; }
		
		return Pos(tempX, tempY);
	}
	
	bool move(Direction moveDir) {
		
		int newX = x;
		int newY = y;
		
		switch (moveDir) {
			case Direction::Up:
				newY -= 1;
				break;
			case Direction::Down:
				newY += 1;
				break;
			case Direction::Left:
				newX -= 1;
				break;
			case Direction::Right:
				newX += 1;
				break;
			default:
				break;
		}
		
		if(newX < 0 || newY < 0 || newX >= 14 || newY >= 9) {
			return false;
		}
		
		x = newX;
		y = newY;
		
		return true;
	}
	
	bool moveInvert(Direction moveDir, bool invertHorizontal, bool invertVertical) {
	
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
	
	bool operator==(Pos const& rhs) const { return x == rhs.x && y == rhs.y; }

	bool sanity() const {
		// checks if im going insane.
		return x >= 0 && y >= 0 && x < 14 && y < 9;
	}
	
};

inline Pos safePos(signed char x, signed char y) {

	if(x < 0) { x = 0; }
	if(y < 0) { y = 0; }
	if(x >= 14) { x = 13; }
	if(y >= 9) { y = 8; }
	
	return Pos(x, y);
}

inline bn::ostringstream& operator<<(bn::ostringstream& stream, const Pos& p) {
	//stream << "(" << p.x << ", " << p.y << ")";
	stream.append("(");
	stream.append(p.x);
	stream.append(", ");
	stream.append(p.y);
	stream.append(")");

	return stream;
}

struct EntityHolder {
	const EntityType t;
	const u8 x;
	const u8 y;
};

class Room {
public:
	const void* collision;
	const void* floor;
	const void* details;

	const void* entities;
	const int entityCount;

	constexpr Room(const void* collision_, const void* floor_, const void* details_, const void* entities_, const int entityCount_) :
	collision(collision_), floor(floor_), details(details_), entities(entities_), entityCount(entityCount_)
	{ }
	
};

#include "Palette.h"

class Entity;


//template <size_t Size>
//using EntitySet = bn::unordered_set<Entity*, Size, bn::hash<Entity*>, bn::equal_to<Entity*>>;

//template <size_t Size>
//using EntitySetIterator = EntitySet<Size>::iterator;
//using EntitySetIterator = bn::unordered_set::iterator;

// set without insanity. slower? def, but im so fucking done
// ill haeve the backend be a vector.
// resizing on every insert,,, gods
// i however, cannot give a fuck anymore :) 
// spent way to long on this only to stop being a dumbass and understand what pointer stability is
// i could maybe use,, unique pointer? but tbh i just dont want to 
// actually, we only have unique pointers! no shared, so im doing this
// insertion will be slow, but at least lookup will be fast.
template <typename T, int maxVecSize>
class SaneSet {
private:
	
	bn::vector<T, maxVecSize> data;

	int binarySearch(const T& elem) const {
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
	
	int getInsertIndex(const T& elem) const {
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
	
public:

	SaneSet() {}	

	SaneSet(const SaneSet& other) : data(other.data) {}
	
	SaneSet& operator=(const SaneSet& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }
	
	void insert(const T& elem) {
		int index = binarySearch(elem);
        if (index == -1) {
            // Element not found, insert it at the appropriate position
            int insertIndex = getInsertIndex(elem);
            data.insert(data.begin() + insertIndex, elem);
        }
	}
	
	bool contains(const T& elem) const {
		return binarySearch(elem) != -1;
	}
	
	bn::vector<T, maxVecSize>::iterator erase(const T& elem) {
        int index = binarySearch(elem);
        if (index != -1) {
            return data.erase(data.begin() + index);
        }
		return data.end();
    }
	
	bn::vector<T, maxVecSize>::iterator erase(const bn::vector<T, maxVecSize>::iterator it) {
		return data.erase(it);
	}
	
	bn::vector<T, maxVecSize>::iterator insert(const bn::vector<T, maxVecSize>::iterator it) {
		int index = binarySearch(*it);
        if (index == -1) {
            // Element not found, insert it at the appropriate position
            int insertIndex = getInsertIndex(*it);
            return data.insert(data.begin() + insertIndex, *it);
        }
		return data.begin() + index;
	}
	
	int size() const {
		return data.size();
	}
	
	// is returning these iterators,,, ok??
	
	auto begin() {
		return data.begin();
	}
	
	auto end() {
		return data.end();
	}
	
	auto cbegin() const {
		return data.cbegin();
	}
	
	auto cend() const {
		return data.cend();
	}
	
	void clear() {
		data.clear();
	}
	
	int maxSize() const {
		return maxVecSize;
	}
	
	
	
};


