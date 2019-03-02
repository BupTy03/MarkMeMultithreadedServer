#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

using boost::asio::ip::tcp;

typedef  boost::shared_ptr<tcp::socket> socket_ptr;

void session(socket_ptr sock);


int main()
{
	while (true)
	{
		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));
		socket_ptr sock(new tcp::socket(io_service));
		acceptor.accept(*sock);
		boost::thread t(boost::bind(session, sock));
	}
}


void session(socket_ptr sock)
{
	std::cout << "connect!" << std::endl;
}