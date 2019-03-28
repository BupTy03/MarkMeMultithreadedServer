#pragma once
#ifndef MY_UTILS_HPP
#define MY_UTILS_HPP

#include <string>
#include <algorithm>

namespace my_utils
{

	template<class T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
	static void concatenate(std::string& str, T val) { str += std::to_string(val); }
	static void concatenate(std::string& str, char val) { str += val; }
	static void concatenate(std::string& str, const char* val) { str += val; }
	static void concatenate(std::string& str, const std::string& val) { str += val; }

	template<class Arg>
	static void format_str_impl(const std::string& fstr, std::string& result, std::string::const_iterator curr, const Arg& arg)
	{
		auto it = std::find(curr, std::cend(fstr), '?');
		if (it == std::cend(fstr)) {
			return;
		}
		if (it == std::cbegin(fstr) || *(it - 1) != '\\') {
			std::copy(curr, it, std::back_inserter(result));
			concatenate(result, arg);
			std::copy(it + 1, std::cend(fstr), std::back_inserter(result));
			return;
		}
		else {
			std::copy(curr, it - 1, std::back_inserter(result));
			result += *it;
		}
		format_str_impl(fstr, result, it + 1, arg);
	}

	template<class Arg, class... Args>
	static void format_str_impl(const std::string& fstr, std::string& result, std::string::const_iterator curr, const Arg& arg, Args&&... args)
	{
		auto it = std::find(curr, std::cend(fstr), '?');
		if (it == std::cend(fstr)) {
			return;
		}
		if (it == std::cbegin(fstr) || *(it - 1) != '\\') {
			std::copy(curr, it, std::back_inserter(result));
			concatenate(result, arg);
			format_str_impl(fstr, result, it + 1, std::forward<Args>(args)...);
		}
		else {
			std::copy(curr, it - 1, std::back_inserter(result));
			result += *it;
			format_str_impl(fstr, result, it + 1, arg, std::forward<Args>(args)...);
		}
	}

	template<class... Args>
	std::string format_str(const std::string& fstr, Args&&... args)
	{
		std::string result;
		result.reserve(std::size(fstr));
		format_str_impl(fstr, result, std::cbegin(fstr), std::forward<Args>(args)...);
		return result;
	}

}

#endif // !MY_UTILS_HPP