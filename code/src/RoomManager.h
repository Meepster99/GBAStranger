#pragma once 

class RoomManager {
public:

	int roomIndex = 139;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};