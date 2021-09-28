#include "ThreadPool.h"
#include <iostream>
#include <string>
#include <condition_variable>
#include <windows.h>
ThreadPool::ThreadPool(int minNum, int maxNum)
{
	do 
	{
		// ʵ�����������
		m_taskQ = new TaskQueue;
		// ��ʼ���̳߳�
		m_minNum = minNum;
		m_maxNum = maxNum;
		m_busyNum = 0;
		m_aliveNum = minNum;
		m_exitNum = 0;
		m_shutdown = false;

		// �����̵߳�������޸��߳���������ڴ�
		m_threadIDs.resize(maxNum);

		/////////////////// �����߳� //////////////////
		// �����������߳�, 1��
		m_managerID = std::thread(manager, this);
		// ������С�̸߳���, �����߳�
		for (int i = 0; i < minNum; ++i)
		{
			m_threadIDs[i] = std::thread(worker, this);
		}

	} while (0);

}

ThreadPool::~ThreadPool()
{
	m_shutdown = true;
	// �������չ������߳�
	if (m_managerID.joinable()) {
		m_managerID.join();
	}
	// ���������������߳�
	m_notEmpty.notify_all();

	for (int i = 0; i < m_maxNum; ++i)
	{
		if (m_threadIDs[i].joinable()) {
			m_threadIDs[i].join();
		}
	}

	if (m_taskQ) delete m_taskQ;
}

void ThreadPool::addTask(Task task)
{
	if (m_shutdown)
	{
		return;
	}
	//�������
	m_taskQ->addTask(task);
	m_notEmpty.notify_all();
}

int ThreadPool::getBusyNumber()
{
	int busyNum = 0;
	m_mutexData.lock();
	busyNum = m_busyNum;
	m_mutexData.unlock();
	return busyNum;
}

int ThreadPool::getAliveNumber()
{
	int threadNum = 0;
	m_mutexData.lock();
	threadNum = m_aliveNum;
	m_mutexData.unlock();
	return threadNum;
}

int ThreadPool::getResult()
{
	int res = 0;
	m_mutexData.lock();
	res = m_result;
	m_mutexData.unlock();
	return res;
}

void * ThreadPool::worker(void * arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	// һֱ��ͣ�Ĺ���
	while (true)
	{
		std::cout << "liveNumber::::" << pool->getAliveNumber() << " busyNumber::::" << pool->getBusyNumber() << std::endl;
		// �����������(������Դ)����
		std::unique_lock<std::mutex> uk(pool->m_mutexPool);
		// �ж���������Ƿ�Ϊ��, ���Ϊ�չ����߳�����
		while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown)
		{
			// �����߳�
			std::cout << "thread-" << std::this_thread::get_id() << " waiting..." << std::endl;
			pool->m_notEmpty.wait(uk);

			// �������֮��, �ж��Ƿ�Ҫ�����߳�
			if (pool->m_exitNum > 0)
			{
				pool->m_exitNum--;
				if (pool->m_aliveNum > pool->m_minNum)
				{
					pool->m_aliveNum--;
					uk.unlock();
					return nullptr;
				}
			}
		}
		// �ж��̳߳��Ƿ񱻹ر���
		if (pool->m_shutdown)
		{
			std::cout << "thread-" << std::this_thread::get_id() << "exit......" << std::endl;
			uk.unlock();
			return nullptr;
		}

		// �����������ȡ��һ������
		Task task = pool->m_taskQ->takeTask();
		// �̳߳ؽ���
		uk.unlock();

		// �������߳�+1
		pool->m_mutexData.lock();
		pool->m_busyNum++;
		pool->m_mutexData.unlock();

		// ִ������
		std::cout << "thread-" << std::this_thread::get_id() << " start working..." ;
		int res = task.fuction(task.arg);
		delete task.arg;
		task.arg = nullptr;

		// ���������
		std::cout << "thread-" << std::this_thread::get_id() << " end working..." << std::endl;
		pool->m_mutexData.lock();
		pool->m_result = res;
		pool->m_results.push_back(res);
		pool->m_busyNum--;
		pool->m_mutexData.unlock();
	}

	return nullptr;
}

void * ThreadPool::manager(void * arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	// ����̳߳�û�йر�, ��һֱ���
	while (!pool->m_shutdown)
	{
		// ÿ��5s���һ��
		Sleep(5);
		// ȡ���̳߳��е����������߳�����
		//  ȡ���������̳߳�����
		pool->m_mutexData.lock();
		int queueSize = pool->m_taskQ->taskNumber();
		int liveNum = pool->m_aliveNum;
		int busyNum = pool->m_busyNum;
		pool->m_mutexData.unlock();

		// �����߳�
		const int NUMBER = 2;
		// ��ǰ�������>�����߳��� && �����߳���<����̸߳���
		if (queueSize > liveNum && liveNum < pool->m_maxNum)
		{
			// �̳߳ؼ���
			pool->m_mutexData.lock();
			int num = 0;
			for (int i = 0; i < pool->m_maxNum && num < NUMBER
				&& pool->m_aliveNum < pool->m_maxNum; ++i)
			{
				if (pool->m_threadIDs[i].get_id() == std::thread::id())
				{
					pool->m_threadIDs[i] = std::thread(worker, pool);
					num++;
					pool->m_aliveNum++;
				}
			}
			pool->m_mutexData.unlock();
		}

		// ���ٶ�����߳�
		// æ�߳�*2 < �����߳���Ŀ && �����߳��� > ��С�߳�����
		if (busyNum * 2 < liveNum && liveNum > pool->m_minNum)
		{
			pool->m_mutexData.lock();
			pool->m_exitNum = NUMBER;
			pool->m_mutexData.unlock();
			for (int i = 0; i < NUMBER; ++i)
			{
				pool->m_notEmpty.notify_all();
			}
		}
	}
	return nullptr;
}