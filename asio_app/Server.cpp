#include "stdafx.h"
#include "Server.h"
#include "Connection.h"

#include <boost/bind.hpp>

void Server::Init()
{
	auto address = boost::asio::ip::address::from_string("127.0.0.1");
	auto endPoint = boost::asio::ip::tcp::endpoint(address, 12300);

	service = new boost::asio::io_service();
	acceptor = new boost::asio::ip::tcp::acceptor(*service, endPoint);

	Accept();

	service->run();
}

void Server::Destroy()
{
	
}

void Server::Accept()
{
	connection = new Connection(acceptor->get_io_service());

	//start accept
	acceptor->async_accept(connection->socket,
		boost::bind(&Server::OnAccept,
			this,
			boost::asio::placeholders::error));
}

void Server::OnAccept(const boost::system::error_code& error)
{
	if (!error)
	{
		std::cout << "Accept" << std::endl;
	}

	Accept();
}