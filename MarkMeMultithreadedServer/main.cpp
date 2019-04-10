#define _CRT_SECURE_NO_WARNINGS

#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"
#include "ConsoleLogger.hpp"
#include "FileLogger.hpp"
#include "Logger.hpp"

#include "Session.hpp"
#include "Server.hpp"

#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

using boost::asio::ip::tcp;

enum class RequestType {
	UNKNOWN,
	INIT,
	STORE,
	STORE_AND_GET
};

std::string gen_random_password(int num);

bool init_server_db(const std::string& db_filename);
std::pair<RequestType, std::string> receive_user_request(tcp::socket& sock);
void send_to_user(tcp::socket& sock, const std::string& answer, Logger& logger);
bool init_user(tcp::socket& sock, SQLDatabase& db, SQLConnection& conn, Logger& logger);
bool store_coordinates(tcp::socket& sock, SQLDatabase& db, SQLConnection& conn, const std::string& coordinates, Logger& logger);
bool get_coordinates();
bool store_and_get_coordinates();
void handle_user_request(tcp::socket& sock, const std::pair<RequestType, std::string>& rdata, Logger& logger);

int main(/*int argc, char* argv[]*/)
{
	constexpr int count_servers = 1;
	constexpr int port = 8189;
	const char* filename = "users.db";

	try {
		// std::unique_ptr<Logger> logger = std::make_unique<ConsoleLogger>();

		if (!init_server_db(filename)) {
			// logger->fatal("Failed to initialize database!");
			std::cerr << "Failed to initialize database!" << std::endl;
			system("pause");
			return 1;
		}

		boost::asio::io_service io_service;

		std::vector<std::shared_ptr<Server>> servers;
		servers.reserve(count_servers);
		for (int i = 0; i < count_servers; ++i)
		{
			tcp::endpoint endpoint(tcp::v4(), port);
			//chat_server_ptr server(new Server(io_service, endpoint));
			servers.push_back(std::make_shared<Server>(io_service, endpoint));
		}

		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
		system("pause");
		return -1;
	}
	catch (...) {
		std::cerr << "Something is going wrong!" << "\n";
		system("pause");
		return -2;
	}

#if 0
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
			}
			catch (const boost::system::system_error& e) {
				logger->critical(e.what());
			}
			catch (...) {
				logger->fatal("Oh, it seems something went wrong!");
			}

			sock->close();
			logger->debug("Connection is closed...\n");
		});
	}

	pool.join();
#endif
	system("pause");
	return 0;
}

std::string gen_random_password(int num)
{
	std::default_random_engine generator(((std::chrono::system_clock::now()).time_since_epoch()).count());
	std::uniform_int_distribution<int> is_char(0, 1);
	std::uniform_int_distribution<int> gen_char(97, 122);
	std::uniform_int_distribution<int> gen_digit(48, 57);

	std::string result;
	result.reserve(num);
	for (int i = 0; i < num; ++i) {
		if (is_char(generator)) {
			result += static_cast<char>(gen_char(generator));
		}
		else {
			result += static_cast<char>(gen_digit(generator));
		}
	}

	return result;
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
		"	password VARCHAR(50)," \
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
		return std::make_pair(RequestType::STORE_AND_GET, std::move(rdata));
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

bool init_user(tcp::socket& sock, SQLDatabase& db, SQLConnection& conn, Logger& logger)
{
	const auto client_ip = ((sock.remote_endpoint()).address()).to_string();

	db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
	if ((db.getLastQueryResults()).empty()) {
		db.execute(conn, "INSERT INTO users(ip) VALUES('%1%');", client_ip);
		db.execute(conn, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
	}
	auto res = db.getLastQueryResults();
	if (db.getLastErrorCode() != 0 || res.empty()) {
		send_to_user(sock, "", logger);
		return false;
	}
	else {
		const auto pass = gen_random_password(8);
		const auto hashed_pass = std::to_string(std::hash<std::string>()(pass));
		db.execute(conn, "UPDATE users SET password = '%1%' WHERE id = '%2%';", hashed_pass, res[0]["id"]);
		send_to_user(sock, res[0]["id"] + " " + pass, logger);
	}

	return true;
}

bool store_coordinates(tcp::socket& sock, SQLDatabase& db, SQLConnection& conn, const std::string& coordinates, Logger& logger)
{
	const auto client_ip = ((sock.remote_endpoint()).address()).to_string();
	db.execute(conn, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", coordinates, client_ip);
	return db.getLastErrorCode() != 0;
}

bool get_coordinates(tcp::socket& sock, SQLDatabase& db, SQLConnection& conn, const std::string& friend_id, const std::string& friend_password, Logger& logger)
{
	db.execute(conn, "SELECT id FROM users WHERE id = '%1%' AND password = '%2%;'",
		friend_id, std::to_string(std::hash<std::string>()(friend_password)));

	if (db.getLastErrorCode() != 0) {
		send_to_user(sock, "", logger);
		return false;
	}

	auto ans_id = db.getLastQueryResults();
	if (ans_id.empty() || ans_id[0]["id"] != friend_id) {
		send_to_user(sock, "", logger);
		return false;
	}

	db.execute(conn, "SELECT coordinates FROM users WHERE id = '%1%';", friend_id);
	auto rcoord = db.getLastQueryResults();
	if (!rcoord.empty()) {
		send_to_user(sock, rcoord[0]["coordinates"], logger);
	}
	else {
		send_to_user(sock, "", logger);
	}
}

bool store_and_get_coordinates(tcp::socket& sock, SQLDatabase& db, SQLConnection& conn, const std::string& data, Logger& logger)
{
	std::istringstream is(data);

	std::string my_coord_1;
	std::string my_coord_2;
	is >> my_coord_1;
	is >> my_coord_2;

	store_coordinates(sock, db, conn, my_coord_1 + " " + my_coord_2, logger);

	std::string friend_id;
	is >> friend_id;

	std::string friend_password;
	is >> friend_password;

	return true;
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
		init_user(sock, db, conn, logger);
	}
		break;
	case RequestType::STORE: {
		logger.info("Saving coordinates...");
		store_coordinates(sock, db, conn, rdata.second, logger);
	}
		break;
	case RequestType::STORE_AND_GET: {
		logger.info("Saving user coordinates and getting friend coordinates...");
		
	}
		break;
	}
}