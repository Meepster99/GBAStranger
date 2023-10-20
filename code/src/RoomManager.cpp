

#include "SharedTypes.h"

#include "AllRooms.h"
#include "RoomManager.h"


RoomManager::RoomManager() {
	setMode(0);
}

Room RoomManager::loadRoom() {
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {
		BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}

	return roomArray[roomIndex];
}

const char* RoomManager::currentRoomName() { 
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}
	
	return roomNameArray[roomIndex].str; 
}

void RoomManager::nextRoom() {
	if(roomIndex + 1 < (int)(sizeof(rooms)/sizeof(rooms[0]))) {
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
	roomIndex = MIN(roomIndex + val, (int)(sizeof(rooms)/sizeof(rooms[0])) - 1);
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
	} else {
		BN_ERROR("unknown room mode set, val= ", mode);
	}
	
}

