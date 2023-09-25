#pragma once 

class RoomManager {
public:

	int roomIndex = 164;
	
	Room loadRoom();
	
	const char* currentRoomName();

};