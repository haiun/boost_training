#pragma once
#include <boost/array.hpp>

class Connection
{
	enum { BUFFERSIZE = 512 };

public:
	Connection(boost::asio::io_service& service);

public:
	void Read();
	void Send();

	void OnRead(const boost::system::error_code& error, std::size_t byte_transferred);
	void OnSend(const boost::system::error_code& error, std::size_t byte_transferred);

public:
	boost::asio::ip::tcp::socket socket;
	boost::system::error_code error;
	boost::array<char, BUFFERSIZE> readBufer;
	boost::array<char, BUFFERSIZE> writeBufer;
};
