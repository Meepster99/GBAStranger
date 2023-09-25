#pragma once

#include "dataWinIncludes.h"

typedef unsigned char u8;

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

enum class Direction {
    Up,
    Down,
    Left,
    Right
};
	
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

enum class TileType {
	Pit,
	Floor,
	Glass,
	Bomb,
	Death,
	Copy,
	Exit,
	Switch,
};

class Pos {
public:

	int x;
	int y;

	constexpr Pos(int x_, int y_) : x(x_), y(y_) { 
		BN_ASSERT(x >= 0 && y >= 0 && x < 14 && y < 9, "invalid pos created at ", x, " ", y);
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
	
	bool operator==(Pos const& rhs) const { return x == rhs.x && y == rhs.y; }

};

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

template <size_t Size>
using EntitySet = bn::unordered_set<Entity*, Size, bn::hash<Entity*>, bn::equal_to<Entity*>>;

template <size_t Size>
using EntitySetIterator = EntitySet<Size>::iterator;

