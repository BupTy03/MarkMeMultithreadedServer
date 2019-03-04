#include "TaskManager.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/asio/basic_streambuf.hpp>
#include <boost/asio/streambuf.hpp>

using boost::asio::ip::tcp;

int main()
{
	std::unordered_map<std::string, std::string> clients;
	while (true) {
		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8189));

		auto sock = std::make_shared<tcp::socket>(io_service);
		acceptor.accept(*sock);

		if (!sock->is_open()) {
			sock->close();
			std::cout << "Connection is not established!" << std::endl;
			continue;
		}
		std::string client_ip = sock->remote_endpoint().address().to_string();
		std::cout << "Connection with host " << client_ip << " is established!" << std::endl;

		boost::asio::streambuf buf;
		boost::asio::streambuf::mutable_buffers_type bufs = buf.prepare(512);
		buf.commit(sock->receive(bufs));

		std::istream is(&buf);

		std::string friends_ip;
		std::getline(is, friends_ip);

		std::cout << "Friends IP address received: " << friends_ip << std::endl;

		std::string coordinates;
		std::getline(is, coordinates);

		std::cout << "Coordinates received: " << coordinates << std::endl;

		clients[client_ip] = coordinates;

		if (clients.find(friends_ip) != std::end(clients)) {
			boost::asio::streambuf b;
			std::ostream os(&b);
			os << clients[friends_ip] << std::endl;
			b.consume(sock->send(b.data()));
			std::cout << "Coordinates: " << clients[friends_ip] << " have been sent." << std::endl;
		}

		sock->close();
		std::cout << "Connection is closed..." << std::endl;
	}
	return 0;
}