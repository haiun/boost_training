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

struct PacketBase
{
	unsigned short id;
	unsigned short size;

	PacketBase(unsigned short _id, unsigned short _size) : id(_id), size(_size) {}
	~PacketBase() {}
};

struct LoginPacket : public PacketBase
{
	LoginPacket() : PacketBase(LOGIN, sizeof(LoginPacket)) {}
};

struct ChatPacket : public PacketBase
{
	ChatPacket() : PacketBase(CHAT, sizeof(ChatPacket)) {}
};

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
	WriteCommand(void* _pData, std::size_t _size) : pData(_pData), size(_size) {}
	~WriteCommand() { delete[] pData; }

public:
	WriteCommand* Clone()
	{
		char* deepCopyData = new char[size];
		memcpy_s(pData, size, deepCopyData, size);
		return new WriteCommand(deepCopyData, size);
	}

public:
	void* pData;
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
		socket().async_connect(endPoint, boost::bind(&TCP_Client::handle_connect, this, boost::asio::placeholders::error));
	}

	void Close()
	{
		if (socket().is_open())
		{
			socket().close();
		}
	}

	void PostWrite(const bool immediately, const int size, WriteCommand* pCommand)
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
				pCurrentCommand = pCommand->Clone();
				writeQueue.push(pCurrentCommand);
			}

			if (!immediately && writeQueue.size() > 1)
				return;

			boost::asio::async_write(socket(), boost::asio::buffer(pCurrentCommand->pData, pCurrentCommand->size),
				boost::bind(&TCP_Client::handle_write, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}

protected:
	void PacketProcess(Command* pCmd)
	{

	}

private:
	void PostRead()
	{
		memset(&readBuffer[0], '\0', readBuffer.size());

		socket().async_read_some(boost::asio::buffer(readBuffer),
			boost::bind(&TCP_Client::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_connect(const boost::system::error_code& error)
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
	}

	void handle_write(const boost::system::error_code& error, std::size_t bytes_transferred)
	{

	}

	void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred)
	{

	}

private:
	boost::asio::io_service& service;
	boost::asio::ip::tcp::socket socketInstance;
	std::array<char, 512> readBuffer;
	std::size_t prevLeftByte;

	std::array<char, 128> packetBuffer;
	boost::recursive_mutex mtx;
	std::queue<WriteCommand*> writeQueue;

	bool activate;
};


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

