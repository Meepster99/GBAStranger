#pragma once 

class RoomManager {
public:

	int roomIndex = 218;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	
};