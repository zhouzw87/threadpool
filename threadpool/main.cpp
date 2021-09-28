#include "ThreadPool.h"
#include <iostream>
#include <windows.h>
int taskFunc(void* arg)
{
	Sleep(50);
	int num = *(int*)arg;
	num++;
	if (num >=40) {
		std::cout << "thread-" << std::this_thread::get_id() << " is working, number is........: " << num << std::endl;
	}
	else {
		std::cout << "thread-" << std::this_thread::get_id() << " is working, number is: " << num << std::endl;
	}
	return num;
}

int main()
{
	// 创建线程池
	ThreadPool pool(8, 16);
	for (int i = 1; i <= 30; ++i)
	{
		Sleep(5);
		int* num = new int(i + 10);
		Task task = Task(taskFunc, num);
		pool.addTask(task);
		std::cout << "m_result<<<<<<<<<<<: " << pool.getResult();
		//std::cout << "live-thread: " << pool.getAliveNumber() << " busy-thread:  " << pool.getBusyNumber() << std::endl;
	}
	Sleep(100);
	//Sleep(30000);
	return 0;
}

