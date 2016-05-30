#pragma once

class Connection;
class Room;

class Server
{
private:
	boost::asio::io_service* service;
	boost::asio::ip::tcp::acceptor* acceptor;
	boost::system::error_code error;
	
	Connection* connection;
	Room* lobby;

public:
	void Init();
	void Destroy();

	void Accept();

	void OnAccept(const boost::system::error_code& error);
};