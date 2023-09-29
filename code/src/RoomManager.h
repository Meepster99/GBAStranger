#pragma once 

class RoomManager {
public:

	int roomIndex = 45;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};