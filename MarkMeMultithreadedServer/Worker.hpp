#ifndef WORKER_HPP
#define WORKER_HPP

#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

struct Worker
{
	Worker();
	virtual ~Worker();

	Worker(const Worker& other);
	Worker& operator=(const Worker& other);

	Worker(Worker&& other) noexcept;
	Worker& operator=(Worker&& other) noexcept;

	void addTask(std::function<void()>);
	inline std::size_t countTasks() const { return tasks_.size(); }

	friend bool operator==(const Worker& first, const Worker& second);
	friend bool operator!=(const Worker& first, const Worker& second);

	friend bool operator>(const Worker& first, const Worker& second);
	friend bool operator<(const Worker& first, const Worker& second);

	friend bool operator>=(const Worker& first, const Worker& second);
	friend bool operator<=(const Worker& first, const Worker& second);

private:
	std::queue<std::function<void()>> tasks_;
	mutable std::mutex mtx_;
	std::condition_variable cv_;
	bool stopped_{ false };
	std::thread thread_;
};

#endif // !WORKER_HPP