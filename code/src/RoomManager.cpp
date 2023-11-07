

#include "SharedTypes.h"

#include "AllRooms.h"
#include "RoomManager.h"

RoomManager::RoomManager() {
	setMode(0);
}

Room RoomManager::loadRoom() {
	
	if(isCustom) {
		return loadCustomRoom();
	}
	
	
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {
		BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}

	return roomArray[roomIndex];
}

const char* RoomManager::currentRoomName() { 
	if(roomIndex < 0 || roomIndex > MAXROOMS - 1) {
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}
	
	return roomNameArray[roomIndex].str; 
}

void RoomManager::nextRoom() {
	if(roomIndex + 1 < MAXROOMS - 2) {
		roomIndex++;
		BN_LOG("roomIndex incremented to ", roomIndex);
	}
}

void RoomManager::prevRoom() {
	if(roomIndex - 1 >= 0 ) {
		roomIndex--;
		BN_LOG("roomIndex decremented to ", roomIndex);
	}
}

void RoomManager::cifReset() {
	roomIndex = 1;
	BN_LOG("cifReset called! roomIndex set to ", roomIndex);
}

void RoomManager::changeFloor(int val) {
	roomIndex = MIN(roomIndex + val, MAXROOMS - 2);
	roomIndex = MAX(roomIndex, 0);
	BN_LOG("roomIndex changed to ", roomIndex);
}

void RoomManager::setMode(int mode) {
	
	if(mode == 0) {
		roomArray = &rooms[0];
		roomNameArray = roomNames;
	} else if(mode == 1) {
		roomArray = &hard_rooms[0];
		roomNameArray = hard_roomNames;
	} else if(mode == 2) {
		// i rlly need to impliment this
		roomArray = &rooms[0];
		roomNameArray = roomNames;
	} else {
		BN_ERROR("unknown room mode set, val= ", mode);
	}
	
	modeTracker = mode;
	
}

void RoomManager::gotoRoom(const char* roomName) {
	
	if(roomName == NULL) {
		BN_LOG("attempting to go to NULL, meaning nextroom");
	} else {
		BN_LOG("attempting to go to ", roomName);
	}
	
	if(roomName == NULL) {
		nextRoom();
		return;
	}
	
	
	
	
	char buffer[32];
	char* roomNameBuffer = buffer;
	
	// if we are in hard mode we want to search for a hard mode room thingy.
	if(modeTracker == 1) {
		strcpy(roomNameBuffer, "hard_");
		roomNameBuffer += WTF(roomNameBuffer);
	}
	
	strcpy(roomNameBuffer, roomName);
	
	
	
	// shit time complexity, but im tired.
	for(int i=0; i<(int)(sizeof(rooms)/sizeof(rooms[0])); i++) {
		
		if(WTF(buffer) != WTF(roomNameArray[i].str)) {
			continue;
		}
		
		if(strcmp(buffer, roomNameArray[i].str) == 0) {
			
			BN_LOG("found match ", roomNameArray[i].str);
			
			roomIndex = i;
			return;
		}
	}
	
	BN_ERROR("in gotoRoom, we were unable to goto the room, ", roomName, "searchstring was ", buffer);
}

void RoomManager::gotoRoom(int newIndex) {
	newIndex = MIN(newIndex, MAXROOMS-1);
	newIndex = MAX(newIndex, 0);
	roomIndex = newIndex;
	BN_LOG("roomIndex gotoed to ", roomIndex);
}

// -----

RoomManager::~RoomManager() {
	
	if(roomOffsets != NULL) {
		// i despise how the delete has two options, for/not arrays.
		delete[] roomOffsets;
		roomOffsets = NULL;
	}
	
	if(roomNamesAlloc != NULL) {
		for(unsigned i = 0; i < roomCountAlloc; ++i) {
			delete[] roomNamesAlloc[i];   
			roomNamesAlloc[i] = NULL;
		}
		delete[] roomNamesAlloc;
	}
	
	
}

void RoomManager::isCustomRooms() {
	
	u8* sram = reinterpret_cast<u8*>(0x0E000000);
	int sramIndex = 0; 
	
	auto readUnsigned = [sram, &sramIndex]() mutable -> unsigned {
		unsigned res = 0;
		for(int i=0; i<4; i++) {
			res = (res << 8) | sram[sramIndex++];
		}
		return res;		 
	};
	

	unsigned firstVal = readUnsigned();
	
	//BN_LOG(firstVal);
	
	/*
	EntityHolder test = {static_cast<EntityType>(readUnsigned()/10), 2, 3};
	
	BN_LOG(test.t);
	BN_LOG(sizeof(test));
	BN_LOG(sizeof(test.t));
	BN_LOG(   *((unsigned*)static_cast<void*>(&test)));
	*/
	
	if(firstVal == 42) {
		isCustom = true;
		initCustomRooms();
	}
	
	//BN_LOG(rm_0001::collision);
	//BN_LOG(rm_0001::entities);
	
	
}

void RoomManager::initCustomRooms() {
	
	
	
	// u8 might be hella slow, but like, ughg idek
	u8* sram = reinterpret_cast<u8*>(0x0E000000);
	int sramIndex = 0; 
	
	// i pray these funcs are inlined, check comp explorer!
	auto readByte = [sram, &sramIndex]() mutable -> u8 {
		return sram[sramIndex++];
	};
	
	auto readShort = [sram, &sramIndex]() mutable -> unsigned short {
		unsigned short res = 0;
		for(int i=0; i<2; i++) {
			res = (res << 8) | sram[sramIndex++];
		}
		return res;		
	};
	
	auto readUnsigned = [sram, &sramIndex]() mutable -> unsigned {
		unsigned res = 0;
		for(int i=0; i<4; i++) {
			res = (res << 8) | sram[sramIndex++];
		}
		return res;		 
	};
	
	(void)readByte;
	(void)readShort;
	(void)readUnsigned;
	
	// skip the first 42
	unsigned shouldBe42 = readUnsigned();
	
	BN_ASSERT(shouldBe42 == 42, "when loading custom rooms, the first unsigned of the savefile wasnt 42. wtf");
	
	roomCountAlloc = readUnsigned();
	
	roomOffsets = new unsigned short[roomCountAlloc];
	
	for(unsigned i = 0; i<roomCountAlloc; i++) {
		roomOffsets[i] = readShort();
	}
	
	char buffer[256];
	int bufferIndex = 0;
	
	roomNamesAlloc = new char*[roomCountAlloc];
	
	for(unsigned i = 0; i<roomCountAlloc; i++) {
		
		memset(buffer, 0, 256);
		
		u8 res = readByte();
		
		buffer[bufferIndex++] = res;		
		
		if(res == '\0') {
			roomNamesAlloc[i] = new char[bufferIndex];
			strcpy(roomNamesAlloc[i], buffer);
		}		
	}
	
}

constexpr static inline EffectHolder defaultEffects[] = {
{&bn::sprite_tiles_items::dw_spr_statue_abaddon,-1,-1,-1,-1},
};
constexpr static inline int defaultEffectsCount = 1;
constexpr static inline SecretHolder defaultSecrets[] = {
{-1,-1,NULL},
};
constexpr static inline int defaultSecretsCount = 1;
constexpr static inline const char* defaultExitDest = NULL;
constexpr static inline const bn::regular_bg_tiles_item* defaultCollisionTiles = &bn::regular_bg_tiles_items::dw_tile_bg_1;
constexpr static inline const bn::regular_bg_tiles_item* defaultDetailsTiles = &bn::regular_bg_tiles_items::dw_tile_edges;

Room RoomManager::loadCustomRoom() {
	
	// could use a bool, but i dont want to risk junk sram data.
	// 42 will be a custom room
	// how the FUCK am i going to do this if i want to use compression?
	// alternatively, one could include ~78 rooms  (conservative estimate)
	// without compression. so fuck it, we ball.
	//int isCustomSave = 42;
	
	
	//

	// nvm, doing this the hard way.
	// nvm we are doing this the even harder way.
	// i wont have enough time to load a thing into the stack into the first place, bc loading 32kb into stack is dumb.
	// so this will be an alloc, and ill use pointer fuckery to load it in. 
	// actually, i dont even need pointer fuckery, ill just read directly from srams
	
	// we will do a stream based approach, as in first(well second, first unsigned is 42), second is room count, and then afterword we load all rooms 
	// that is going to suck 
	// additionally, not supporting anything like,,, dialogue, or etc
	// going to use lambdas for specific like,,, reading of unsigneds
	
	// reading everything ahead of time would be,, shit 
	// should i then therefore,, have this be a custom reader? meaning it should be in room manager? yes
	// and also, i should have a table with offsets to where each individual rooms starts? ok 
	// actually,,,,, if i wanna get REALLY weird with it 
	// i can have an offset to collision, offset to details, offset to floor, and then entityies, and an unsigned at the start for entity count, and just set those jank 
	// ass pointers as like,,,, the actual things.
	// only concern will be,,, the entityholder struct(what size is EntityType? are the u8s actually like, being stored as such?
	// struct allignment has a large chance to fuck me here
	
	/*
	
	Save format:
	unsigned, for room count 
	a table, length of roomcount, with offsets to the start of each roomindex.
	an array of strings, length of roomcount, for all room names.
	
	For each room:
	compressed collision data // have the decomp algo be put in here, and have it inc sram.
	compressed details data 
	compressed floor data
	
	unsigned, for entityCount 
	
	and then, 3 bytes per entity(this can be reduced!!!) for entityType, x, y 
	// going with more than 3bytes per ent 
	EntityHolder struct is 8 bytes, 4 for type, 2 for each pos. 
	i could reduce it, but im tired ok.
	
	each room offset will now hold:
	entityCount(short)
	offset to start details(short)
	offset to start floor (short)
	offset to start entities (short)
	(collision offset will implicitly be right here, roomOffset+8)
	and then, we just set those pointers to the room object, and we are CHILLIN
	
	
	
	
	
	*/
	
	BN_ASSERT(isCustom, "tried loading a custom room when the savefile didnt have any?");
	
	
	// u8 might be hella slow, but like, ughg idek
	u8* sram = reinterpret_cast<u8*>(0x0E000000);
	int sramIndex = 0; 
	
	// i pray these funcs are inlined, check comp explorer!
	auto readByte = [sram, &sramIndex]() mutable -> u8 {
		return sram[sramIndex++];
	};
	
	auto readShort = [sram, &sramIndex]() mutable -> unsigned short {
		unsigned short res = 0;
		for(int i=0; i<2; i++) {
			res = (res << 8) | sram[sramIndex++];
		}
		return res;		
	};
	
	auto readUnsigned = [sram, &sramIndex]() mutable -> unsigned {
		unsigned res = 0;
		for(int i=0; i<4; i++) {
			res = (res << 8) | sram[sramIndex++];
		}
		return res;		 
	};
	
	(void)readByte;
	(void)readShort;
	(void)readUnsigned;
	
	sramIndex = (int)roomOffsets[roomIndex];
	
	unsigned short collisionOffset = sramIndex + 8;
	unsigned short entityCount = readShort();
	unsigned short detailsOffset = readShort();
	unsigned short floorOffset = readShort();
	unsigned short entitiesOffset = readShort();
	
	//BN_LOG("entityoffset = ", entitiesOffset);
	
	/*
	constexpr Room(
	const void* collision_, const void* floor_, const void* details_, 
	const void* entities_, const int entityCount_, 
	const void* effects_, const int effectsCount_,
	const void* secrets_, const int secretsCount_,
	const void* exitDest_,
	const void* collisionTiles_, const void* detailsTiles_
	)
	*/
	//sramIndex = entitiesOffset;
	//unsigned test = readUnsigned();
	//BN_LOG(test);
	
	// what the fuck. what the FUCK,,, what the,,,fuck
	Room res(
	reinterpret_cast<void*>(0x0E000000 + collisionOffset),
	reinterpret_cast<void*>(0x0E000000 + floorOffset),
	reinterpret_cast<void*>(0x0E000000 + detailsOffset),
	reinterpret_cast<void*>(0x0E000000 + entitiesOffset),
	entityCount,
	&defaultEffects,
	defaultEffectsCount,
	&defaultSecrets,
	defaultSecretsCount,
	defaultExitDest,
	defaultCollisionTiles,
	defaultDetailsTiles
	);
	
	BN_LOG("room res entitypointyer = ", res.entities);
	
	return res;

}

