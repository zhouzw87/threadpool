#pragma once
#include <thread>
#include <mutex>
#include "TaskQueue.h"
class ThreadPool
{
public:
	ThreadPool(int minNum, int maxNum);
	~ThreadPool();

	// 添加任务
	void addTask(Task task);
	// 获取忙线程的个数
	int getBusyNumber();
	// 获取活着的线程个数
	int getAliveNumber();
	//获取线程处理结果
	int getResult();

private:
	// 工作的线程的任务函数
	static void* worker(void* arg);
	// 管理者线程的任务函数
	static void* manager(void* arg);

private:
	TaskQueue* m_taskQ; // 任务队列

	std::mutex  m_mutexPool;// 锁整个的线程池
	std::mutex  m_mutexData;// 锁数据操作
	std::condition_variable  m_notEmpty;// 任务队列是不是空了
	std::vector<std::thread> m_threadIDs;// 工作的线程ID
	std::thread m_managerID;// 管理者线程ID

	int m_minNum;// 最小线程数量
	int m_maxNum;// 最大线程数量
	int m_busyNum; // 忙(工作)的线程的个数
	int m_aliveNum; // 活着的线程的个数
	int m_exitNum; // 要销毁的线程个数
	bool m_shutdown;// 是不是要销毁线程池, 销毁为1, 不销毁为0

	std::vector<int> m_results;

	int m_result;
};

