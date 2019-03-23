#include "TasksManager.hpp"

#include <algorithm>

void TasksManager::addWorker()
{
	workers_.emplace_back();
}
void TasksManager::addWorkers(std::size_t num)
{
	workers_.resize(workers_.size() + num);
}

void TasksManager::addTask(std::function<void()> task)
{
	auto it = std::min_element(std::begin(workers_), std::end(workers_));
	it->addTask(std::move(task));
}