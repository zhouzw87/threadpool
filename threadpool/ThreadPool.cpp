#include "ThreadPool.h"
#include <iostream>
#include <string>
#include <condition_variable>
#include <windows.h>
ThreadPool::ThreadPool(int minNum, int maxNum)
{
	do 
	{
		// 实例化任务队列
		m_taskQ = new TaskQueue;
		// 初始化线程池
		m_minNum = minNum;
		m_maxNum = maxNum;
		m_busyNum = 0;
		m_aliveNum = minNum;
		m_exitNum = 0;
		m_shutdown = false;

		// 根据线程的最大上限给线程数组分配内存
		m_threadIDs.resize(maxNum);

		/////////////////// 创建线程 //////////////////
		// 创建管理者线程, 1个
		m_managerID = std::thread(manager, this);
		// 根据最小线程个数, 创建线程
		for (int i = 0; i < minNum; ++i)
		{
			m_threadIDs[i] = std::thread(worker, this);
		}

	} while (0);

}

ThreadPool::~ThreadPool()
{
	m_shutdown = true;
	// 阻塞回收管理者线程
	if (m_managerID.joinable()) {
		m_managerID.join();
	}
	// 唤醒所有消费者线程
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
	//添加任务
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
	// 一直不停的工作
	while (true)
	{
		std::cout << "liveNumber::::" << pool->getAliveNumber() << " busyNumber::::" << pool->getBusyNumber() << std::endl;
		// 访问任务队列(共享资源)加锁
		std::unique_lock<std::mutex> uk(pool->m_mutexPool);
		// 判断任务队列是否为空, 如果为空工作线程阻塞
		while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown)
		{
			// 阻塞线程
			std::cout << "thread-" << std::this_thread::get_id() << " waiting..." << std::endl;
			pool->m_notEmpty.wait(uk);

			// 解除阻塞之后, 判断是否要销毁线程
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
		// 判断线程池是否被关闭了
		if (pool->m_shutdown)
		{
			std::cout << "thread-" << std::this_thread::get_id() << "exit......" << std::endl;
			uk.unlock();
			return nullptr;
		}

		// 从任务队列中取出一个任务
		Task task = pool->m_taskQ->takeTask();
		// 线程池解锁
		uk.unlock();

		// 工作的线程+1
		pool->m_mutexData.lock();
		pool->m_busyNum++;
		pool->m_mutexData.unlock();

		// 执行任务
		std::cout << "thread-" << std::this_thread::get_id() << " start working..." ;
		int res = task.fuction(task.arg);
		delete task.arg;
		task.arg = nullptr;

		// 任务处理结束
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
	// 如果线程池没有关闭, 就一直检测
	while (!pool->m_shutdown)
	{
		// 每隔5s检测一次
		Sleep(5);
		// 取出线程池中的任务数和线程数量
		//  取出工作的线程池数量
		pool->m_mutexData.lock();
		int queueSize = pool->m_taskQ->taskNumber();
		int liveNum = pool->m_aliveNum;
		int busyNum = pool->m_busyNum;
		pool->m_mutexData.unlock();

		// 创建线程
		const int NUMBER = 2;
		// 当前任务个数>存活的线程数 && 存活的线程数<最大线程个数
		if (queueSize > liveNum && liveNum < pool->m_maxNum)
		{
			// 线程池加锁
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

		// 销毁多余的线程
		// 忙线程*2 < 存活的线程数目 && 存活的线程数 > 最小线程数量
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