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
	// ��ȡ��ǰ�������������
	inline int taskNumber()
	{
		return (int)m_queue.size();
	}
private:
	std::mutex m_mutex;
	std::queue<Task> m_queue;
};

