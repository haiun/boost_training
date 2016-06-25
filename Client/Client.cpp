#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <queue>

enum PacketEnum
{
	NONE,
	LOGIN,
	CHAT,
	END
};

#pragma pack(push, 1)
struct PacketBase
{
	unsigned short id;
	unsigned short size;

	PacketBase(unsigned short _id, unsigned short _size) : id(_id), size(_size) {}
	~PacketBase() {}

	template< typename T >
	T* Cast() { return reinterpret_cast<T*>(this); }
};

struct LoginPacket : public PacketBase
{
	LoginPacket() : PacketBase(LOGIN, sizeof(LoginPacket)) {}
};

struct ChatPacket : public PacketBase
{
	char message[128];

	ChatPacket() : PacketBase(CHAT, sizeof(ChatPacket)) {}
};
#pragma pack(pop, 1)


class Command
{
public:
	Command(void* _pData, std::size_t _size) : pData(_pData), size(_size) {}
	~Command() {}

public:
	Command* Clone()
	{
		return new Command(pData, size);
	}

public:
	void* pData;
	std::size_t size;
};

class WriteCommand
{
public:
	WriteCommand(PacketBase* packet) : pData(packet), size(packet->size) {}
	~WriteCommand() { delete pData; }

public:
	PacketBase* pData;
	std::size_t size;
};

class TCP_Client
{
public:
	TCP_Client(boost::asio::io_service& io_service)
		: service(io_service)
		, socketInstance(io_service)
		, activate(false)
	{

	}

	~TCP_Client()
	{
		boost::lock_guard<boost::recursive_mutex>	lock(mtx);

		while (!writeQueue.empty())
		{
			WriteCommand* pWriteCommand = writeQueue.front();
			writeQueue.pop();
			delete pWriteCommand;
		}
	}

public:
	boost::asio::ip::tcp::socket& socket() { return socketInstance; }
	bool IsOpen() { return socket().is_open(); }
	void Activate() { activate = true; }
	bool IsActivate() { return activate; }

	void Connect(boost::asio::ip::tcp::endpoint endPoint)
	{
		prevLeftByte = 0;
		socket().async_connect(endPoint, [this](const boost::system::error_code& error)
		{
			if (!error)
			{
				std::cout << "handle_connect" << std::endl;
				PostRead();
			}
			else
			{
				std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
			}
		});
	}

	void Close()
	{
		if (socket().is_open())
		{
			socket().close();
		}
	}

	void PostWrite(const bool immediately, const std::size_t size, WriteCommand* pCommand)
	{
		WriteCommand* pCurrentCommand = nullptr;
		{
			boost::lock_guard<boost::recursive_mutex> lock(mtx);

			if (immediately)
			{
				pCurrentCommand = pCommand;
			}
			else
			{
				pCurrentCommand = pCommand;
				writeQueue.push(pCurrentCommand);
			}

			if (!immediately && writeQueue.size() > 1)
				return;

			boost::asio::async_write(socket(), boost::asio::buffer(pCurrentCommand->pData, pCurrentCommand->size),
				[this](const boost::system::error_code& error, std::size_t bytes_transferred) {
				WriteCommand* pNextCommand = nullptr;
				{
					boost::lock_guard<boost::recursive_mutex> lock(mtx);

					WriteCommand* pCompleteCommand = writeQueue.front();
					writeQueue.pop();
					delete pCompleteCommand;

					if (!writeQueue.empty())
					{
						pNextCommand = writeQueue.front();
					}
				}

				if (pNextCommand != nullptr)
				{
					PostWrite(true, pNextCommand->size, pNextCommand);
				}
			});
		}
	}

protected:
	void PacketProcess(PacketBase* pCmd)
	{
		switch (pCmd->id)
		{
		case LOGIN:
			Activate();
			break;

		case CHAT:
			ChatPacket* pChat = pCmd->Cast<ChatPacket>();
			std::cout << pChat->message << std::endl;
			break;
		}
	}

private:
	void PostRead()
	{
		socket().async_read_some(boost::asio::buffer(readBuffer),
			[this](const boost::system::error_code& error, const std::size_t bytes_transferred) {
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

				Close();
			}
			else
			{
				memcpy_s(&packetBuffer[prevLeftByte], 1024 - prevLeftByte, readBuffer.data(), bytes_transferred);

				std::size_t leftByte = prevLeftByte + bytes_transferred;
				std::size_t readByte = 0;

				const int PACKET_HEAD_SIZE = sizeof(PacketBase);
				while (leftByte > 0)
				{
					if (leftByte < PACKET_HEAD_SIZE)
						break;

					PacketBase* pBase = reinterpret_cast<PacketBase*>(&packetBuffer[readByte]);
					unsigned short packetSize = pBase->size;

					if (packetSize > leftByte)
						break;

					PacketProcess(pBase);
					leftByte -= packetSize;
					readByte += packetSize;
				}

				if (leftByte)
				{
					std::array<char, 512>	swapBuffer;
					memcpy_s(&swapBuffer[0], 512, &readBuffer[readByte], leftByte);
					memcpy_s(&readBuffer[0], 512, &swapBuffer[0], leftByte);
				}

				prevLeftByte = leftByte;

				PostRead();
			}
		});
	}

private:
	boost::asio::io_service& service;
	boost::asio::ip::tcp::socket socketInstance;
	std::array<char, 512> readBuffer;
	std::size_t prevLeftByte;

	std::array<char, 1024> packetBuffer;
	boost::recursive_mutex mtx;
	std::queue<WriteCommand*> writeQueue;

	bool activate;
};


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

	service.stop();

	client.Close();

	thread.join();

    return 0;
}

