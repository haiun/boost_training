#include "stdafx.h"
#include "Connection.h"
#include "Packet.h"

Connection::Connection(boost::asio::io_service& service)
	: socket(service)
{

}

void Connection::RecvLength()
{
	socket.async_receive(boost::asio::buffer(recvLengthBuffer, LENGTH),
		0,
		boost::bind(&Connection::OnRecvLength,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::RecvBody(std::size_t size)
{
	socket.async_receive(boost::asio::buffer(recvBodyBuffer, size - LENGTH),
		0,
		boost::bind(&Connection::OnRecvBody,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::Send(void* data, std::size_t size)
{
	socket.async_send(boost::asio::buffer(writeBuffer, size),
		0,
		boost::bind(&Connection::OnSend,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::Send(boost::asio::mutable_buffer buffer)
{
	std::size_t size = boost::asio::buffer_size(buffer);
	socket.async_send(boost::asio::buffer(buffer, size),
		0,
		boost::bind(&Connection::OnSend,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::OnRecvLength(const boost::system::error_code& error, std::size_t byte_transferred)
{
	if (error)
		return;

	unsigned __int32 length = GetLength(recvLengthBuffer.data());
	RecvBody(length);
}

void Connection::OnRecvBody(const boost::system::error_code& error, std::size_t byte_transferred)
{
	if (error)
		return;

	RecvLength();
}

void Connection::OnSend(const boost::system::error_code& error, std::size_t byte_transferred)
{

}
