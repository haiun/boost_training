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

	io_service.run();


    return 0;
}

