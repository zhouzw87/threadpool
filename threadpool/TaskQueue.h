#pragma once
#include <queue>
#include <thread>
#include <mutex>

using Callback = int(*)(void*);
struct Task
{
	Task()
	{
		fuction = nullptr;
		arg = nullptr;
	}
	Task(Callback f, void* a)
	{
		fuction = f;
		arg = a;
	}
	Callback fuction;
	void *arg;
};
class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();
	void addTask(Task task);
	Task takeTask();
	// 获取当前队列中任务个数
	inline int taskNumber()
	{
		return (int)m_queue.size();
	}
private:
	std::mutex m_mutex;
	std::queue<Task> m_queue;
};

