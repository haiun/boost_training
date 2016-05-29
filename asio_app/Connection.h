#pragma once
#include <boost/array.hpp>

class Connection
{
	enum { BUFFERSIZE = 512 };

public:
	void Read();
	void Send();

	void OnRead(const boost::system::error_code& error, std::size_t byte_transferred);
	void OnSend();

public:
	boost::asio::ip::tcp::socket* socket;
	boost::system::error_code error;
	boost::array<char, BUFFERSIZE> readBufer;
};
