#ifndef TASKS_MANAGER_HPP
#define TASKS_MANAGER_HPP

#include "Worker.hpp"

#include <vector>
#include <functional>

struct TasksManager
{
	TasksManager(std::size_t num = 1)
		: workers_{num}
	{}

	void addWorker();
	void addWorkers(std::size_t num);

	void addTask(std::function<void()> task);

private:
	std::vector<Worker> workers_;
};

#endif // !TASKS_MANAGER_HPP