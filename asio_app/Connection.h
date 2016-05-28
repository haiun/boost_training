#pragma once

class Connection
{
public:
	boost::asio::ip::tcp::socket* socket;
	boost::system::error_code error;
};
