#pragma once
#ifndef DATA_BASE_HPP
#define DATA_BASE_HPP

#include <map>
#include <mutex>

namespace my
{

	class ConcurrentMap
	{
	public:
		ConcurrentMap(){}

		ConcurrentMap(const ConcurrentMap& other) : records_{other.records_} {}
		ConcurrentMap& operator=(const ConcurrentMap& other) { this->records_ = other.records_; }

		ConcurrentMap(ConcurrentMap&& other) : records_{ std::move(other.records_) } {}
		ConcurrentMap& operator=(ConcurrentMap&& other) { std::swap(this->records_, other.records_); }

		void setValue(const std::string& key, const std::string& value)
		{
			std::unique_lock<std::mutex> lock(mtx_);
			records_[key] = value;
		}

		// Throws exception: std::out_of_range
		const std::string& getValue(const std::string& key) const
		{
			std::unique_lock<std::mutex> lock(mtx_);
			std::string val = records_.at(key);
			return val;
		}

		const std::string& getValue(const std::string& key, bool& exists) const
		{
			std::unique_lock<std::mutex> lock(mtx_);

			auto it = records_.find(key);
			exists = (it != records_.end());

			std::string result;
			if (exists)
				result = it->second;

			return result;
		}

		bool containsKey(const std::string& key) const
		{
			std::unique_lock<std::mutex> lock(mtx_);
			bool cont = (records_.find(key) != records_.end());
			return cont;
		}

	private:
		std::map<std::string, std::string> records_;
		mutable std::mutex mtx_;
	};

}

#endif // !DATA_BASE_HPP
