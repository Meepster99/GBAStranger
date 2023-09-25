

#include "SharedTypes.h"

#include "AllRooms.h"
#include "RoomManager.h"

Room RoomManager::loadRoom() {
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {
		BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}

	return rooms[roomIndex];
}

const char* RoomManager::currentRoomName() { 
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}
	
	return roomNames[roomIndex]; 
}
