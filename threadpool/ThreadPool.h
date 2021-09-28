#pragma once
#include <thread>
#include <mutex>
#include "TaskQueue.h"
class ThreadPool
{
public:
	ThreadPool(int minNum, int maxNum);
	~ThreadPool();

	// �������
	void addTask(Task task);
	// ��ȡæ�̵߳ĸ���
	int getBusyNumber();
	// ��ȡ���ŵ��̸߳���
	int getAliveNumber();
	//��ȡ�̴߳�����
	int getResult();

private:
	// �������̵߳�������
	static void* worker(void* arg);
	// �������̵߳�������
	static void* manager(void* arg);

private:
	TaskQueue* m_taskQ; // �������

	std::mutex  m_mutexPool;// ���������̳߳�
	std::mutex  m_mutexData;// �����ݲ���
	std::condition_variable  m_notEmpty;// ��������ǲ��ǿ���
	std::vector<std::thread> m_threadIDs;// �������߳�ID
	std::thread m_managerID;// �������߳�ID

	int m_minNum;// ��С�߳�����
	int m_maxNum;// ����߳�����
	int m_busyNum; // æ(����)���̵߳ĸ���
	int m_aliveNum; // ���ŵ��̵߳ĸ���
	int m_exitNum; // Ҫ���ٵ��̸߳���
	bool m_shutdown;// �ǲ���Ҫ�����̳߳�, ����Ϊ1, ������Ϊ0

	std::vector<int> m_results;

	int m_result;
};

