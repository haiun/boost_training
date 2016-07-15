#pragma once
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <queue>

#include "../Packet/Packet.h"

class TCP_Client
{
public:
	TCP_Client(boost::asio::io_service& io_service)
		: service(io_service)
		, socketInstance(io_service)
		, activate(false)
		, bindedPacketProc(nullptr)
	{

	}

	~TCP_Client()
	{
		boost::lock_guard<boost::recursive_mutex>	lock(mtx);

		while (!writeQueue.empty())
		{
			WriteCommand* pWriteCommand = writeQueue.front();
			writeQueue.pop();
			pWriteCommand->Release();
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

			boost::asio::async_write(socket(), boost::asio::buffer(pCurrentCommand->pData, pCurrentCommand->size()),
				[this](const boost::system::error_code& error, std::size_t bytes_transferred) {
				WriteCommand* pNextCommand = nullptr;
				{
					boost::lock_guard<boost::recursive_mutex> lock(mtx);

					WriteCommand* pCompleteCommand = writeQueue.front();
					writeQueue.pop();
					pCompleteCommand->Release();

					if (!writeQueue.empty())
					{
						pNextCommand = writeQueue.front();
					}
				}

				if (pNextCommand != nullptr)
				{
					PostWrite(true, pNextCommand->size(), pNextCommand);
				}
			});
		}
	}

	template<typename PacketProcCallback>
	void BindPacketProc(PacketProcCallback&& callback)
	{
		bindedPacketProc = callback;
	}

protected:
	void PacketProcess(PacketBase* pCmd)
	{
		switch (pCmd->id)
		{
		case LOGIN:
		{
			Activate();
			break;
		}

		case CHAT:
		{
			ChatPacket* pChat = pCmd->Cast<ChatPacket>();
			std::cout << pChat->message << std::endl;
			break;
		}

		case MOVE:
		{
			MovePacket* pMove = pCmd->Cast<MovePacket>();
			printf("%zd: P(%d,%d) V(%d,%d) T(%ld)\n", pMove->sessionID, pMove->position[0], pMove->position[1], pMove->velocity[0], pMove->velocity[1], pMove->time);
			break;
		}
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
					if (bindedPacketProc != nullptr)
						bindedPacketProc(pBase);

					leftByte -= packetSize;
					readByte += packetSize;
				}

				if (leftByte)
				{
					std::array<char, 1024>	swapBuffer;
					memcpy_s(&swapBuffer[0], 1024, &readBuffer[readByte], leftByte);
					memcpy_s(&readBuffer[0], 1024, &swapBuffer[0], leftByte);
				}

				prevLeftByte = leftByte;

				PostRead();
			}
		});
	}

private:
	boost::asio::io_service& service;
	boost::asio::ip::tcp::socket socketInstance;
	std::array<char, 1024> readBuffer;
	std::size_t prevLeftByte;

	std::array<char, 1024> packetBuffer;
	boost::recursive_mutex mtx;
	std::queue<WriteCommand*> writeQueue;

	bool activate;
	std::function< void(PacketBase*) > bindedPacketProc;
};
