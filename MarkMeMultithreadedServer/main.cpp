#define DEBUG

#include "SimpleLogger.hpp"
#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"
#include "Server.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <stdexcept>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

bool init_server_db(const std::string& db_filename);

int main(/*int argc, char* argv[]*/)
{
	constexpr int count_servers = 1;
	constexpr int port = 8189;
	const char* filename = "users.db";

	try {
		if (!init_server_db(filename)) {
			LogFatal("Unable to connect to database!");
			system("pause");
			return 1;
		}

		boost::asio::io_service io_service;

		std::vector<std::shared_ptr<Server>> servers;
		servers.reserve(count_servers);
		for (int i = 0; i < count_servers; ++i) {
			servers.push_back(std::make_shared<Server>(io_service, tcp::endpoint(tcp::v4(), port)));
		}

		io_service.run();
	}
	catch (std::exception& e) {
		LogFatal(std::string("Exception: ") + e.what());
		system("pause");
		return -1;
	}
	catch (...) {
		LogFatal("Something is going wrong!");
		system("pause");
		return -2;
	}

	system("pause");
	return 0;
}

bool init_server_db(const std::string& db_filename)
{
	SQLConnection connection(db_filename);
	if (!connection.open()) {
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