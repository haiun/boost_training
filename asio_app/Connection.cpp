#include "stdafx.h"
#include "Connection.h"

#include <boost/bind.hpp>

void Connection::Read()
{
	socket->async_receive(boost::asio::buffer(readBufer, 512),
		0,
		boost::bind(&Connection::OnRead,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::Send()
{
	//socket->asyn
}

void Connection::OnRead(const boost::system::error_code& error, std::size_t byte_transferred)
{

}
