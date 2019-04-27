#define DEBUG

#include "SimpleLogger.hpp"
#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"
#include "Server.hpp"

#include <iostream>
#include <regex>
#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <stdexcept>
#include <boost/asio.hpp>

void print_help();
bool setup_args(int argc, char* argv[], int& count_servers, std::string& db_filename);
bool init_server_db(const std::string& db_filename);
void start(const int count_of_servers, const int port, const std::string& filename);

int main(int argc, char* argv[])
{
	using std::cout;
	using std::endl;

	constexpr int port = 8189;
	int count_servers = 1;
	std::string db_filename = "users.db";

	if (!setup_args(argc, argv, count_servers, db_filename)) {
		system("pause");
		return 1;
	}

	if (!init_server_db(db_filename)) {
		cout << "Unable to connect to the database!\nFilename: " << db_filename << endl;
		system("pause");
		return 2;
	}

	while (true) {
		try {
			start(count_servers, port, db_filename);
		}
		catch (const std::exception& e) {
			cout << "Server is down. :/" << endl;
			LogFatal(std::string("Exception: ") + e.what());
			return 2;
		}
		catch (...) {
			cout << "Server is down. :/" << endl;
			LogFatal("Something went wrong.");
			return 3;
		}
		cout << "Restarting server..." << endl;
	}
	return 0;
}

void print_help()
{
	std::cout << "Options:\n"
		<< "\t-srv=[positive integer]\t\tCount of servers."
		<< "\t-db=[filename]\t\tName of file where database is stored." << std::endl;
}

bool setup_args(int argc, char* argv[], int& count_servers, std::string& db_filename)
{
	using std::cout;
	using std::endl;
	using std::stoi;

	if (argc > 3) {
		cout << "It's too many command line options." << endl;
		return false;
	}

	std::regex reg("^\\-(\\S+)=(\\S+)$");
	for (int i = 1; i < argc; ++i) {
		std::cmatch cm;
		if (!std::regex_match(argv[i], cm, reg)) {
			cout << "Error: Invalid argument format: \"" << argv[i] << "\" :(" << endl;
			return false;
		}
		const std::string arg_name = cm[1].str();
		const std::string arg_value = cm[2].str();
		if (arg_name == "srv") {
			try {
				count_servers = stoi(arg_value);
			}
			catch (std::exception& e) {
				cout << "Error: Count of servers must be positive integer." << endl;
				return false;
			}
			if (count_servers <= 0) {
				cout << "Error: Count of servers must be POSITIVE integer." << endl;
				return false;
			}
		}
		else if (arg_name == "db") {
			db_filename = arg_value;
		}
		else {
			cout << "Unrecognized command line option: \"" << arg_name << "\"  :[" << endl;
			print_help();
		}
	}

	return true;
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

void start(const int count_of_servers, const int port, const std::string& filename)
{
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;
	std::vector<std::shared_ptr<Server>> servers;
	servers.reserve(count_of_servers);
	for (int i = 0; i < count_of_servers; ++i) {
		servers.push_back(std::make_shared<Server>(io_service, tcp::endpoint(tcp::v4(), port), filename));
	}

	io_service.run();
}