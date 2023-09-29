#pragma once 

class RoomManager {
public:

	int roomIndex = 135;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};