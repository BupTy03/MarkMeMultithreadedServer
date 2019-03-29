#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

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

	boost::asio::thread_pool pool;
	while (true) {
		auto sock = std::make_shared<tcp::socket>(io_service);
		acceptor.accept(*sock);

		boost::asio::dispatch(pool, [sock]() {
			if (!sock->is_open()) {
				sock->close();
				std::cout << "Connection is not established!" << std::endl;
				return;
			}
			std::string client_ip = ((sock->remote_endpoint()).address()).to_string();
			std::cout << "Connection with host " << client_ip << " is established!" << std::endl;

			boost::asio::streambuf buf;
			boost::asio::streambuf::mutable_buffers_type bufs = buf.prepare(128);
			try {
				buf.commit(sock->receive(bufs));
			}
			catch (const std::length_error& e) {
				std::cerr << "Error: Data is greater than the size of the output sequence." << std::endl;
				return;
			}
			catch (const boost::system::system_error& e) {
				std::cerr << "Error: Retrieving data is failed." << std::endl;
				return;
			}
			catch (...) {
				std::cerr << "Oh, it seems something went wrong with getting the data." << std::endl;
			}

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
			db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);

			if ((db.getLastQueryResults()).empty()) {
				db.execute(conn, "INSERT INTO users(ip, coordinates) VALUES('%1%', '%2%');", client_ip, coordinates);
			}
			else {
				db.execute(conn, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", coordinates, client_ip);
			}

			db.execute(conn, "SELECT coordinates FROM users WHERE ip = '%1%';", friends_ip);
			auto results = db.getLastQueryResults();
			if (!results.empty()) {
				boost::asio::streambuf b;
				std::ostream os(&b);
				os << results[0]["coordinates"] << std::endl;
				try{
					b.consume(sock->send(b.data()));
				}
				catch (const std::length_error& e) {
					std::cerr << "Error: Data is greater than the size of the input sequence." << std::endl;
					return;
				}
				catch (const boost::system::system_error& e) {
					std::cerr << "Error: Sending data is failed." << std::endl;
					return;
				}
				catch (...) {
					std::cerr << "Oh, it seems something went wrong with sending the data." << std::endl;
				}
				std::cout << "Coordinates: " << results[0]["coordinates"] << " have been sent." << std::endl;
			}

			sock->close();
			std::cout << "Connection is closed...\n" << std::endl;
		});
	}

	pool.join();

	return 0;
}