#include "TasksManager.hpp"
#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/basic_streambuf.hpp>
#include <boost/asio/streambuf.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
	SQLConnection connection("users.db");
	if (!connection.open()) {
		std::cout << "Unable to connect to database!" << std::endl;
		return 1;
	}
	SQLDatabase database;
	database.execute(connection, "CREATE TABLE IF NOT EXISTS users(" \
								 "	id INTEGER PRIMARY KEY AUTOINCREMENT," \
								 "	ip VARCHAR(50)," \
								 "	coordinates VARCHAR(50));");
	connection.close();

	boost::asio::io_service io_service;
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8189));

	TasksManager tm(4);

	while (true) {
		auto sock = std::make_shared<tcp::socket>(io_service);
		acceptor.accept(*sock);

		tm.addTask([sock]() {
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

			SQLConnection conn("users.db");
			if (!conn.open()) {
				std::cout << "Unable to connect to database!" << std::endl;
				return;
			}
			SQLDatabase db;
			std::string exists = "SELECT id FROM users WHERE ip = '";
			db.execute(conn, exists + client_ip + "';");

			if ((db.getLastQueryResults()).empty()) {
				std::string insert_coord = "INSERT INTO users(ip, coordinates) VALUES('";
				insert_coord += client_ip;
				insert_coord += "', '";
				insert_coord += coordinates;
				db.execute(conn, insert_coord + "');");
			}
			else {
				std::string update_coord = "UPDATE users SET coordinates = '";
				update_coord += coordinates;
				update_coord += "' WHERE ip = '";
				update_coord += client_ip;
				db.execute(conn, update_coord + "';");
			}

			std::string select_coord = "SELECT coordinates FROM users WHERE ip = '";
			db.execute(conn, select_coord + friends_ip + "';");
			auto results = db.getLastQueryResults();
			if (!results.empty()) {
				boost::asio::streambuf b;
				std::ostream os(&b);
				os << results[0]["coordinates"] << std::endl;
				b.consume(sock->send(b.data()));
				std::cout << "Coordinates: " << results[0]["coordinates"] << " have been sent." << std::endl;
			}

			sock->close();
			std::cout << "Connection is closed...\n" << std::endl;
		});
	}

	return 0;
}