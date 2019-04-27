#ifndef SERVER_HPP
#define SERVER_HPP

#include "Session.hpp"

#include <string>
#include <memory>

#include <boost/asio.hpp>

class Server
{
public:
	Server(boost::asio::io_service& io_service,
		const boost::asio::ip::tcp::endpoint& endpoint, const std::string& db_filename)
		: ioService_(io_service)
		, acceptor_(io_service, endpoint)
		, dbFilename_(db_filename)
	{ startAccept(); }

	void startAccept();
	void handleAccept(std::shared_ptr<Session> session,
		const boost::system::error_code& error);

private:
	boost::asio::io_service& ioService_;
	boost::asio::ip::tcp::acceptor acceptor_;
	std::string dbFilename_;
};

#endif // !SERVER_HPP
