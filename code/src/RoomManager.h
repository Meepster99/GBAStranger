#pragma once 

class RoomManager {
public:

	int roomIndex = 289;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};