// asio_app.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "Server.h"
#include "Client.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>

int main()
{
	Server server;
	Client client;

	boost::thread th1 = boost::thread(boost::bind(&Server::Init, &server));
	boost::thread th2 = boost::thread(boost::bind(&Client::Init, &client));

	th1.join();
	std::cout << "server.Init()" << std::endl;

	Sleep(1000);

	th2.join();
	std::cout << "client.Init()" << std::endl;

	Sleep(5000);

	client.Destroy();
	std::cout << "client.Destroy()" << std::endl;

	Sleep(1000);

	server.Destroy();
	std::cout << "server.Destroy()" << std::endl;

    return 0;
}

