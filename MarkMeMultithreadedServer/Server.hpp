#ifndef SERVER_HPP
#define SERVER_HPP

#include "Session.hpp"

#include <memory>

#include <boost/asio.hpp>

class Server
{
public:
	Server(boost::asio::io_service& io_service,
		const boost::asio::ip::tcp::endpoint& endpoint)
		: io_service_(io_service)
		, acceptor_(io_service, endpoint)
	{ startAccept(); }

	void startAccept();
	void handleAccept(std::shared_ptr<Session> session,
		const boost::system::error_code& error);

private:
	boost::asio::io_service& io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // !SERVER_HPP
