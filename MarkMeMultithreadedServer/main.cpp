#define _CRT_SECURE_NO_WARNINGS

#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"
#include "ConsoleLogger.hpp"
#include "FileLogger.hpp"
#include "Logger.hpp"

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
std::pair<RequestType, std::string> receive_user_request(tcp::socket& sock);
void send_to_user(tcp::socket& sock, const std::string& answer, Logger& logger);
void handle_user_request(tcp::socket& sock, const std::pair<RequestType, std::string>& rdata, Logger& logger);

int main(int argc, char* argv[])
{
	std::unique_ptr<Logger> logger = std::make_unique<FileLogger>("log.txt");

	if (!init_server_db("users.db")) {
		logger->fatal("Failed to initialize database!");
		return 1;
	}

	boost::asio::io_service io_service;
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8189));

	boost::asio::thread_pool pool;
	while (true) {
		auto sock = std::make_shared<tcp::socket>(io_service);
		acceptor.accept(*sock);

		boost::asio::dispatch(pool, [sock, &logger]() {
			if (!sock->is_open()) {
				sock->close();
				logger->critical("Connection is not established!");
				return;
			}

			logger->debug("Connection with host " + ((sock->remote_endpoint()).address()).to_string() + " is established!");

			try {
				handle_user_request(*sock, receive_user_request(*sock), *logger);
			}
			catch (const std::length_error& e) {
				logger->critical(e.what());
				sock->close();
				logger->debug("Connection is closed...\n");
				return;
			}
			catch (const boost::system::system_error& e) {
				logger->critical(e.what());
				sock->close();
				logger->debug("Connection is closed...\n");
				return;
			}
			catch (...) {
				logger->fatal("Oh, it seems something went wrong!");
				sock->close();
				logger->debug("Connection is closed...\n");
				return;
			}

			sock->close();
			logger->debug("Connection is closed...\n");
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

std::pair<RequestType, std::string> receive_user_request(tcp::socket& sock)
{
	boost::asio::streambuf buf;
	boost::asio::streambuf::mutable_buffers_type bufs = buf.prepare(128);

	buf.commit(sock.receive(bufs));

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

void send_to_user(tcp::socket& sock, const std::string& answer, Logger& logger)
{
	boost::asio::streambuf b;
	std::ostream os(&b);
	os << answer << std::endl;
	b.consume(sock.send(b.data()));
	logger.info("Answer: " + answer + " have been sent.");
}

void handle_user_request(tcp::socket& sock, const std::pair<RequestType, std::string>& rdata, Logger& logger)
{
	SQLConnection conn("users.db");
	if (!conn.open()) {
		logger.critical("Unable to connect to database!");
		return;
	}
	SQLDatabase db;
	auto client_ip = ((sock.remote_endpoint()).address()).to_string();
	switch (rdata.first) {
	case RequestType::INIT: {
		logger.info("Initializing user...");
		db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
		if ((db.getLastQueryResults()).empty()) {
			db.execute(conn, "INSERT INTO users(ip) VALUES('%1%');", client_ip);
			db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
		}
		auto res = db.getLastQueryResults();
		if (db.getLastErrorCode() != 0 || res.empty()) {
			send_to_user(sock, "", logger);
		}
		else {
			send_to_user(sock, res[0]["id"], logger);
		}
	}
		break;
	case RequestType::STORE: {
		logger.info("Saving coordinates...");
		db.execute(conn, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", rdata.second, client_ip);
		if (db.getLastErrorCode() != 0) {
			send_to_user(sock, "", logger);
		}
		else {
			send_to_user(sock, "", logger);
		}
	}
		break;
	case RequestType::GET_AND_STORE: {
		logger.info("Saving user coordinates and getting friend coordinates...");
		std::istringstream is(rdata.second);

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
			send_to_user(sock, rcoord[0]["coordinates"], logger);
		}
		else {
			send_to_user(sock, "", logger);
		}
	}
		break;
	}
}