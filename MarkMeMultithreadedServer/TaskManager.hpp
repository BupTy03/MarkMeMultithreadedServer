#pragma once
#ifndef TASK_MANAGER_HPP
#define TASK_MANAGER_HPP

#include <vector>
#include <queue>
#include <thread>
#include <utility>
#include <mutex>
#include <functional>

namespace my
{

	class TaskManager
	{
	public:
		TaskManager(std::size_t count);
		virtual ~TaskManager();

		void addTask(const std::function<void()>& task);
		void addTask(std::function<void()>&& task);

		template<class... Args>
		void emplaceTask(Args... args);

	private:
		std::vector<std::thread> threads_;
		std::queue<std::function<void()>> tasks_;
		std::mutex mtx_;
		std::condition_variable cond_var_;
		bool stopped_{ false };
		std::function<bool()> has_tasks_or_stopped_;
	};

	TaskManager::TaskManager(std::size_t count) :
		has_tasks_or_stopped_([this]() {
		return !tasks_.empty() || stopped_;
	})
	{
		threads_.reserve(count);
		for (std::size_t i = 0; i < count; ++i) {
			threads_.emplace_back(
				[this]() {
				while (true) {
					std::function<void()> func;
					{
						std::unique_lock<std::mutex> lock(mtx_);
						cond_var_.wait(lock, has_tasks_or_stopped_);

						if (stopped_) {
							return;
						}

						func = std::move(tasks_.front());
						tasks_.pop();
					}
					func();
				}
			}
			);
		}
	}

	TaskManager::~TaskManager()
	{
		stopped_ = true;
		cond_var_.notify_all();
		for (auto& th : threads_) {
			th.join();
		}
	}

	void TaskManager::addTask(const std::function<void()>& task)
	{
		{
			std::unique_lock<std::mutex> lock(mtx_);
			tasks_.push(task);
		}
		cond_var_.notify_one();
	}

	void TaskManager::addTask(std::function<void()>&& task)
	{
		{
			std::unique_lock<std::mutex> lock(mtx_);
			tasks_.push(std::move(task));
		}
		cond_var_.notify_one();
	}

	template<class... Args>
	void TaskManager::emplaceTask(Args... args)
	{
		{
			std::unique_lock<std::mutex> lock(mtx_);
			tasks_.emplace(std::forward<Args>(args)...);
		}
		cond_var_.notify_one();
	}

}

#endif // !TASK_MANAGER_HPP
