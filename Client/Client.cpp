#include "stdafx.h"
#include "../Packet/TCP_Client.h"

int main()
{
	boost::asio::io_service service;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 33440);
	
	TCP_Client	client(service);
	client.Connect(endpoint);

	boost::thread thread(boost::bind(&boost::asio::io_service::run, &service));

	char inputBuffer[256] = { 0, };
	std::cin >> inputBuffer;
	LoginPacket* packet = new LoginPacket();
	client.PostWrite(false, packet->size, new WriteCommand(packet));

	while (std::cin >> inputBuffer)
	{

	}

	/*
	int i = 0;
	while (true)
	{
		++i;
		if (strnlen_s(inputBuffer, 256) == 0)
		{
			break;
		}

		if (!client.IsOpen())
		{
			std::cout << "서버와 연결되지 않았습니다" << std::endl;
			continue;
		}
		
		ChatPacket* packet = new ChatPacket();
		sprintf_s(packet->message, "%s %d", inputBuffer, i);
		client.PostWrite(false, packet->size, new WriteCommand(packet));

		Sleep(100);
	}
	*/

	service.stop();

	client.Close();

	thread.join();

    return 0;
}

