// asio_app.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <vector>

int main()
{
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket socket(io_service);
	boost::system::error_code error;

	auto address = boost::asio::ip::address::from_string("127.0.0.1");
	auto endPoint = boost::asio::ip::tcp::endpoint(address, 33440);

	socket.connect(endPoint, error);
	if (error)
	{
		std::cout << error.message() << std::endl;
		return 0;
	}



    return 0;
}

