#pragma once 

struct MessageStrJank;

class RoomManager {
public:

	RoomManager();

	int roomIndex = 0;
	
	const Room* roomArray = NULL;
	const MessageStrJank* roomNameArray = NULL;
	
	Room loadRoom();
	
	const char* currentRoomName();

	void nextRoom();
	void prevRoom();
	void cifReset();
	void changeFloor(int val);
	void setMode(int mode);
};