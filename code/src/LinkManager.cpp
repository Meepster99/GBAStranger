
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




this is not easily feasable with my limited expirence.
unless i do this, the easy way, with lag, and possibly disable sound as a result.

how do i know what entity to move where??
i could give each entity an id?
or have past and current positions, but tracking that is a full byte 
and i could also,, just send the move dir????
im to tired and burnt out for this

*/

// how did i not know about bit fields earler???

void LinkManager::sendPacket(const Packet packet) {
	bn::link::send(packet.upperData);
	bn::link::send(packet.lowerData);
}

ShortReturn LinkManager::recvShort() {
	
	static bn::optional<bn::link_state> linkStateOpt;
	
	linkStateOpt = bn::link::receive();
	
	ShortReturn res;
	memset(res.data, 0xFFFF, 4 * sizeof(unsigned short));
	
	if(!linkStateOpt.has_value()) {
		return res;
	}
	
	bn::link_state& linkState = linkStateOpt.value();
	
	ID = linkState.current_player_id();
	playerCount = linkState.player_count();

	const bn::ivector<bn::link_player>& otherData = linkState.other_players();
	
	for(const auto& data : otherData) {
		res.data[data.id()] = data.data();
	}

	return res;
}

bool LinkManager::recvPacket() {
	
	ShortReturn res1 = recvShort();
	ShortReturn res2 = recvShort();
	
	unsigned successCount = 0;
	for(int i=0; i<4; i++) {
		if(res1.data[i] == 0xFFFF || res2.data[i] == 0xFFFF) {
			BN_ASSERT(res1.data[i] == 0xFFFF && res2.data[i] == 0xFFFF, "recvshort data was messed up");
			continue;
		}
		
		successCount++;
		
		Packet tempPacket;
		tempPacket.upperData = res1.data[i];
		tempPacket.lowerData = res2.data[i];
		
		BN_ASSERT(allPackets[i].size() + 1 != allPackets[i].max_size(), "recv packet buffer overflow");
		
		allPackets[i].push_back(tempPacket);		
	}
	
	return successCount != 0;
}

void LinkManager::sendState() {

	Packet temp;
	
	if(entityManager->playerStart == entityManager->player->p) {
		return;
	}
	
	temp.prevX = entityManager->playerStart.x; 
	temp.prevY = entityManager->playerStart.y; 
	
	temp.x = entityManager->player->p.x;
	temp.y = entityManager->player->p.y;
	temp.entityType = entityManager->player->entityType();
	temp.packetType = PacketType::Entity;
	
	sendPacket(temp);
	
}

void LinkManager::recvState() {

	allPackets[0].clear();
	allPackets[1].clear();
	allPackets[2].clear();
	allPackets[3].clear();
	
	while(recvPacket()) { }
	
	for(int i=0; i<4; i++) {
		for(const auto& packet : allPackets[i]) {
			if(packet.packetType == PacketType::Entity) {
				
				//switch
				
				Pos startPos = Pos(packet.prevX, packet.prevY);
				Pos endPos = Pos(packet.x, packet.y);
				
				BN_LOG(startPos, " --> ", endPos);
				
				SaneSet<Entity*, 4>& temp = entityManager->getMap(startPos);
				
				//Entity* tempEntity = NULL;
				
				if(temp.size() == 0 && packet.entityType == EntityType::Player) {
					OtherPlayer* e = new OtherPlayer(startPos, i);
					
					BN_ASSERT(entityManager->playerList.size() + 1 != entityManager->playerList.max_size(), "otherplayer buffer overflow, tempsize = ", temp.size());
					entityManager->playerList.push_back(e);
					entityManager->entityList.insert(e);
					entityManager->obstacleList.insert(e);	
					
					entityManager->entityMap[e->p.x][e->p.y].insert(e);
					entityManager->futureEntityMap[e->p.x][e->p.y].insert(e);
				}
					
				entityManager->moveEntityToPos(startPos, endPos);
				
			} else if(packet.packetType == PacketType::Tile) {
				
				
				
				
				
				
				
			} else {
				BN_ERROR("unknown packet type");
			}
		}
	}
	
	
	entityManager->updateScreen();

}





