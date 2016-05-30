#include "stdafx.h"
#include "Connection.h"

#include <boost/bind.hpp>

Connection::Connection(boost::asio::io_service& service)
	: socket(service)
{

}

void Connection::Read()
{
	socket.async_receive(boost::asio::buffer(readBufer, BUFFERSIZE),
		0,
		boost::bind(&Connection::OnRead,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::Send()
{
	socket.async_send(boost::asio::buffer(writeBufer, BUFFERSIZE),
		0,
		boost::bind(&Connection::OnSend,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::OnRead(const boost::system::error_code& error, std::size_t byte_transferred)
{

}

void Connection::OnSend(const boost::system::error_code& error, std::size_t byte_transferred)
{

}
