
#include "LinkManager.h"

#include "Game.h"
#include "TileManager.h"
#include "EffectsManager.h"
#include "EntityManager.h"

#include "bn_link_baud_rate.h"

/*

what needs to be sent/synced
how should multiplayer even work
should all players have to input a move, or should pppl just be able to run around 
run around ig 
this is going to be a nightmare for a feature no one will ever use 
bc,, gods moving memory around like,,,, 
ok. ideally, we fit everything into,,, one unsigned,,, using a union or something, but 
ugh i could use a bitset,,, but like 
ugh.
ill make a class

whenever a player makes a move, that player evals its gamestate, and then sends 
it over to everyone else 
who then syncs their gamestate to that sent version?
issue is, bits 
sending the whole state is easy, sending changes is hard but better 
but if i manage my data right,,, this couldnt be to hard to just send the whole state 

assuming im operating at 9600bps, thats 160 bits per frame,,, which is not good 
next level up is 38400bps, 640, which is more than enough tbh

goal: compress whole gamestate into less than 640 bytes.
send bytes when needed, yea

data needed:
room index
player type 

entityList
floor data. 

thats (curse)ing easy 
and then, we just,,,, update shit if needed during vblank.
but,,,, hmm
what if during vblank we change the inputed data for differences, and then like yea?
issue is should this be done during vblank? i dont want to waste cycles there 
we will do it in main thread

and assuming we only redraw diffs, that will be v helpful
bc we are still sending the full room state, meaning sync will occur even if player enters room midway 


how do i ensure that we,, dont send over the other players? 
probs just another class, but i planned on doing that anyway

doing this nicely is just not going to be possibe.
 
before a move, copy and backup the state
also oh my god if a player dies like,,, i dont even want to think about that 
i just thing im not going to do this
no, this whole implimentation is fucked 

like,,,, ugh i could just send the whole room state, but thats bad 
i could just send player input, but then i couldnt sync rooms 
i could do some bs where uppon entering a room it requests the room state???? 

also shit. the data rate is bits. not bytes
how abt this. 
ill have it only send player moves. (ill figure out killing entities later)
and ill s


ugh. fuck it, im just going to send ALL BOARD CHANGES.
and backup the game state before and after,,, yea

gods i just, if i do all of it and it lags like,, ugh

*/

// how did i not know about bit fields earler???

void LinkManager::sendPacket(const Packet packet) {
	
	/*
	#if BN_CFG_LINK_BAUD_RATE == BN_LINK_BAUD_RATE_9600_BPS
		const int bps = 9600;
	#elif BN_CFG_LINK_BAUD_RATE == BN_LINK_BAUD_RATE_38400_BPS    
		const int bps = 38400;
	#elif BN_CFG_LINK_BAUD_RATE == BN_LINK_BAUD_RATE_57600_BPS    
		const int bps = 57600;
	#elif BN_CFG_LINK_BAUD_RATE == BN_LINK_BAUD_RATE_115200_BPS   
		const int bps = 115200;
	#else 
		const int bps = 9600;
	#endif
	
	
	const int bytesPerFrame = (bps / 60) / 8;
	
	static bn::timer sendTimer;
	*/
	
	//bn::link::send(d);
	
	
}

static unsigned short LinkManager::recvShort() {
	
	static bn::optional<bn::link_state> linkStateOpt;
	
	
	
	
}

bn::optional<Packet> LinkManager::recvPacket() {
	linkStateOpt = bn::link::receive();
		
	if(!linkStateOpt.has_value()) {
		return;
	}
	
	bn::link_state& linkState = linkStateOpt.value();
	
	ID = linkState.current_player_id();
	playerCount = linkState.player_count();

	const bn::ivector<bn::link_player>& otherData = linkState.other_players();
	
	if(otherData.size() == 0) {
		break;
	}

	for(const auto& data : otherData) {
		allData.push_back(data);
	}
}

void LinkManager::sendState() {



	
}

void LinkManager::recvState() {
	
	// this might be v slow
	// maybe do a static alloc?
	allData.clear();
	
	while(true) {
		
		bn::optional<Packet> temp = recvPacket();
		
		if(!temp.has_value()) {
			break;
		}
			
	}
	

	
	
}

void LinkManager::backupState() {
	
	
	
	
}




