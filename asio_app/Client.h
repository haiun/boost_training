#pragma once

class Connection;

class Client
{
private:
	boost::asio::io_service* service;
	Connection* pConnection;

public:
	void Init();
	void Destroy();

	void OnConnect(const boost::system::error_code& error);
};