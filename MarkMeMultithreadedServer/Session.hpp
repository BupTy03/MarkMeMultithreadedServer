#ifndef SESSION_HPP
#define SESSION_HPP

#define DEBUG

#include "SQLConnection.hpp"
#include "SQLDatabase.hpp"

#include <string>
#include <memory>

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using namespace boost::asio::ip;

class Session : public std::enable_shared_from_this<Session>
{
public:
	
	explicit Session(boost::asio::io_service& io_service, const std::string& filename)
		: socket_(io_service)
		, dbConnection_(filename)
	{}

	static constexpr std::size_t BUFFER_SIZE_IN_BYTES = 128;

	inline tcp::socket& socket() noexcept { return socket_; }

	void start();
	void receiveDataHandle(const boost::system::error_code&, std::size_t);

private:
	std::string generatePassword(int num);

	bool sendToUser(const std::string&);

	bool checkCoordinatesFormat(const std::string&);
	bool checkFriendFormat(const std::string&, const std::string&);

	void initUser();
	void storeCoordinates(const std::string&);
	void getFriendCoordinates(const std::string&, const std::string&);

private:
	tcp::socket socket_;
	std::string buffer_;
	SQLConnection dbConnection_;
	SQLDatabase database_;
};

#endif // !SESSION_HPP
