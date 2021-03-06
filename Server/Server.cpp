#include "stdafx.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <queue>

#include "../Packet/Packet.h"


class Room
{
public:
	boost::asio::strand strand;
	std::vector<std::string> log;

public:
	void Call(WriteCommand* pCmd)
	{
		
		pCmd->Release();
	}
};

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
		, strand(io_service)
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

	void Broadcast(WriteCommand* pCmd)
	{
		std::for_each(sessionList.begin(), sessionList.end(), [pCmd](ServerSession* pSession) {
			if (!pSession->socket().is_open())
				return;

			pSession->PostSend(pCmd->Clone());
		});
	}

	void PacketProcess(const std::size_t sessionID, PacketBase* pPacket)
	{
		switch (pPacket->id)
		{
		case LOGIN:
		{
			{
				LoginPacket* pLogin = pPacket->Cast<LoginPacket>();

				LoginPacket* send = new LoginPacket();
				send->sessionID = sessionID;

				sessionList[sessionID]->PostSend(WriteCommand::Create(send));
			}

			{
				MovePacket* pMove = new MovePacket();
				pMove->sessionID = sessionID;
				pMove->position[0] = 0;
				pMove->position[1] = 0;
				pMove->velocity[0] = 0;
				pMove->velocity[1] = 0;

				WriteCommand* pCmd = WriteCommand::Create(pMove);
				Broadcast(pCmd);
				pCmd->Release();
			}
			break;
		}

		case CHAT:
		{
			ChatPacket* pChat = pPacket->Cast<ChatPacket>();

			ChatPacket* send = new ChatPacket();
			memcpy(send->message, pChat->message, 128);
			WriteCommand* pCmd = WriteCommand::Create(send);
			Broadcast(pCmd);

			pCmd->Clone();

			strand.dispatch([this, pCmd]()
			{
				auto data = pCmd->pData->Cast<ChatPacket>();

				for (int i = 0; i < 10; ++i)
				{
					printf(data->message);
					Sleep(10);
					printf("\n");
				}

				Sleep(10);

				pCmd->Release();
			});

			pCmd->Release();
			break;
		}

		case MOVE:
		{
			MovePacket* pMove = pPacket->Cast<MovePacket>();

			MovePacket* send = new MovePacket();
			send->sessionID = pMove->sessionID;
			send->position[0] = pMove->position[0];
			send->position[1] = pMove->position[1];
			send->velocity[0] = pMove->velocity[0];
			send->velocity[1] = pMove->velocity[1];
			WriteCommand* pCmd = WriteCommand::Create(send);
			Broadcast(pCmd);

			pCmd->Clone();
			strand.get_io_service().dispatch(

			strand.wrap([this, pCmd]()
			{
				printf("MOVE");
				printf("MOVE");
				printf("MOVE\n");
			})

			);

			pCmd->Release();
			break;
		}
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
	boost::asio::strand strand;
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
