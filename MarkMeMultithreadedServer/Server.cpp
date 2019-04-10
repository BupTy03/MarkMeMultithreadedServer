#include "Server.hpp"

#include <boost/bind.hpp>

void Server::startAccept()
{
	auto next_session = std::make_shared<Session>(io_service_, "users.db");
	acceptor_.async_accept(next_session->socket(),
		boost::bind(&Server::handleAccept, this, next_session,
			boost::asio::placeholders::error));
}

void Server::handleAccept(std::shared_ptr<Session> session,
	const boost::system::error_code& error)
{
	if (!error) {
		session->start();
	}

	startAccept();
}