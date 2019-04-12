#include "Session.hpp"
#include "SimpleLogger.hpp"

#include <iostream>
#include <sstream>
#include <random>
#include <functional>
#include <regex>

#include <boost/bind.hpp>

void Session::start()
{
	(SimpleLogger::Instance()).info("Session is started...");
	if (!buffer_.empty()) {
		buffer_.clear();
	}
	buffer_.resize(BUFFER_SIZE_IN_BYTES);

	socket_.async_read_some(
		boost::asio::buffer(&buffer_[0], std::size(buffer_)),
		boost::bind(
			&Session::receiveDataHandle, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred
		)
	);

	/*
	boost::asio::async_read(socket_,
		boost::asio::buffer(&buffer_[0], buffer_.size() - 1),
		boost::bind(
			&Session::receiveDataHandle, shared_from_this(),
			boost::asio::placeholders::error));
	*/
}

void Session::receiveDataHandle(const boost::system::error_code& error, std::size_t bytes_tr)
{
	if (error) {
		(SimpleLogger::Instance()).critical("Error while receiving data: " + error.message());
		return;
	}

	auto data_str = buffer_.substr(0, bytes_tr);

	(SimpleLogger::Instance()).debug("Received data: " + data_str);

	std::istringstream is(data_str);
	std::string header;
	is >> header;

	if (header.compare("/init") == 0) {
		(SimpleLogger::Instance()).info("Initializing user...");
		initUser();
	}
	else if (header.compare("/store") == 0) {
		(SimpleLogger::Instance()).info("Storing coordinates...");
		std::string coordinates;
		is >> coordinates;
		if (checkCoordinatesFormat(coordinates)) {
			storeCoordinates(coordinates);
		}
		else {
			std::cerr << "Error: Wrong coordinates format" << std::endl;
		}
	}
	else if (header.compare("/get") == 0) {
		(SimpleLogger::Instance()).info("Getting coordinates...");

		std::string friend_id;
		std::string friend_pass;

		is >> friend_id;
		is >> friend_pass;
	
		if (checkFriendFormat(friend_id, friend_pass)) {
			getFriendCoordinates(friend_id, friend_pass);
		}
		else {
			(SimpleLogger::Instance()).critical("Error: Wrong friend format");
		}
	}
	else if (header.compare("/store_and_get") == 0) {
		(SimpleLogger::Instance()).info("Storing and getting coordinates...");

		std::string latitude;
		std::string longitude;
		std::string friend_id;
		std::string friend_pass;

		is >> latitude;
		is >> longitude;
		is >> friend_id;
		is >> friend_pass;

		auto coordinates = latitude + " " + longitude;

		if (checkCoordinatesFormat(coordinates)) {
			storeCoordinates(coordinates);
			(SimpleLogger::Instance()).debug("Coordinates: " + coordinates + " are stored.");
		}
		else {
			(SimpleLogger::Instance()).critical("Error: Wrong coordinates format");
		}

		if (checkFriendFormat(friend_id, friend_pass)) {
			getFriendCoordinates(friend_id, friend_pass);
		}
		else {
			(SimpleLogger::Instance()).critical("Error: Wrong friend format");
		}
	}
	(SimpleLogger::Instance()).info("Connection closed...\n");
}

std::string Session::generatePassword(int num)
{
	std::default_random_engine generator(((std::chrono::system_clock::now()).time_since_epoch()).count());
	std::uniform_int_distribution<int> is_char(0, 1);
	std::uniform_int_distribution<int> gen_char(static_cast<int>('a'), static_cast<int>('z'));
	std::uniform_int_distribution<int> gen_digit(static_cast<int>('0'), static_cast<int>('9'));

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

bool Session::checkCoordinatesFormat(const std::string& coordinates)
{
	std::regex reg("\\d+,\\d+\\s\\d+,\\d+");
	return std::regex_match(coordinates, reg);
}

bool Session::checkFriendFormat(const std::string& friend_id, const std::string& friend_password)
{
	if (friend_id.empty() || friend_password.empty()) {
		return false;
	}

	for (auto ch : friend_id) {
		if (!(ch >= '0' && ch <= '9')) {
			return false;
		}
	}

	for (auto ch : friend_password) {
		if (!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'z')) {
			return false;
		}
	}

	return true;
}

bool Session::sendToUser(const std::string& data)
{
	(SimpleLogger::Instance()).debug("Sending data: " + data);
	boost::system::error_code ec;
	boost::asio::write(socket_, boost::asio::buffer(&data[0], data.size()), ec);
	if (ec) {
		(SimpleLogger::Instance()).critical("Error while sending data.");
	}
	return !ec;
	/*
		boost::asio::async_write(socket_,
			boost::asio::buffer(&data[0], data.size()),
			boost::bind(&Session::handleWrite, shared_from_this(),
			boost::asio::placeholders::error));
	*/
}

void Session::initUser()
{
	const auto client_ip = ((socket_.remote_endpoint()).address()).to_string();

	UniqueSQLConnection uconn(dbConnection_);

	database_.execute(dbConnection_, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
	auto r = database_.getLastQueryResults();
	if (r.empty() || r[0]["id"] == "NULL") {
		database_.execute(dbConnection_, "INSERT INTO users(ip) VALUES('%1%');", client_ip);
		database_.execute(dbConnection_, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
	}

	auto res = database_.getLastQueryResults();
	if (res.empty()) {
		(SimpleLogger::Instance()).critical("Database error: " + database_.getLastErrorMessage());
		return;
	}
	(SimpleLogger::Instance()).info("Generating password...");
	const auto pass = generatePassword(8);
	database_.execute(dbConnection_, "UPDATE users SET password = '%1%' WHERE id = '%2%';", 
		std::to_string(std::hash<std::string>()(pass)), res[0]["id"]);
	sendToUser(res[0]["id"] + " " + pass);
}

void Session::storeCoordinates(const std::string& coordinates)
{
	const auto client_ip = ((socket_.remote_endpoint()).address()).to_string();

	UniqueSQLConnection uconn(dbConnection_);
	database_.execute(dbConnection_, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", coordinates, client_ip);
	if (database_.getLastErrorCode() != 0) {
		(SimpleLogger::Instance()).critical("Database error: " + database_.getLastErrorMessage());
	}
}

void Session::getFriendCoordinates(const std::string& friend_id, const std::string& friend_password)
{
	// (SimpleLogger::Instance()).info("Friend id: " + friend_id);
	// (SimpleLogger::Instance()).info("Friend pass: " + friend_password);

	UniqueSQLConnection uconn(dbConnection_);
	database_.execute(dbConnection_, "SELECT id FROM users WHERE id = '%1%' AND password = '%2%';",
		friend_id, std::to_string(std::hash<std::string>()(friend_password)));

	if (database_.getLastErrorCode() != 0) {
		(SimpleLogger::Instance()).critical(database_.getLastErrorMessage());
		return;
	}

	auto ans_id = database_.getLastQueryResults();
	if (ans_id.empty() || ans_id[0]["id"] != friend_id) {
		return;
	}

	database_.execute(dbConnection_, "SELECT coordinates FROM users WHERE id = '%1%';", friend_id);
	if (database_.getLastErrorCode() != 0) {
		(SimpleLogger::Instance()).critical(database_.getLastErrorMessage());
		return;
	}
	auto rcoord = database_.getLastQueryResults();

	if (rcoord.empty()) return;

	sendToUser(rcoord[0]["coordinates"]);
	(SimpleLogger::Instance()).debug("Friend coordinates: " + rcoord[0]["coordinates"] + " sent.");
}