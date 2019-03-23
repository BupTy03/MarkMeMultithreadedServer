#include "Worker.hpp"

Worker::Worker()
	: thread_(
		[this]()
{
	while (true) {
		std::unique_lock<std::mutex> lk(mtx_); // lock
		cv_.wait(lk, [this] { return stopped_ || !tasks_.empty(); });

		if (stopped_) {
			return;
		}

		std::function<void()> tsk(std::move(tasks_.front()));
		tasks_.pop();

		lk.unlock(); // unlock

		tsk(); // execute task;
	}
}
)
{}

Worker::Worker(const Worker& other)
{
	std::unique_lock<std::mutex> lk(this->mtx_);
	std::unique_lock<std::mutex> lk2(other.mtx_);
	this->tasks_ = other.tasks_;
}
Worker& Worker::operator=(const Worker& other)
{
	if (this == &other) {
		return *this;
	}
	std::unique_lock<std::mutex> lk(this->mtx_);
	std::unique_lock<std::mutex> lk2(other.mtx_);
	this->tasks_ = other.tasks_;
	return *this;
}

Worker::Worker(Worker&& other) noexcept
{
	std::unique_lock<std::mutex> lk(this->mtx_);
	std::unique_lock<std::mutex> lk2(other.mtx_);
	std::swap(this->tasks_, other.tasks_);
}
Worker& Worker::operator=(Worker&& other) noexcept
{
	if (this == &other) {
		return *this;
	}
	std::unique_lock<std::mutex> lk(this->mtx_);
	std::unique_lock<std::mutex> lk2(other.mtx_);
	std::swap(this->tasks_, other.tasks_);
	return *this;
}

Worker::~Worker()
{
	{
		std::lock_guard<std::mutex> lk(mtx_);
		stopped_ = true;
		cv_.notify_all();
	}

	if (thread_.joinable()) {
		thread_.join();
	}
}

void Worker::addTask(std::function<void()> f)
{
	std::unique_lock<std::mutex> lk(mtx_);

	const bool empty = tasks_.empty();
	tasks_.push(std::move(f));

	if (empty) {
		cv_.notify_all();
	}
}

bool operator==(const Worker& first, const Worker& second)
{
	return first.tasks_.size() == second.tasks_.size();
}
bool operator!=(const Worker& first, const Worker& second)
{
	return first.tasks_.size() != second.tasks_.size();
}

bool operator>(const Worker& first, const Worker& second)
{
	return first.tasks_.size() > second.tasks_.size();
}
bool operator<(const Worker& first, const Worker& second)
{
	return first.tasks_.size() < second.tasks_.size();
}

bool operator>=(const Worker& first, const Worker& second)
{
	return first.tasks_.size() >= second.tasks_.size();
}
bool operator<=(const Worker& first, const Worker& second)
{
	return first.tasks_.size() <= second.tasks_.size();
}