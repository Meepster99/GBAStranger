#pragma once 

class RoomManager {
public:

	int roomIndex = 226;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};