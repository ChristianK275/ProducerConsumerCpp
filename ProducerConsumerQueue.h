#pragma once
#include <array>
#include <atomic>

template<typename T, unsigned Size>
class CProducerConsumerQueue
{
public:
	static const unsigned size = Size;
	
	CProducerConsumerQueue() = default;

	bool Enqueue(const T& elem) noexcept
	{
		/* Single writer*/
		unsigned write = m_WritePos.load();
		unsigned new_write = Increment(write);

		if (new_write == m_ReadPos.load())
			return false;
		
		m_Data[write] = elem;
		m_WritePos.store(new_write);

		return true;
	}

	bool Dequeue(T& out) noexcept
	{

		/* critical section, increment the read pos */
		unsigned write = m_WritePos.load();
		
		static bool released = false;
		while (m_ConsumerLock.compare_exchange_strong(released, true)) {}
		unsigned read = m_ReadPos.load();

		bool not_empty = read != write;
		if(not_empty)
		{
			out = m_Data[read];
			unsigned new_read = Increment(read);
			m_ReadPos.store(new_read);
		}

		m_ConsumerLock.store(released);
		return not_empty;
	}

private:

	static unsigned Increment(unsigned i) noexcept
	{
		return ++i == size ? 0 : i;
	}

	std::array<T, size> m_Data;
	std::atomic<unsigned> m_WritePos{ 0 };
	std::atomic<unsigned> m_ReadPos{ 0 };
	std::atomic<bool> m_ConsumerLock{ false };
};
