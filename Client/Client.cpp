#include "stdafx.h"
#include <boost/asio.hpp>
#include <iostream>

int main()
{
	boost::asio::io_service service;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 33440);
	boost::system::error_code connect_error;
	boost::asio::ip::tcp::socket socket(service);
	
	socket.connect(endpoint, connect_error);
	if (connect_error)
	{
		std::cout << "cannot connect. error : " << connect_error.value() << " message : " << connect_error.message() << std::endl;
		return 0;
	}
	else
	{
		std::cout << "connected" << std::endl;
	}

	for (int i = 0; i < 7; ++i)
	{
		char message[128] = { 0, };
		sprintf_s(message, 127, "%d - send", i);
		size_t messageLen = strnlen_s(message, 127);

		boost::system::error_code ignored_error;
		socket.write_some(boost::asio::buffer(message, messageLen), ignored_error);

		std::array<char, 128> buf;
		buf.assign(0);
		boost::system::error_code error;
		size_t len = socket.read_some(boost::asio::buffer(buf), error);

		if (error)
		{
			if (error == boost::asio::error::eof)
			{
				std::cout << "disconn" << std::endl;
			}
			else
			{
				std::cout << "error" << std::endl;
			}
			break;
		}

		std::cout << "server:" << &buf[0] << std::endl;
	}

	if (socket.is_open())
	{
		socket.close();
	}

	getchar();

    return 0;
}

