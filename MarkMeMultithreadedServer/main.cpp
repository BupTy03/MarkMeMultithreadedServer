#include "SimpleLogger.hpp"
#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"
#include "Server.hpp"

#include <iostream>
#include <regex>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <stdexcept>
#include <boost/asio.hpp>

void print_help();
bool setup_args(int argc, char* argv[], int& count_servers, std::string& db_filename, int& server_port);
bool init_server_db(const std::string& db_filename);
void start(const int count_of_servers, const int port, const std::string& filename);

int main(int argc, char* argv[])
{
	using std::cout;
	using std::cerr;
	using std::endl;

	int port = 8189;
	int count_servers = 1;
	std::string db_filename = "users.db";

	if (!setup_args(argc, argv, count_servers, db_filename, port)) {
		cerr << "Unable to setup command line arguments!" << endl;
		system("pause");
		return 1;
	}

	if (!init_server_db(db_filename)) {
		cerr << "Unable to connect to the database!\nFilename: " << db_filename << endl;
		system("pause");
		return 2;
	}

	while (true) {
		try {
			start(count_servers, port, db_filename);
		}
		catch (const std::exception& e) {
			cerr << "Server is down. :/" << endl;
			LogFatal(std::string("Exception: ") + e.what());
		}
		catch (...) {
			cerr << "Server is down. :/" << endl;
			LogFatal("Something went wrong.");
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

bool setup_args(int argc, char* argv[], int& count_servers, std::string& db_filename, int& server_port)
{
	using std::cerr;
	using std::endl;
	using std::stoi;

	if (argc > 4) {
		cerr << "It's too many command line options." << endl;
		return false;
	}

	std::regex reg("^\\-(\\S+)=(\\S+)$");
	for (int i = 1; i < argc; ++i) {
		std::cmatch cm;
		if (!std::regex_match(argv[i], cm, reg) || cm.size() != 3) {
			cerr << "Error: Invalid argument format: \"" << argv[i] << "\" :(" << endl;
			return false;
		}
		const std::string arg_name = cm[1].str();
		const std::string arg_value = cm[2].str();
		if (arg_name == "srv") {
			try {
				count_servers = stoi(arg_value);
			}
			catch (std::exception& e) {
				cerr << "Error: Count of servers must be positive integer." << endl;
				return false;
			}
			if (count_servers <= 0) {
				cerr << "Error: Count of servers must be POSITIVE integer." << endl;
				return false;
			}
		}
		else if (arg_name == "db") {
			db_filename = arg_value;
		}
		else if (arg_name == "port") {
			try {
				server_port = stoi(arg_value);
			}
			catch (std::exception& e) {
				cerr << "Error: Server port must be positive integer." << endl;
				return false;
			}
			if (server_port < 0) {
				cerr << "Error: Server port must be POSITIVE integer." << endl;
				return false;
			}
		}
		else {
			cerr << "Unrecognized command line option: \"" << arg_name << "\"  :[" << endl;
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