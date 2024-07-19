#pragma once

#include "SharedTypes.h"

struct MessageStrJank;

namespace RoomManager {


	// dont have to use a vector here. i worry bc like, running out of space. i can use an unsigned short tho bc, 32kb = 2^15
	// gods but regardless, it just burns 2k of stack mem, when i dont (curse)ing need it. im allocating

	extern unsigned roomCountAlloc;
	extern unsigned short* roomOffsets;
	extern char** roomNamesAlloc;
	extern bool isCustom;
	extern int roomIndex;
	extern int modeTracker;
	extern const Room* roomArray;
	extern const MessageStrJank* roomNameArray;

	void RoomManager();

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

	bool isWhiteRooms();

};
