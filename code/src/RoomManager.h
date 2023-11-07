#pragma once 

struct MessageStrJank;

class RoomManager {
public:

	RoomManager();
	
	bool isCustom = false;

	int roomIndex = 0;
	int modeTracker = 0;
	
	const Room* roomArray = NULL;
	const MessageStrJank* roomNameArray = NULL;
	
	Room loadRoom();
	
	const char* currentRoomName();
	unsigned currentRoomHash();
	
	void nextRoom();
	void prevRoom();
	void cifReset();
	void changeFloor(int val);
	void setMode(int mode);
	
	void gotoRoom(const char* roomName);
	void gotoRoom(int newIndex);
		
	void isCustomRooms();
	void initCustomRooms();
	Room loadCustomRoom();
	
	~RoomManager();

private:
	// dont have to use a vector here. i worry bc like, running out of space. i can use an unsigned short tho bc, 32kb = 2^15
	// gods but regardless, it just burns 2k of stack mem, when i dont fucking need it. im allocating
	
	unsigned roomCountAlloc = 0;
	unsigned short* roomOffsets = NULL;
	char** roomNamesAlloc = NULL;
	
};