#pragma once

#include "Connection.h"

//Room�� �������� �ʴ� Connection�� �������� �ʴ´�??
//Server�� ����� Room Lobby�� �� ������ ������ ��� Connection�� ��� ������, ���� Connection�� �����Ѵ�??

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

