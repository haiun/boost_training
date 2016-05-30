#pragma once

#include <boost/lockfree/queue.hpp>
#include <list>

class Room
{
	boost::lockfree::queue<Connection*>	enterQueue;
	std::list<Connection*>				connectionList;

public:
	void Enter(Connection* connection) {}

private:
	void Leave(Connection* connection) {}
};

