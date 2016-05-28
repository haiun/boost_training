#include "stdafx.h"
#include "Client.h"
#include "Connection.h"
#include <boost/bind.hpp>

void Client::Init()
{
	auto address = boost::asio::ip::address::from_string("127.0.0.1");
	auto endPoint = boost::asio::ip::tcp::endpoint(address, 12300);

	service = new boost::asio::io_service();

	pConnection = new Connection();
	pConnection->socket = new boost::asio::ip::tcp::socket(*service);

	pConnection->socket->async_connect(endPoint, boost::bind(&Client::OnConnect, this, boost::asio::placeholders::error));

	service->run();
}

void Client::Destroy()
{

}

void Client::OnConnect(const boost::system::error_code& error)
{
	if (!error)
	{
		std::cout << "Connected" << std::endl;
	}
}