#pragma once 

#include "SharedTypes.h"
#include "Tile.h"

class EntityManager;

class Sprite {
public:
	
	bn::sprite_ptr spritePointer;
		
	bn::fixed screenx = 0;
	bn::fixed screeny = 0;

	Sprite(bn::sprite_tiles_item startTilesItem) : 
		spritePointer(
			bn::sprite_ptr::create(bn::fixed(0), bn::fixed(0), 
			bn::sprite_shape_size(16, 16),
			startTilesItem.create_tiles(),
			defaultPalette.getSpritePalette().create_palette())
			)
		{ }
		
	void updatePosition(int x, int y) {
		screenx = x * 16 - 8 - (6 * 16);
		screeny = y * 16 - (4 * 16);
		spritePointer.set_x(screenx);
		spritePointer.set_y(screeny);
	}
	
	void setVisible(bool vis) {	
		spritePointer.set_visible(vis);
	}
	
};

// -----

class Entity {
public:

	Pos p;
	Direction currentDir = Direction::Down;
	
	EntityManager* entityManager;
	
	bn::vector<bn::sprite_tiles_item , 8> spriteTilesArray;
	Sprite sprite;
	
	Entity(Pos p_) : p(p_), 
		spriteTilesArray(1, bn::sprite_tiles_items::dw_spr_gray_w_d),
		sprite(spriteTilesArray[0])
		{ }

	virtual ~Entity() = default;
	
	virtual bool isEnemy() const = 0;
	virtual bool isObstacle() const = 0;
	virtual bool isPlayer() const = 0;
	virtual bool canFall() const = 0;
	
	virtual Entity* clone() const = 0;
	virtual EntityType entityType() const = 0;

};

// -----

class Player : public Entity {
public:
	
	Player(Pos p_) : Entity(p_) {
		
	}
	
	bool isEnemy() const override { return false; }
	bool isObstacle() const override { return false; }
	bool isPlayer() const override { return true; }
	bool canFall() const override { return true; }
	
	Player* clone() const override { return new Player(*this); }
	EntityType entityType() const override { return EntityType::Player; }
	
	FloorTile* rod = NULL;
};

// -----

class Enemy : public Entity {
public:	
	
	Enemy(Pos p_) : Entity(p_) {}
	
	bool isEnemy() const override { return true; }
	bool isObstacle() const override { return false; }
	bool isPlayer() const override { return false; }
	
	
};

class Obstacle : public Entity {
public:	

	// contains the list of bumps done to this obstacle this tick/cycle/move, gods i need to get a more consistent wording.
	bn::vector<Direction, 4> bumpDirections;
	
	Obstacle(Pos p_) : Entity(p_) {}
	
	bool isEnemy() const override { return false; }
	bool isObstacle() const override { return true; }
	bool isPlayer() const override { return false; }
	bool canFall() const override { return true; }
	
};

// -----

class Leech : public Enemy {
public:

	Leech(Pos p_) : Enemy(p_) {}

	Leech* clone() const override { return new Leech(*this); }

	EntityType entityType() const override { return EntityType::Leech; }
	
	bool canFall() const override { return false; }

};

class Maggot : public Enemy {
public:

	Maggot(Pos p_) : Enemy(p_) {}

	Maggot* clone() const override { return new Maggot(*this); }

	EntityType entityType() const override { return EntityType::Maggot; }
	
	bool canFall() const override { return false; }

};

class Eye : public Enemy {
public:

	Eye(Pos p_) : Enemy(p_) {}

	Eye* clone() const override { return new Eye(*this); }

	EntityType entityType() const override { return EntityType::Eye; }
	
	bool canFall() const override { return false; }

};

class Bull : public Enemy {
public:

	Bull(Pos p_) : Enemy(p_) {}

	Bull* clone() const override { return new Bull(*this); }

	EntityType entityType() const override { return EntityType::Bull; }
	
	bool canFall() const override { return false; }

};

class Chester : public Enemy {
public:

	Chester(Pos p_) : Enemy(p_) {}

	Chester* clone() const override { return new Chester(*this); }

	EntityType entityType() const override { return EntityType::Chester; }
	
	bool canFall() const override { return true; }

};

class Mimic : public Enemy {
public:
	const bool invertHorizontal = false;
	const bool invertVertical = false;
	
	Mimic(Pos p_, bool invertHorizontal_, bool invertVertical_) : Enemy(p_),
		invertHorizontal(invertHorizontal_),
		invertVertical(invertVertical_)
	{
		
	}
	
};

class WhiteMimic : public Mimic {
public:

	WhiteMimic(Pos p_) : Mimic(p_, true, false) {}

	WhiteMimic* clone() const override { return new WhiteMimic(*this); }

	EntityType entityType() const override { return EntityType::WhiteMimic; }
	
	bool canFall() const override { return true; }

};

class GrayMimic : public Mimic {
public:

	GrayMimic(Pos p_) : Mimic(p_, false, true) {}

	GrayMimic* clone() const override { return new GrayMimic(*this); }

	EntityType entityType() const override { return EntityType::GrayMimic; }
	
	bool canFall() const override { return true; }

};

class BlackMimic : public Mimic {
public:

	BlackMimic(Pos p_) : Mimic(p_, true, true) {}

	BlackMimic* clone() const override { return new BlackMimic(*this); }

	EntityType entityType() const override { return EntityType::BlackMimic; }

	bool canFall() const override { return true; }
	
};

class Diamond : public Enemy {
public:

	Diamond(Pos p_) : Enemy(p_) {}

	Diamond* clone() const override { return new Diamond(*this); }

	EntityType entityType() const override { return EntityType::Diamond; }
	
	bool canFall() const override { return false; }

};

class Shadow : public Enemy {
public:

	Shadow(Pos p_) : Enemy(p_) {}

	Shadow* clone() const override { return new Shadow(*this); }

	EntityType entityType() const override { return EntityType::Shadow; }

	bool canFall() const override { return false; }
	
};

// -----

class Boulder : public Obstacle {
public:

	Boulder(Pos p_) : Obstacle(p_) {}

	Boulder* clone() const override { return new Boulder(*this); }

	EntityType entityType() const override { return EntityType::Boulder; }

};

class Chest : public Obstacle {
public:

	Chest(Pos p_) : Obstacle(p_) {}

	Chest* clone() const override { return new Chest(*this); }

	EntityType entityType() const override { return EntityType::Chest; }

};

class AddStatue : public Obstacle {
public:

	AddStatue(Pos p_) : Obstacle(p_) {}

	AddStatue* clone() const override { return new AddStatue(*this); }

	EntityType entityType() const override { return EntityType::AddStatue; }

};

class EusStatue : public Obstacle {
public:

	EusStatue(Pos p_) : Obstacle(p_) {}

	EusStatue* clone() const override { return new EusStatue(*this); }

	EntityType entityType() const override { return EntityType::EusStatue; }

};

class BeeStatue : public Obstacle {
public:

	BeeStatue(Pos p_) : Obstacle(p_) {}

	BeeStatue* clone() const override { return new BeeStatue(*this); }

	EntityType entityType() const override { return EntityType::BeeStatue; }

};

class MonStatue : public Obstacle {
public:

	MonStatue(Pos p_) : Obstacle(p_) {}

	MonStatue* clone() const override { return new MonStatue(*this); }

	EntityType entityType() const override { return EntityType::MonStatue; }

};

class TanStatue : public Obstacle {
public:

	TanStatue(Pos p_) : Obstacle(p_) {}

	TanStatue* clone() const override { return new TanStatue(*this); }

	EntityType entityType() const override { return EntityType::TanStatue; }

};

class GorStatue : public Obstacle {
public:

	GorStatue(Pos p_) : Obstacle(p_) {}

	GorStatue* clone() const override { return new GorStatue(*this); }

	EntityType entityType() const override { return EntityType::GorStatue; }

};

class LevStatue : public Obstacle {
public:

	LevStatue(Pos p_) : Obstacle(p_) {}

	LevStatue* clone() const override { return new LevStatue(*this); }

	EntityType entityType() const override { return EntityType::LevStatue; }

};

class CifStatue : public Obstacle {
public:

	CifStatue(Pos p_) : Obstacle(p_) {}

	CifStatue* clone() const override { return new CifStatue(*this); }

	EntityType entityType() const override { return EntityType::CifStatue; }

};

