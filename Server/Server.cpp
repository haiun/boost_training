#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <iostream>

class ServerSession
{
public:
	ServerSession(boost::asio::io_service& service)
		: socketInstance(service)
	{

	}

	boost::asio::ip::tcp::socket& socket()
	{
		return socketInstance;
	}

	void PostRead()
	{
		memset(&recvBuffer, '\0', sizeof(recvBuffer));
		socketInstance.async_read_some
		(
			boost::asio::buffer(recvBuffer),
			boost::bind(&ServerSession::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
		);
	}

private:
	void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
	{

	}

	void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
	{
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
		}
		else
		{
			const std::string recvData = recvBuffer.data();

			std::cout << "read:" << recvData << std::endl;

			writeMessage = recvBuffer;

			boost::asio::async_write(socket(), boost::asio::buffer(writeMessage),
				boost::bind(&ServerSession::handle_write, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

			PostRead();
		}
	}

	boost::asio::ip::tcp::socket socketInstance;
	std::array<char, 128> writeMessage;
	std::array<char, 128> recvBuffer;
};

class TCP_Server
{
public:
	TCP_Server(boost::asio::io_service& io_service)
		: acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 33440))
		, pSession(nullptr)
	{
		StartAccept();
	}

	~TCP_Server()
	{
		if (pSession != nullptr)
			delete pSession;
	}

public:
	void StartAccept()
	{
		std::cout << "StartAccept" << std::endl;

		pSession = new ServerSession(acceptor.get_io_service());
		acceptor.async_accept(pSession->socket(),
			boost::bind(&TCP_Server::handle_accept, this, pSession, boost::asio::placeholders::error));
	}

	void handle_accept(ServerSession* pSession, const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "handle_accept" << std::endl;
			pSession->PostRead();
		}
	}

private:
	boost::asio::ip::tcp::acceptor acceptor;
	ServerSession* pSession;
	int seqNumber;
};

void main()
{
	boost::asio::io_service service;
	TCP_Server server(service);
	service.run();

	getchar();
}



/*
int main()
{
	boost::asio::io_service service;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 33440);
	boost::asio::ip::tcp::acceptor acceptor(service, endpoint);
	boost::asio::ip::tcp::socket socket(service);
	
	acceptor.accept(socket);

	for (;;)
	{
		std::array<unsigned char, 128> buf;
		buf.assign(0);
		boost::system::error_code error;
		size_t len = socket.read_some(boost::asio::buffer(buf), error);

		if (error)
		{
			if (error == boost::asio::error::eof)
			{
				std::cout << "disconnected" << std::endl;
			}
			else
			{
				std::cout << "error : " << error.value() << " message : " << error.message() << std::endl;
			}
			break;
		}

		char message[128] = { 0, };
		sprintf_s(message, 127, "re:%s", &buf[0]);
		size_t messageLen = strnlen_s(message, 127);

		boost::system::error_code ignored_error;
		socket.write_some(boost::asio::buffer(message, messageLen), ignored_error);
	}

	getchar();

    return 0;
}
*/