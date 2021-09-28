#include "TaskQueue.h"

TaskQueue::TaskQueue()
{
}

TaskQueue::~TaskQueue()
{
}

void TaskQueue::addTask(Task task)
{
	m_mutex.lock();
	m_queue.push(task);
	m_mutex.unlock();
}

Task TaskQueue::takeTask()
{
	Task task;
	m_mutex.lock();
	if (m_queue.size()>0)
	{
		task = m_queue.front();
		m_queue.pop();
	}

	m_mutex.unlock();
	return task;
}
