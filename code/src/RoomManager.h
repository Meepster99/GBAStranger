#pragma once 

class RoomManager {
public:

	int roomIndex = 189;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};