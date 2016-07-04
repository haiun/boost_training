#include "stdafx.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <queue>

#include "../Packet/Packet.h"


class ServerInterface
{
public:
	virtual ~ServerInterface() {}

public:
	virtual void CloseSession(const std::size_t sessionID) = 0;
	virtual void PacketProcess(const std::size_t sessionID, PacketBase* pPacket) = 0;
};

class ServerSession
{
public:
	ServerSession(std::size_t _sessionID, boost::asio::io_service& service, ServerInterface* _pServer)
		: socketInstance(service)
		, sessionID(_sessionID)
		, pServer(_pServer)
	{

	}

	~ServerSession()
	{
	}

	void Init()
	{
		prevLeftByte = 0;
	}

	boost::asio::ip::tcp::socket& socket()
	{
		return socketInstance;
	}

	void PostRead()
	{
		socket().async_read_some(boost::asio::buffer(readBuffer),
			[this](boost::system::error_code error, std::size_t bytes_transferred) {
			if (error)
			{
				if (error == boost::asio::error::eof)
				{
					std::cout << "disconn" << std::endl;
				}
				else
				{
					std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
				}

				pServer->CloseSession(sessionID);
			}
			else
			{
				std::size_t leftByte = prevLeftByte + bytes_transferred;
				std::size_t readByte = 0;

				const int PACKET_HEAD_SIZE = sizeof(PacketBase);
				while (leftByte > 0)
				{
					if (leftByte < PACKET_HEAD_SIZE)
						break;

					PacketBase* pHeader = (PacketBase*)(&readBuffer[readByte]);
					unsigned short packetSize = pHeader->size;

					if (packetSize > leftByte)
						break;

					pServer->PacketProcess(sessionID, pHeader);
					leftByte -= packetSize;
					readByte += packetSize;
				}

				if (leftByte > 0)
				{
					//memmove_s(&readBuffer[0], leftByte, &readBuffer[readByte], leftByte);

					std::array<char, 1024>	swapBuffer;
					memcpy_s(&swapBuffer[0], 1024, &readBuffer[readByte], leftByte);
					memcpy_s(&readBuffer[0], 1024, &swapBuffer[0], leftByte);
				}

				prevLeftByte = leftByte;

				PostRead();
			}
		});
	}

	void PostSend(WriteCommand* pCommand)
	{
		boost::asio::async_write(socket(), boost::asio::buffer(pCommand->pData, pCommand->size()),
			[this, pCommand](const boost::system::error_code& error, std::size_t bytes_transferred) {
			pCommand->Release();
		});
	}

private:
	std::size_t sessionID;
	boost::asio::ip::tcp::socket socketInstance;

	std::array<char, 1024> readBuffer;
	std::size_t prevLeftByte;
	ServerInterface* pServer;
};

class TCP_Server : public ServerInterface
{
public:
	TCP_Server(boost::asio::io_service& io_service)
		: acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 33440))
		, isAccepting(false)
	{
		
	}

	~TCP_Server()
	{
		for (std::size_t i = 0; i < sessionList.size(); ++i)
		{
			ServerSession* pSession = sessionList[i];
			if (pSession->socket().is_open())
				pSession->socket().close();
			delete pSession;
		}
	}

public:
	void Init(const std::size_t sessionCount)
	{
		sessionList.reserve(sessionCount);
		for (size_t i = 0; i < sessionCount; ++i)
		{
			ServerSession* pSession = new ServerSession(i, acceptor.get_io_service(), this);
			sessionList.push_back(pSession);
			sessionQueue.push_back(i);
		}
	}

	void Start()
	{
		PostAccept();
	}

	void CloseSession(const std::size_t sessionID)
	{
		sessionList[sessionID]->socket().close();
		sessionQueue.push_back(sessionID);
	}

	void PacketProcess(const std::size_t sessionID, PacketBase* pPacket)
	{
		switch (pPacket->id)
		{
		case LOGIN:
		{
			LoginPacket* pLogin = pPacket->Cast<LoginPacket>();

			LoginPacket* send = new LoginPacket();

			sessionList[sessionID]->PostSend(new WriteCommand(send));

		}
		break;

		case CHAT:
		{
			ChatPacket* pChat = pPacket->Cast<ChatPacket>();

			std::cout << pChat->message << std::endl;

			ChatPacket* send = new ChatPacket();
			memcpy(send->message, pChat->message, 128);
			WriteCommand* pCmd = new WriteCommand(send);
			for (std::size_t i = 0; i < sessionList.size(); ++i)
			{
				if (!sessionList[i]->socket().is_open())
					continue;

				sessionList[i]->PostSend(pCmd->Clone());
			}
			pCmd->Release();
		}
		break;
		}
	}

private:
	bool PostAccept()
	{
		std::cout << "PostAccept()" << std::endl;

		if (sessionQueue.empty())
		{
			isAccepting = false;
			return false;
		}

		isAccepting = true;
		std::size_t sessionID = sessionQueue.front();
		sessionQueue.pop_front();

		ServerSession* pSession = sessionList[sessionID];
		acceptor.async_accept(pSession->socket(),
			[this, pSession](const boost::system::error_code& error) {
			if (!error)
			{
				std::cout << "handle_accept" << std::endl;

				pSession->Init();
				pSession->PostRead();

				PostAccept();
			}
			else
			{
				std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
			}
		});

		return true;
	}

private:
	boost::asio::ip::tcp::acceptor acceptor;
	bool isAccepting;
	std::vector<ServerSession*>	sessionList;
	std::deque<std::size_t> sessionQueue;
};

void main()
{
	boost::asio::io_service service;
	TCP_Server server(service);
	server.Init(10);
	server.Start();

	boost::thread thread1(boost::bind(&boost::asio::io_service::run, &service));
	boost::thread thread2(boost::bind(&boost::asio::io_service::run, &service));
	boost::thread thread3(boost::bind(&boost::asio::io_service::run, &service));
	boost::thread thread4(boost::bind(&boost::asio::io_service::run, &service));

	getchar();
}
