#pragma once 

class RoomManager {
public:

	int roomIndex = 150;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};