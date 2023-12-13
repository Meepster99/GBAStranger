#pragma once 

#include "SharedTypes.h"
#include "Tile.h"

#include <functional>

class Game;
class EntityManager;
class EffectsManager;
class TileManager;
class CutsceneManager;
class Effect;

class Sprite {
public:
	
	// now that im extremely deep into this project, ive realized 
	// that having this class EXTEND sprite_ptr would actually be what i wanted it to be 
	// bc all i use this for is for constructors.
	
	bn::sprite_ptr spritePointer;
		
	bn::fixed screenx = 0;
	bn::fixed screeny = 0;
	
	static Palette* spritePalette;

	Sprite(const bn::sprite_tiles_item& startTilesItem) : 
		spritePointer(
			//bn::sprite_ptr::create(bn::fixed(0), bn::fixed(0), 
			bn::sprite_ptr::create(bn::fixed(-120), bn::fixed(-80), 
			bn::sprite_shape_size(16, 16),
			startTilesItem.create_tiles(),
			spritePalette->getSpritePalette().create_palette())
			)
		{
		spritePointer.set_z_order(0);
		spritePointer.set_bg_priority(1);
		spritePointer.set_blending_enabled(false);
		}
		
	Sprite(const bn::sprite_tiles_item& startTilesItem, const bn::sprite_shape_size& size) : 
		spritePointer(
			bn::sprite_ptr::create(bn::fixed(0), bn::fixed(0), 
			size,
			startTilesItem.create_tiles(),
			spritePalette->getSpritePalette().create_palette())
			)
		{
		spritePointer.set_z_order(0);
		spritePointer.set_bg_priority(1);
		spritePointer.set_blending_enabled(false);
		}
		
	Sprite(const bn::sprite_tiles_item& startTilesItem, const bn::sprite_shape_size& size, int x, int y) : 
		spritePointer(
			bn::sprite_ptr::create(bn::fixed(0), bn::fixed(0), 
			size,
			startTilesItem.create_tiles(),
			spritePalette->getSpritePalette().create_palette())
			)
		{
		spritePointer.set_z_order(0);
		spritePointer.set_bg_priority(1);
		updateRawPosition(x + (size.width() / 2), y);
		spritePointer.set_blending_enabled(false);
		}
		
	Sprite(const bn::sprite_item& startItem) : 
		spritePointer(
			bn::sprite_ptr::create(bn::fixed(0), bn::fixed(0), 
			startItem
			))
		{
		spritePointer.set_z_order(0);
		spritePointer.set_bg_priority(1);
		spritePointer.set_palette(spritePalette->getSpritePalette().create_palette());
		spritePointer.set_blending_enabled(false);
		}
	
		
	USEEWRAM void updatePosition(const Pos& p) {
		screenx = p.x * 16 - 8 - (6 * 16); 
		screeny = p.y * 16 - (4 * 16);
		spritePointer.set_x(screenx);
		spritePointer.set_y(screeny);
	}
	
	USEEWRAM void updateRawPosition(const int x, const int y) {
		screenx = x - 224/2 + 8;
		screeny = y - 144/2 + 8;
		spritePointer.set_x(screenx);
		spritePointer.set_y(screeny);
	}
	
	USEEWRAM Pos getCurrentScreenPos() const {
		return Pos((screenx.integer() + 8 + (6 * 16))/16, (screeny.integer() + (4 * 16))/16);
	}
	
	USEEWRAM void setRawX(const bn::fixed x) {
		screenx = x - 224/2 + 8;
		spritePointer.set_x(screenx);
	}
	
	USEEWRAM void setRawY(const bn::fixed y) {
		screeny = y - 144/2 + 8;
		spritePointer.set_y(screeny);
	}
	
	USEEWRAM void setVisible(bool vis) {	
		spritePointer.set_visible(vis);
	}
	
};

// -----

class Entity {
public:

	Pos p;
	Direction currentDir = Direction::Down;
	
	bn::vector<bn::sprite_tiles_item , 8> spriteTilesArray;
	Sprite sprite;
	
	// making these static should save memory.
	static EntityManager* entityManager;
	static EffectsManager* effectsManager;
	static TileManager* tileManager;
	static CutsceneManager* cutsceneManager;
	static Game* game;
	
	//bn::vector<bn::pair<bn::sprite_tiles_item, int>, 4> fallData;
	
	// THIS VARIABLE SHOULD BE MADE STATIC!!!!
	int animationIndex = 0;
	int tileIndex = 0;
	
	bn::optional<EntityType> deathReason;
		
	Entity(Pos p_) : p(p_), 
		spriteTilesArray(1, bn::sprite_tiles_items::dw_spr_gray_w_d),
		sprite(spriteTilesArray[0])
		{ 
		
		BN_ASSERT(p.sanity(), "point sanity failed in entity constructor?");
		
		}

	virtual ~Entity() = default;
	
	// these funcs should rllt be changed to const static!
	virtual bool isEnemy() const = 0;
	virtual bool isObstacle() const = 0;
	virtual bool isPlayer() const = 0;
	virtual bool canFall() const = 0;
	virtual bool canPush() const = 0;
	
	virtual Entity* clone() const = 0;
	virtual EntityType entityType() const = 0;
	
	USEEWRAM virtual bn::optional<Direction> getNextMove() = 0;
	virtual void moveFailed() = 0;
	virtual void moveSucceded() = 0;

	virtual void updateTileIndex() {
		
		tileIndex = static_cast<int>(currentDir);
		
		BN_ASSERT(tileIndex < spriteTilesArray.size(), "tried loading a tileIndex out of the sprite array bounds! ", __PRETTY_FUNCTION__);
		
	}
	
	virtual void doTick() {
		
		updateTileIndex();

		animationIndex = animationIndex + 1;
		//animationIndex = animationIndex % spriteTilesArray[tileIndex].graphics_count();
		// why when i was going through and fixing unneeded modulos earlier did i not catch this?? this is going to be huge!
		if(animationIndex >= spriteTilesArray[tileIndex].graphics_count()) {
			animationIndex = 0;
		}
	
	
		// should this be here?
		sprite.spritePointer.set_tiles(
			spriteTilesArray[tileIndex],
			animationIndex
		);
	
		// should this be called here?
		//doUpdate();
	}

	virtual void doUpdate() {

		updateTileIndex();
		
		// just incase
		//animationIndex = animationIndex % spriteTilesArray[tileIndex].graphics_count();
		
		if(animationIndex >= spriteTilesArray[tileIndex].graphics_count()) {
			animationIndex = 0;
		}
		
		sprite.spritePointer.set_tiles(
			spriteTilesArray[tileIndex],
			animationIndex
		);
		
		sprite.updatePosition(p);
	}
	
	USEEWRAM void updatePosition() {
		sprite.updatePosition(p);
	}
	
	virtual void isDead();// { return; } // needed for,,, lev statue falls?
	
	virtual void killedPlayer() { 
		// this func will be called if this entity kills a player, and do all the animations and suc
		// should this be depracated??
		
		doTick();
	}
	
private:
	bool isFalling = false;

};

// -----

class Player : public Entity {
public:
	
	Player(Pos p_);
	
	bool isEnemy() const override { return false; }
	bool isObstacle() const override { return false; }
	bool isPlayer() const override { return true; }
	bool canFall() const override { return true; }
	bool canPush() const override { return true; }
	
	Player* clone() const override { return new Player(*this); }
	EntityType entityType() const override { return EntityType::Player; }
	
	bool hasRod = true;
	bool hasSuperRod = false;
	//FloorTile* rod = NULL;
	// ik i should be useing a queue, but your honor, im (curse)ing tired, and my tummy hurt
	bn::vector<FloorTile*, 128> rod; 
	
	void pushRod(Pos tilePos);
	void popRod(Pos tilePos);
	bool inRod(FloorTile* tile);
	
	int locustCount = 0;
	bool isVoided = false;
	
	bool hasMemory = false;
	bool hasWings = false;
	bool hasSword = false;
	
	bool hasWingsTile = false;
	
	// jank as (curse)
	int wingsUse = 0;
	unsigned wingMoveCheck = -1;
	
	//void startFall() override;
	
	bn::optional<Direction> nextMove;
	
	bn::optional<Direction> getNextMove() override;

	bn::pair<bool, bn::optional<Direction>> doInput();
	
	void updateTileIndex() override;
	
	int pushAnimation = 0;
	
	void moveFailed() override { return; }
	void moveSucceded() override { return; }
	
};

// -----

class Enemy : public Entity {
public:	
	
	Enemy(Pos p_) : Entity(p_) {}
	
	bool isEnemy() const override { return true; }
	bool isObstacle() const override { return false; }
	bool isPlayer() const override { return false; }
	bool canPush() const override { return false; }
	
	bn::optional<Direction> getNextMove() override;
	
	void moveFailed() override { return; }
	void moveSucceded() override { return; }
	
};

class Obstacle : public Entity {
public:	

	// contains the list of bumps done to this obstacle this tick/cycle/move, gods i need to get a more consistent wording.
	bn::vector<Direction, 4> bumpDirections;
	
	// for things like,,,, hitting a chest3/6 times, or add statue
	// programming this is going to suck
	int specialBumpCount = 0;
	int playerIdleStart = 0;
	bool wasMoved = false;
	
	// this is so dumb. i should make it such that the only thing with these things are interactables.
	virtual void specialBumpFunction() { return; }
	
	Obstacle(Pos p_) : Entity(p_) {}
	
	bool isEnemy() const override { return false; }
	bool isObstacle() const override { return true; }
	bool isPlayer() const override { return false; }
	bool canFall() const override { return true; }
	bool canPush() const override { return false; }
	
	bn::optional<Direction> getNextMove() override;
	
	void moveFailed() override;
	void moveSucceded() override;
	
	void updateTileIndex() override {
		tileIndex = 0;
	}
	
	void doTick() override { return; }
	
	// THIS MIGHT BE VERY BAD!
	
	// i can,,,, maybe save calls by only calling doUpdate on the first
	
	//void startFall() override;
	
	virtual void interact() { return; }
	
	virtual bool kicked();

};

// -----

class Leech : public Enemy {
public:

	Leech(Pos p_) : Enemy(p_) {

		currentDir = Direction::Right;
	
		spriteTilesArray.clear(); 
        // insert two blanks so that i can use the default dotick and direction methods
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cl_left);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cl_right);
		
		
		
	}

	Leech* clone() const override { return new Leech(*this); }

	EntityType entityType() const override { return EntityType::Leech; }
	
	bool canFall() const override { return false; }
	
	void moveFailed() override {
		if(currentDir == Direction::Right) {
			currentDir = Direction::Left;
		} else {
			currentDir = Direction::Right;
		}
		return;
	}

};

class Maggot : public Enemy {
public:

	Maggot(Pos p_) : Enemy(p_) {
		currentDir = Direction::Down;
		
		spriteTilesArray.clear(); 

		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cc_up);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cc_down);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cc_falling, 6));
	}

	Maggot* clone() const override { return new Maggot(*this); }

	EntityType entityType() const override { return EntityType::Maggot; }
	
	bool canFall() const override { return false; }

	void moveFailed() override {
		if(currentDir == Direction::Up) {
			currentDir = Direction::Down;
		} else {
			currentDir = Direction::Up;
		}
		return;
	}
	
};

class Eye : public Enemy {
public:

	Eye(Pos p_) : Enemy(p_) {
		
		spriteTilesArray.clear(); 

		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_ch);	
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_ch_falling, 6));
	}

	Eye* clone() const override { return new Eye(*this); }

	EntityType entityType() const override { return EntityType::Eye; }
	
	bool canFall() const override { return false; }
	
	bn::optional<Direction> getNextMove() override { return bn::optional<Direction>(); }

};

class Bull : public Enemy {
public:

	Bull(Pos p_) : Enemy(p_) {
		
		spriteTilesArray.clear(); 

		// movement
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cg_up);	
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cg_down);	
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cg_left);	
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cg_right);	

		// idle 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cg_idle);	
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cg_falling, 6));

	}
	
	bn::optional<Direction> nextMove;
	
	bool idle = true;

	Bull* clone() const override { return new Bull(*this); }

	EntityType entityType() const override { return EntityType::Bull; }
	
	bool canFall() const override { return false; }

	bn::optional<Direction> getNextMove() override;
	
	void moveFailed() override;
	
	void updateTileIndex() override {
		if(idle) {
			tileIndex = 4;
		} else {
			Entity::updateTileIndex();
		}
	}
	
};

class Chester : public Enemy {
public:

	Chester(Pos p_) : Enemy(p_) {

		currentDir = Direction::Right;
	
		spriteTilesArray.clear(); 

		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_gray_w_d);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cs_left);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cs_right);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cs_fall, 8));
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cs_falling, 6));
		
	}
	
	bn::optional<Direction> nextMove;
	
	bool idle = true;
	
	void moveFailed() override;

	Chester* clone() const override { return new Chester(*this); }

	EntityType entityType() const override { return EntityType::Chester; }
	
	bool canFall() const override { return true; }
	
	bn::optional<Direction> getNextMove() override;
	
	void updateTileIndex() override {
		if(currentDir == Direction::Right) {
			tileIndex = 3;
		} else if(currentDir == Direction::Left) {
			tileIndex = 2;
		}
	}

};

class Mimic : public Enemy {
public:
	const bool invertHorizontal = false;
	const bool invertVertical = false;
	
	bn::optional<Direction> nextMove;
	
	Mimic(Pos p_, bool invertHorizontal_, bool invertVertical_) : Enemy(p_),
		invertHorizontal(invertHorizontal_),
		invertVertical(invertVertical_)
	{
		
	}
	
	bool canPush() const override { return true; }
	
	bn::optional<Direction> getNextMove() override;
	
};

class WhiteMimic : public Mimic {
public:

	WhiteMimic(Pos p_) : Mimic(p_, true, false) {
		
		currentDir = Direction::Down;
		
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_up);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_down);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_left);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_right);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cm_falling, 6));	
	}

	WhiteMimic* clone() const override { return new WhiteMimic(*this); }

	EntityType entityType() const override { return EntityType::WhiteMimic; }
	
	bool canFall() const override { return true; }

};

class GrayMimic : public Mimic {
public:

	GrayMimic(Pos p_) : Mimic(p_, false, true) {
		
		currentDir = Direction::Up;
		
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_up1);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_down1);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_left1);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_right1);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cm_falling1, 6));	
		
	}

	GrayMimic* clone() const override { return new GrayMimic(*this); }

	EntityType entityType() const override { return EntityType::GrayMimic; }
	
	bool canFall() const override { return true; }

};

class BlackMimic : public Mimic {
public:

	BlackMimic(Pos p_) : Mimic(p_, true, true) {
		
		currentDir = Direction::Up;
		
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_up2);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_down2);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_left2);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cm_right2);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_cm_falling2, 6));	
		
	}

	BlackMimic* clone() const override { return new BlackMimic(*this); }

	EntityType entityType() const override { return EntityType::BlackMimic; }

	bool canFall() const override { return true; }
	
};

class Diamond : public Enemy {
public:

	Diamond(Pos p_) : Enemy(p_) {
		
		spriteTilesArray.clear(); 
		
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_co_move);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_co_move);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_co_move);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_co_move);

		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_co_idle);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_co_fall, 2));		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_co_falling, 6));		
	}
	
	bn::optional<Direction> nextMove;

	Diamond* clone() const override { return new Diamond(*this); }

	EntityType entityType() const override { return EntityType::Diamond; }
	
	bool canFall() const override { return false; }
	
	bool idle = true;
	
	bn::optional<Direction> getNextMove() override;
	
	void updateTileIndex() override {
		if(idle) {
			tileIndex = 4;
		} else {
			Entity::updateTileIndex();
		}
	}

};

class Shadow : public Enemy {
public:

	Shadow(Pos p_) : Enemy(p_) {
		
		spriteTilesArray.clear(); 
		
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cr_up);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cr_down);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cr_left);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_cr_right);
		
		// here as a temporary measure
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_co_falling, 6));		
		
	}

	// the following may be a very bad idea
	// this allows shadows to properly die when an enemy TOUCHES, not collides with them, but,,, i dont like it.
	// erodes trust in my funcs.
	bool isEnemy() const override { return false; }
	bool isPlayer() const override { return true; }
	
	Shadow* clone() const override { return new Shadow(*this); }

	EntityType entityType() const override { return EntityType::Shadow; }

	bool canFall() const override { return false; }
	
	bn::optional<Direction> getNextMove() override { return bn::optional<Direction>(); }
	
	// this is overriden since all shadows need to share the same tick animation state. 
	// now that i think abt it, doesnt everything? 
	// well yes, bug i think since, shadows are the only entities that can be created on,,, any time after the room loads 
	// then they arent const 
	// but maybe this is overkill? i could just set the anim index whenever a new shadow is created.
	
};

// -----

class Boulder : public Obstacle {
public:

	Boulder(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_boulder);
		
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	Boulder* clone() const override { return new Boulder(*this); }

	EntityType entityType() const override { return EntityType::Boulder; }
	
	const char* specialDialogue = NULL;
	
	void interact() override;

};

class Chest : public Obstacle {
public:

	bool gotBonus = false;
	
	Chest(Pos p_, bool isEmpty = false);

	Chest* clone() const override { return new Chest(*this); }

	EntityType entityType() const override { return EntityType::Chest; }
	
	bn::optional<Direction> getNextMove() { 
		bumpDirections.clear();
		return bn::optional<Direction>(); 
	}
	
	int interactCount = 0;
	void interact() override; // spr_textbox_extra
	
	void specialBumpFunction() override;
	
};

class AddStatue : public Obstacle {
public:

	AddStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_voider);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	AddStatue* clone() const override { return new AddStatue(*this); }

	EntityType entityType() const override { return EntityType::AddStatue; }
	
	// all of this is shit code
	std::function<void(AddStatue*)> specialBumpFunctionPointer = [](AddStatue* obj) -> void {
		(void)obj;
		return;
	};
	
	void specialBumpFunction() override { 
		specialBumpFunctionPointer(this);
	}

};

class EusStatue : public Obstacle {
public:

	EusStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_lover);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	EusStatue* clone() const override { return new EusStatue(*this); }

	EntityType entityType() const override { return EntityType::EusStatue; }
	
	void isDead() override;

};
	
class BeeStatue : public Obstacle {
public:

	BeeStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_smiler);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	BeeStatue* clone() const override { return new BeeStatue(*this); }

	EntityType entityType() const override { return EntityType::BeeStatue; }

};

class MonStatue : public Obstacle {
public:

	MonStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_greeder);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	MonStatue* clone() const override { return new MonStatue(*this); }

	EntityType entityType() const override { return EntityType::MonStatue; }
	
	bn::optional<Direction> getNextMove() override;

};

class TanStatue : public Obstacle {
public:

	TanStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_killer);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	TanStatue* clone() const override { return new TanStatue(*this); }

	EntityType entityType() const override { return EntityType::TanStatue; }
	
	void isDead() override;

};

class GorStatue : public Obstacle {
public:

	const Pos startPos;

	GorStatue(Pos p_) : Obstacle(p_), startPos(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_slower);
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_slower_stop);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	GorStatue* clone() const override { return new GorStatue(*this); }

	EntityType entityType() const override { return EntityType::GorStatue; }
	
	bn::optional<Direction> getNextMove() override;
	
	void updateTileIndex() override { }
	
};

class LevStatue : public Obstacle {
public:

	static int rodUses;
	static int totalLev;
	
	Effect* activeEffect = NULL;

	LevStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_watcher);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
		
		totalLev++;
	}
	
	~LevStatue();
	
	void isDead() override;
	
	bool isActive = false;
	void activate();

	LevStatue* clone() const override { return new LevStatue(*this); }

	EntityType entityType() const override { return EntityType::LevStatue; }

};

class CifStatue : public Obstacle {
public:

	CifStatue(Pos p_) : Obstacle(p_) {
		spriteTilesArray.clear(); 
		spriteTilesArray.push_back(bn::sprite_tiles_items::dw_spr_atoner);
		//fallData.push_back(bn::pair<bn::sprite_tiles_item, u8>(bn::sprite_tiles_items::dw_spr_fall, 6));
	}

	CifStatue* clone() const override { return new CifStatue(*this); }

	EntityType entityType() const override { return EntityType::CifStatue; }

};

// 

class Interactable : public Obstacle {
public:

	
	// oh gods. oh. gods.
	// this,,, this function, this whole class. what am i on
	// there rlly should be a way to pass the function with the params inside of it, but im tired, and this works
	std::function<void(void*)> interactFunc;
	std::function<bool(void*)> kickFunc;
	
	void* interactFuncParams;
	void* kickFuncParams;
	
	std::function<void(Interactable*)> specialBumpFunc = NULL;

	// i dont like this naming convention at all
	
	// now that i have access to functional, i rlly should rewrite this.(if i can?)
	// could i,,, instead of passing shit in seperately like,,, just have lambda captures capture the param 
	// omfg, thats what i should of done
	Interactable(Pos p_, std::function<void(void*)> interactFunc_, std::function<bool(void*)> kickFunc_, void* interactFuncParams_, void* kickFuncParams_, 
		std::function<void(Interactable*)> initFunc = NULL,
		std::function<void(Interactable*)> specialBumpFunc_ = NULL
		) : 
		Obstacle(p_), 
		interactFunc(interactFunc_), kickFunc(kickFunc_),
		interactFuncParams(interactFuncParams_), kickFuncParams(kickFuncParams_), specialBumpFunc(specialBumpFunc_)
		{
			
			if(interactFuncParams == NULL) {
				interactFuncParams = this;
			}
			if(kickFuncParams == NULL) {
				kickFuncParams = this;
			}
			
		//sprite.setVisible(true);
		sprite.setVisible(false);
		//sprite.spritePointer.set_bg_priority(0);
		if(initFunc != NULL) {
			initFunc(this);
		}
	}
	
	// i think this func should be depracated by now right?
	Interactable* clone() const override { return new Interactable(*this); }
	
	EntityType entityType() const override { return EntityType::Interactable; }
	
	// this might be stupid. but should i make it pass this if the params r null? 
	// this func was originally here to interact with things like bigsprites,,,idek
	//,,,, fuck it. we ball
	void interact() override { bumpDirections.clear(); return interactFunc(interactFuncParams); }
	bool kicked() override { bumpDirections.clear(); return kickFunc(kickFuncParams); }
	
	bn::optional<Direction> getNextMove() { 
		bumpDirections.clear();
		return bn::optional<Direction>(); 
	}
	
	void specialBumpFunction() {
		if(specialBumpFunc != NULL) {
			specialBumpFunc(this);
		}
		return; 
	}
	
};



