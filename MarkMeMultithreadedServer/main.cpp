#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <sstream>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

using boost::asio::ip::tcp;

enum class RequestType {
	UNKNOWN,
	INIT,
	STORE,
	GET_AND_STORE
};

bool init_server_db(const std::string& db_filename);

std::pair<RequestType, std::string> receive_user_request(std::shared_ptr<tcp::socket> sock);

void send_to_user(std::shared_ptr<tcp::socket> sock, const std::string& answer);



int main(int argc, char* argv[])
{
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
			
			auto received = receive_user_request(sock);

			SQLConnection conn("users.db");
			if (!conn.open()) {
				std::cout << "Unable to connect to database!" << std::endl;
				return;
			}
			SQLDatabase db;
			switch(received.first)
			{
			case RequestType::INIT: {
				std::cout << "Initializing user..." << std::endl;
				db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
				if ((db.getLastQueryResults()).empty()) {
					db.execute(conn, "INSERT INTO users(ip) VALUES('%1%');", client_ip);
					db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
				}
				auto res = db.getLastQueryResults();
				if (db.getLastErrorCode() != 0 || res.empty()) {
					send_to_user(sock, "error");
				}
				else {
					send_to_user(sock, res[0]["id"]);
				}
			}
				break;
			case RequestType::STORE: {
				std::cout << "Saving coordinates..." << std::endl;
				db.execute(conn, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", received.second, client_ip);
				if (db.getLastErrorCode() != 0) {
					send_to_user(sock, "error");
				}
				else {
					send_to_user(sock, "");
				}
			}
				break;
			case RequestType::GET_AND_STORE: {
				std::cout << "Saving user coordinates and getting friend coordinates..." << std::endl;
				std::istringstream is(received.second);

				std::string my_coord_1;
				std::string my_coord_2;
				is >> my_coord_1;
				is >> my_coord_2;

				db.execute(conn, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", my_coord_1 + " " + my_coord_2, client_ip);

				std::string friend_id;
				is >> friend_id;

				db.execute(conn, "SELECT coordinates FROM users WHERE id = '%1%';", friend_id);
				auto rcoord = db.getLastQueryResults();
				if (!rcoord.empty()) {
					send_to_user(sock, rcoord[0]["coordinates"]);
				}
				else {
					send_to_user(sock, "");
				}
			}
				break;
			}

			sock->close();
			std::cout << "Connection is closed...\n" << std::endl;
		});
	}

	pool.join();

	return 0;
}

bool init_server_db(const std::string& db_filename)
{
	SQLConnection connection(db_filename);
	if (!connection.open()) {
		std::cout << "Unable to connect to database!" << std::endl;
		return false;
	}
	SQLDatabase database;
	database.execute(connection, 
		"CREATE TABLE IF NOT EXISTS users(" \
		"	id INTEGER PRIMARY KEY AUTOINCREMENT," \
		"	ip VARCHAR(50)," \
		"	coordinates VARCHAR(50));");

	return true;
}

std::pair<RequestType, std::string> receive_user_request(std::shared_ptr<tcp::socket> sock)
{
	boost::asio::streambuf buf;
	boost::asio::streambuf::mutable_buffers_type bufs = buf.prepare(128);
	try {
		buf.commit(sock->receive(bufs));
	}
	catch (const std::length_error& e) {
		std::cerr << "Error: Data is greater than the size of the output sequence." << std::endl;
		return std::make_pair(RequestType::UNKNOWN, ""); 
	}
	catch (const boost::system::system_error& e) {
		std::cerr << "Error: Retrieving data is failed." << std::endl;
		return std::make_pair(RequestType::UNKNOWN, "");
	}
	catch (...) {
		std::cerr << "Oh, it seems something went wrong with getting the data." << std::endl;
		return std::make_pair(RequestType::UNKNOWN, "");
	}

	std::istream is(&buf);

	std::string rtype;
	is >> rtype;

	std::string rdata;
	std::getline(is, rdata);

	if (rtype.compare("/init") == 0) {
		return std::make_pair(RequestType::INIT, std::move(rdata));
	}
	else if (rtype.compare("/store") == 0) {
		return std::make_pair(RequestType::STORE, std::move(rdata));
	}
	else if (rtype.compare("/get_and_store") == 0) {
		return std::make_pair(RequestType::GET_AND_STORE, std::move(rdata));
	}
	
	return std::make_pair(RequestType::UNKNOWN, std::string());
}

void send_to_user(std::shared_ptr<tcp::socket> sock, const std::string& answer)
{
	boost::asio::streambuf b;
	std::ostream os(&b);
	os << answer << std::endl;
	try {
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
		return;
	}
	std::cout << "Answer: " << answer << " have been sent." << std::endl;
}