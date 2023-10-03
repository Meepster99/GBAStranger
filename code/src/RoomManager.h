#pragma once 

class RoomManager {
public:

	int roomIndex = 7;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};