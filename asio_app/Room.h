#pragma once

#include "Connection.h"

//Room에 존제하지 않는 Connection은 존재하지 않는다??
//Server에 선언된 Room Lobby는 한 물리적 서버의 모든 Connection을 들고 있으며, 닫힌 Connection을 검출한다??

class Room
{
	boost::lockfree::queue<RoomConnection*>	enterQueue;
	std::list<RoomConnection*>				connectionList;

public:
	void Enter(RoomConnection* connection)
	{
		connection->refRoom++;
		enterQueue.push(connection);
	}

	void Leave(RoomConnection* connection)
	{
		connectionList.remove(connection);
		connection->refRoom--;
	}
};

