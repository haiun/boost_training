// asio_app.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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

