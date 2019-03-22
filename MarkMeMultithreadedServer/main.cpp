/*
#include "TaskManager.hpp"
#include "ConcurrentMap.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/asio/basic_streambuf.hpp>
#include <boost/asio/streambuf.hpp>

using boost::asio::ip::tcp;

*/

#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	const std::string doubler = "CREATE TABLE IF NOT EXISTS foo(a,b,c); INSERT INTO foo VALUES(1,2,3); INSERT INTO foo SELECT * FROM foo;";
	const std::string select = "SELECT * FROM foo;";

	SQLConnection connection("database.db");
	SQLDatabase db;
	if (!connection.open()) {
		std::cout << "Ñonnection is not established!" << std::endl;
	}
	else if (!db.execute(connection, select)) {
		std::cout << "Execution failed!" << std::endl;
		std::cout << "Error: " << db.getLastErrorMessage() << std::endl;
	}
	else {
		for (auto& record : db.getLastQueryResults()) {
			for (auto&[key, val] : record) {
				std::cout << key << " = " << val << std::endl;
			}
			std::cout << std::endl;
		}
		std::cout << "Executed successfully!" << std::endl;
	}

	system("pause");

#if 0
	my::ConcurrentMap clients;
	boost::asio::io_service io_service;
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8189));

	my::TaskManager tm(10);

	while (true) {
		auto sock = std::make_shared<tcp::socket>(io_service);
		acceptor.accept(*sock);

		tm.emplaceTask([sock, &clients]() {
			if (!sock->is_open()) {
				sock->close();
				std::cout << "Connection is not established!" << std::endl;
				return;
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

			clients.setValue(client_ip, coordinates);

			if (clients.containsKey(friends_ip)) {
				boost::asio::streambuf b;
				std::ostream os(&b);
				os << clients.getValue(friends_ip) << std::endl;
				b.consume(sock->send(b.data()));
				std::cout << "Coordinates: " << clients.getValue(friends_ip) << " have been sent." << std::endl;
			}

			sock->close();
			std::cout << "Connection is closed..." << std::endl;
		});
	}
#endif

	return 0;
}