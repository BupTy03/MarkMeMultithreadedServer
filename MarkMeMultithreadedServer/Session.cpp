#include "Session.hpp"

#include <iostream>
#include <sstream>
#include <random>
#include <functional>
#include <regex>

#include <boost/bind.hpp>

void Session::start()
{
	std::cout << "Session is started..." << std::endl;
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
	std::cout << "receiveDataHandle()" << std::endl;
	if (error) {
		std::cerr << "Error " << error.message() << " while receiving data." << std::endl;
		return;
	}

	std::cout << "Received data: " << buffer_ << std::endl;

	std::istringstream is(buffer_.substr(0, bytes_tr));
	std::string header;
	is >> header;

	if (!dbConnection_.isConnected()) {
		dbConnection_.open();
	}

	if (header.compare("/init") == 0) {
		std::cout << "Initializing user..." << std::endl;
		initUser();
	}
	else if (header.compare("/store") == 0) {
		std::cout << "Storing coordinates..." << std::endl;
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
		std::cout << "Getting coordinates..." << std::endl;
		std::string friend_id;
		std::string friend_pass;

		is >> friend_id;
		is >> friend_pass;
	
		if (checkFriendFormat(friend_id, friend_pass)) {
			getFriendCoordinates(friend_id, friend_pass);
		}
		else {
			std::cerr << "Error: Wrong friend format" << std::endl;
		}
	}
	else if (header.compare("/store_and_get") == 0) {
		std::cout << "Storing and getting coordinates..." << std::endl;
		std::string coordinates;
		std::string friend_id;
		std::string friend_pass;

		is >> coordinates;
		is >> friend_id;
		is >> friend_pass;

		if (checkCoordinatesFormat(coordinates)) {
			storeCoordinates(coordinates);
			std::cout << "Coordinates: " << coordinates << " are stored." << std::endl;
		}
		else {
			std::cerr << "Error: Wrong coordinates format" << std::endl;
		}

		if (checkFriendFormat(friend_id, friend_pass)) {
			getFriendCoordinates(friend_id, friend_pass);
		}
		else {
			std::cerr << "Error: Wrong friend format" << std::endl;
		}
	}

	dbConnection_.close();
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

/*
void Session::handleWrite(const boost::system::error_code& error)
{
	if (error) {
		// log error
	}
}
*/

bool Session::checkCoordinatesFormat(const std::string& coordinates)
{
	std::regex reg("\\d+,\\d+\\s\\d+,\\d+");
	return std::regex_match(coordinates, reg);
}

bool Session::checkFriendFormat(const std::string& friend_id, const std::string& friend_password)
{
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
	std::cout << "Sending data: " << data << std::endl;
	boost::system::error_code ec;
	boost::asio::write(socket_, boost::asio::buffer(&data[0], data.size()), ec);
	if (ec) {
		std::cerr << "Error while sending data." << std::endl;
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

	database_.execute(dbConnection_, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
	auto r = database_.getLastQueryResults();
	if (r.empty() || r[0]["id"] == "NULL") {
		database_.execute(dbConnection_, "INSERT INTO users(ip) VALUES('%1%');", client_ip);
		database_.execute(dbConnection_, "SELECT id FROM users WHERE ip = '%1%';", client_ip);
	}
	auto res = database_.getLastQueryResults();
	if (res.empty()) {
		std::cerr << "Database error: " << database_.getLastErrorMessage() << std::endl;
		sendToUser("");
	}
	else {
		std::cout << "Generating password..." << std::endl;
		const auto pass = generatePassword(8);
		const auto hashed_pass = std::to_string(std::hash<std::string>()(pass));
		database_.execute(dbConnection_, "UPDATE users SET password = '%1%' WHERE id = '%2%';", hashed_pass, res[0]["id"]);
		sendToUser(res[0]["id"] + " " + pass);
	}
}

void Session::storeCoordinates(const std::string& coordinates)
{
	const auto client_ip = ((socket_.remote_endpoint()).address()).to_string();
	database_.execute(dbConnection_, "UPDATE users SET coordinates = '%1%' WHERE ip = '%2%';", coordinates, client_ip);
	sendToUser("");
}

void Session::getFriendCoordinates(const std::string& friend_id, const std::string& friend_password)
{
	database_.execute(dbConnection_, "SELECT id FROM users WHERE id = '%1%' AND password = '%2%;'",
		friend_id, std::to_string(std::hash<std::string>()(friend_password)));

	if (database_.getLastErrorCode() != 0) {
		sendToUser("");
		return;
	}

	auto ans_id = database_.getLastQueryResults();
	if (ans_id.empty() || ans_id[0]["id"] != friend_id) {
		sendToUser("");
		return;
	}

	database_.execute(dbConnection_, "SELECT coordinates FROM users WHERE id = '%1%';", friend_id);
	auto rcoord = database_.getLastQueryResults();
	if (!rcoord.empty()) {
		sendToUser(rcoord[0]["coordinates"]);
		std::cout << "Friend coordinates: " << rcoord[0]["coordinates"] << " sent." << std::endl;
	}
	else {
		sendToUser("");
	}
}