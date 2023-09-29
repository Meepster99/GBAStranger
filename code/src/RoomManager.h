#pragma once 

class RoomManager {
public:

	int roomIndex = 49;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};