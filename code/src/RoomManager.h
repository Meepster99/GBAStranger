#pragma once 

class RoomManager {
public:

	int roomIndex = 116;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};