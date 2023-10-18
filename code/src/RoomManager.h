#pragma once 

class RoomManager {
public:

	int roomIndex = 0;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	void cifReset();
};