

#include "EntityManager.h"


void EntityManager::loadEntities(EntityHolder* entitiesPointer, int entitiesCount) {
	
	// delete old data 
	
	for(EntitySetIterator<128> it = entityList.begin(); it != entityList.end(); ++it) {
		Entity* temp = *it;
		delete temp;
	}
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			entityMap[x][y].clear();
		}
	}

	entityList.clear();
	enemyList.clear();
	obstacleList.clear();
	
	for(int i=0; i<entitiesCount; i++) {
		
		EntityHolder temp = entitiesPointer[i];
		Pos tempPos(temp.x, temp.y);
		
		switch(temp.t) {
			case EntityType::Player:
				player = new Player(tempPos);
				entityList.insert(player);
				break;
			case EntityType::Leech:
				entityList.insert(new Leech(tempPos));
				break;
			case EntityType::Maggot:
				entityList.insert(new Maggot(tempPos));
				break;
			case EntityType::Eye:
				entityList.insert(new Eye(tempPos));
				break;
			case EntityType::Bull:
				entityList.insert(new Bull(tempPos));
				break;
			case EntityType::Chester:
				entityList.insert(new Chester(tempPos));
				break;
			case EntityType::WhiteMimic:
				entityList.insert(new WhiteMimic(tempPos));
				break;
			case EntityType::GrayMimic:
				entityList.insert(new GrayMimic(tempPos));
				break;
			case EntityType::BlackMimic:
				entityList.insert(new BlackMimic(tempPos));
				break;
			case EntityType::Diamond:
				entityList.insert(new Diamond(tempPos));
				break;
			case EntityType::Shadow:
				entityList.insert(new Shadow(tempPos));
				break;
			case EntityType::Boulder:
				entityList.insert(new Boulder(tempPos));
				break;
			case EntityType::Chest:
				entityList.insert(new Chest(tempPos));
				break;
			case EntityType::AddStatue:
				entityList.insert(new AddStatue(tempPos));
				break;
			case EntityType::EusStatue:
				entityList.insert(new EusStatue(tempPos));
				break;
			case EntityType::BeeStatue:
				entityList.insert(new BeeStatue(tempPos));
				break;
			case EntityType::MonStatue:
				entityList.insert(new MonStatue(tempPos));
				break;
			case EntityType::TanStatue:
				entityList.insert(new TanStatue(tempPos));
				break;
			case EntityType::GorStatue:
				entityList.insert(new GorStatue(tempPos));
				break;
			case EntityType::LevStatue:
				entityList.insert(new LevStatue(tempPos));
				break;
			case EntityType::CifStatue:
				entityList.insert(new CifStatue(tempPos));
				break;
			default:
				BN_ERROR("unknown entity tried to get loaded in, wtf");
				break;
		}
		
	}
	
	// create starting map.
	for(EntitySetIterator<128> it = entityList.begin(); it != entityList.end(); ++it) {
		
		Entity* temp = *it;
		
		
		getMap(temp->p).insert(temp);
		
		if(temp->isEnemy()) {
			enemyList.insert(temp);
		} else if(temp->isObstacle()) {
			obstacleList.insert(temp);
		}
		
	}
	
	// TODO determine leech and maggot starting directions here.
	
	
	
	
	updateMap();
	
}

void EntityManager::doMove(Direction playerMove) {
	
	
	// insert player's move into its func 
	// intsert mimics move into its func 
	// (maybe) insert diamonds move into its func? (MORE NEEDS TO BE DONE ON THIS)
	// (maybe) insert shadows move into its func? (MORE NEEDS TO BE DONE ON THIS)
	
	
	// check if player is moving into an obstacle
	// move obstacle
	// if that obstacle kills something, remove it from all sublists.
	
	// for all mimics
	// check if mimic is moving into obstacle
	// average out the movement of the obstacle
	// then move all obstacles
	// if that obstacle kills something, remove it from all sublists.
	
	// obstacle collision is done via: if (not obstacle) and (not collision), do move
	// enemy collision is done via: if (not obstacle) and (not enemy) and (not collision), do move
	
	// -----
	
	// duplicate entityMap.
	// old entityMap WILL BE USED FOR COLLISION.
	
	// for enemy in enemyList
	// domove
	
	
	// go through each pos in entityMap, and determine what happens 
	// only one entity, chill 
	// >=2 entities, killall
	// >=1 obstacle, and any entities, this shouldnt happen i believe?
	
	// copy over new entityMap to old, and update all subarrays.
	// delete anything which needs to be killed
	

}

void EntityManager::updateMap() {

	// update the entity map, and sublists, and also kill enemies if needed
	
	// static to avoid realloc, cleared to prevent double use
	// or,, i could use iterators, tbh ima do that.
	// butanos vector doesnt have a remove by value, save these for delete later
	// honestly after what ive seen, i think i might as well impliment my own remove tbh
	// nvm, finally going to figure out the butano sets.
	//static bn::vector<Entity*, MAXSPRITES> killList;
	
	for(int x=0; x<14; x++) {
		for(int y=0; y<9; y++) {
			
			if(entityMap[x][y].size() == 0) {
				continue;
			}
			
			if(entityMap[x][y].size() == 1) {
				continue;
			}
			
			// remove all enemies, keep obstacles
			//for(int i=0; i<entityMap[x][y].size(); i++) {
			for(EntitySetIterator<4> it = entityMap[x][y].begin(); it != entityMap[x][y].end(); ) {
				
				Entity* temp = *it;
				
				if(temp->isPlayer()) {
					BN_ERROR("go impliment death dummy");
				} else if(temp->isObstacle()) {
					++it;
					continue;
				} else if(temp->isEnemy()) {
					// kill
					it = entityMap[x][y].erase(it);
					
					entityList.erase(temp);
					enemyList.erase(temp);					
					
					continue;
				} else {
					BN_ERROR("a entity was somehow not a player, obstacle, or enemy. wtf.");
				}
			}
		}
	}

}

