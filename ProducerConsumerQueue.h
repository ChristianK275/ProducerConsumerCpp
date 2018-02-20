#pragma once
#include <array>
#include <atomic>

template<typename T, unsigned LogSize=8>
class CProducerConsumerQueue
{
public:
	static const unsigned sz = 1 << LogSize;
	
	CProducerConsumerQueue() = default;

	bool Enqueue(const T& elem) noexcept
	{
		/* Single writer*/
		unsigned write = m_WritePos.load();
		unsigned new_write = (write + 1) & mask();
		unsigned read = m_ReadPos.load();

		if (new_write == read)
			return false;
		
		m_Data[write] = elem;
		m_WritePos.store(new_write);

		return true;
	}

	bool Dequeue(T& out) noexcept
	{
		for(;;)
		{
			unsigned write = m_WritePos.load();
			unsigned read = m_ReadPos.load();

			/* check if there's somthing in queue */
			if (write == read)
				return false;

			out = m_Data[read];

			unsigned new_read = (read + 1) & mask();
			/* keep going until only one thread reads the element */
			if (m_ReadPos.compare_exchange_strong(read, new_read))
				return true; 
		}
	}

private:
	static constexpr unsigned mask()
	{
		unsigned out = 0;
		unsigned i = LogSize;
		while (i-- > 0U)
			out |= 1U << i;
		return out;
	}

	std::atomic<unsigned> m_WritePos;
	std::atomic<unsigned> m_ReadPos;
	std::array<T, sz> m_Data;
};
