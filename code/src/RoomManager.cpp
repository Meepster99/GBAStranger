

#include "RoomManager.h"
#include "AllRooms.h"
#include "Game.h"



// for checking the actual dump of the roomdata,
// C:\devkitPro\devkitARM\arm-none-eabi\bin\objdump.exe -S -D .\build\RoomManager.o > idk.txt
// more work needs to be done on the room exporter, and compressing it

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

unsigned RoomManager::currentRoomHash() {
	if(roomIndex < 0 || roomIndex > MAXROOMS - 1) {
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}

	const char* temp = roomNameArray[roomIndex].str;

	// goofy
	if(WTF(temp) >= 5) {
		if(temp[0] == 'h' &&
			temp[1] == 'a' &&
			temp[2] == 'r' &&
			temp[3] == 'd' &&
			temp[4] == '_') {
				temp += 5;
		}
	}

	return hashString(temp);
}

void RoomManager::nextRoom() {
	if(roomIndex + 1 < MAXROOMS - 1) {
		roomIndex++;
		if(currentRoomHash() == hashString("rm_rm4\0")) {
			roomIndex--;
		}
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
	int backup = roomIndex;

	roomIndex = MIN(roomIndex + val, MAXROOMS - 1);
	roomIndex = MAX(roomIndex, 0);

	if(currentRoomHash() == hashString("rm_rm4\0")) {
		roomIndex = backup;
	}

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

	BN_LOG("actual search string is ", buffer);

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

bool RoomManager::isWhiteRooms() {

	static int prevRoomIndex = -1;
	static bool prevRes = false;

	if(roomIndex == prevRoomIndex) {
		return prevRes;
	}

	prevRoomIndex = roomIndex;

	if((strstrCustom(currentRoomName(), "_u_00\0") == NULL) &&
		(strstrCustom(currentRoomName(), "_u_en\0") == NULL)) {
		prevRes = false;
		return false;
	}

	prevRes = true;
	return true;
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

	if(firstVal == 42) {
		isCustom = true;
		initCustomRooms();
	}
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

	u8 burdenStates = readByte();
	//u8 burdenStates = 0;
	globalGame->saveData.hasMemory   = !!(burdenStates & 0b0001);
	globalGame->saveData.hasWings    = !!(burdenStates & 0b0010);
	globalGame->saveData.hasSword    = !!(burdenStates & 0b0100);
	globalGame->saveData.hasSuperRod = !!(burdenStates & 0b1000);

	globalGame->saveData.hasRod = true;

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
	unsigned short detailsOffset = readShort();
	unsigned short floorOffset = readShort();
	unsigned short entityCount = readShort();
	unsigned short entitiesOffset = readShort();

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
