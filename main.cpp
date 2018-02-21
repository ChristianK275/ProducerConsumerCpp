#include "ProducerConsumerQueue.h"
#include <thread>
#include <iostream>
#include <mutex>

static bool quit = false;

CProducerConsumerQueue<int*, 32> q;
std::mutex print_mutex;

template<typename ...Args>
void PrintSynced(const char* fmt, Args... args)
{
	std::lock_guard<std::mutex> lock(print_mutex);
	std::printf(fmt, args...);
}

void Producer()
{
	int i = 0;
	int* p = nullptr;

	while(!quit)
	{
		if (!p)
			p = new int(i++);
		if (q.Enqueue(p))
		{
			//PrintSynced("Enqueue %i\n", i);
			p = nullptr;
		}
	}
}

void Consumer(size_t id)
{
	while(!quit)
	{
		int* i;
		if(q.Dequeue(i))
			delete i;
	}
}

int main()
{
	std::thread p1(Producer);

	std::array<std::thread, 3> consumers;
	size_t thread_id = 0;
	for(size_t i = 0; i < consumers.size(); ++i)
		consumers[i] = std::thread(Consumer, thread_id++);

	p1.join();

	for (std::thread& t : consumers)
		t.join();

	return 0;
}