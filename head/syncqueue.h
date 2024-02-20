#pragma once

#include <queue>

namespace core
{
	template<typename T>
	class TSyncQueue
	{
		std::queue<T>		m_queItems;

	public:
		TSyncQueue(void)
			: m_queItems()
		{			
			Create();
		}

		~TSyncQueue(void)
		{
			Destroy();
		}

		ECODE Create(void)
		{
			return EC_SUCCESS;
		}

		void Destroy(void)
		{
			
		}

		bool IsEmpty(void)
		{
			return true;
		}
		
		size_t Count(void)
		{
			return 0;
		}

		void Clear(void)
		{
			
		}

		void Push(T inItem)
		{
			
		}

		EWAIT Pop(T* pOutItem)
		{
			EWAIT nRet;
				return nRet;
		}
	};
}