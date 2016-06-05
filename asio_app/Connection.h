#pragma once
#include <boost/array.hpp>
#include <boost/atomic/atomic.hpp>

class Connection
{
	enum { LENGTH = 4 };
	enum { BUFFER = 512 };

public:
	Connection(boost::asio::io_service& service);

public:
	void RecvLength();
	void RecvBody(std::size_t bodySize);
	void Send(void* data, std::size_t size);
	void Send(boost::asio::mutable_buffer buffer);

	void OnRecvLength(const boost::system::error_code& error, std::size_t byte_transferred);
	void OnRecvBody(const boost::system::error_code& error, std::size_t byte_transferred);
	void OnSend(const boost::system::error_code& error, std::size_t byte_transferred);

public:
	boost::asio::ip::tcp::socket socket;
	boost::array<BYTE, LENGTH> recvLengthBuffer;
	boost::array<BYTE, BUFFER> recvBodyBuffer;
	BYTE* writeBuffer;
	std::queue<void*>	sendBufferQueue;
};

class RoomConnection : public Connection
{
public:
	RoomConnection(boost::asio::io_service& service) : Connection(service), refRoom(0) {}

public:
	boost::atomics::atomic_int refRoom;
};
