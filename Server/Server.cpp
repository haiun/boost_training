#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <iostream>
#include <queue>


class WriteCommand
{
public:
	WriteCommand(void* _pData, std::size_t _size) : pData(_pData), size(_size)	{}
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

class ServerInterface
{
public:
	virtual ~ServerInterface() {}

public:
	virtual void CloseSession(const std::size_t sessionID) = 0;
	virtual void PacketProcess(const std::size_t sessionID, WriteCommand* cmd) = 0;
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
		while (!writeQueue.empty())
		{
			WriteCommand* pWriteCommand = writeQueue.front();
			writeQueue.pop();
			delete pWriteCommand;
		}
	}

	void Init()
	{
		packetBufferMark = 0;
	}

	boost::asio::ip::tcp::socket& socket()
	{
		return socketInstance;
	}

	void PostRead()
	{
		socket().async_read_some
		(
			boost::asio::buffer(recvBuffer),
			boost::bind(&ServerSession::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
		);
	}

	void PostSend(const bool immediatly, WriteCommand* pCommand)
	{
		WriteCommand* pCurrentCommand = nullptr;

		if (immediatly)
		{
			pCurrentCommand = pCommand;
		}
		else
		{
			pCurrentCommand = pCommand->Clone();
			writeQueue.push(pCurrentCommand);
		}

		if (!immediatly && writeQueue.size() > 1)
			return;

		boost::asio::async_write(socket(), boost::asio::buffer(pCurrentCommand->pData, pCurrentCommand->size),
			boost::bind(&ServerSession::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

private:
	void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
	{
		WriteCommand* pPrevCmd = writeQueue.front();
		writeQueue.pop();
		delete pPrevCmd;

		if (!writeQueue.empty())
		{
			WriteCommand* pCmd = writeQueue.front();
			PostSend(true, pCmd);
		}
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

			pServer->CloseSession(sessionID);
		}
		else
		{
			const std::string recvData = recvBuffer.data();

			std::size_t readDataByte = packetBufferMark + bytes_transferred;
			std::size_t readData = 0;

			const int PACKET_HEAD_SIZE = 8;
			while (readDataByte > 0)
			{
				if (readDataByte < PACKET_HEAD_SIZE)
					break;

				//if 
			}

			PostRead();
		}
	}

	std::size_t sessionID;
	boost::asio::ip::tcp::socket socketInstance;
	std::queue<WriteCommand*>	writeQueue;

	std::array<char, 128> recvBuffer;
	std::size_t packetBufferMark;
	ServerInterface* pServer;
};

class TCP_Server : public ServerInterface
{
public:
	TCP_Server(boost::asio::io_service& io_service)
		: acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 33440))
		, pSession(nullptr)
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
		//PostAccept();
	}

	void CloseSession(const std::size_t sessionID)
	{
		sessionList[sessionID]->socket().close();
		sessionQueue.push_back(sessionID);
	}

	void PacketProcess(const std::size_t sessionID, WriteCommand* cmd)
	{

	}

private:
	bool PostAccept()
	{
		if (sessionQueue.empty())
		{
			isAccepting = false;
			return false;
		}

		isAccepting = true;
		std::size_t sessionID = sessionQueue.front();
		sessionQueue.pop_front();

		ServerSession* targetSession = sessionList[sessionID];
		acceptor.async_accept(targetSession->socket(),
			boost::bind(&TCP_Server::handle_accept, this, targetSession, boost::asio::placeholders::error));

		return true;
	}

	void handle_accept(ServerSession* pSession, const boost::system::error_code& error)
	{
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
	}

private:
	boost::asio::ip::tcp::acceptor acceptor;
	ServerSession* pSession;
	bool isAccepting;
	std::vector<ServerSession*>	sessionList;
	std::deque<std::size_t> sessionQueue;
};

void main()
{
	boost::asio::io_service service;
	TCP_Server server(service);
	service.run();

	getchar();
}
