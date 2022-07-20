#include "pch.h"
#include "MemoryManager.h"
#include "MemoryPool.h"

/*-------------
	MemoryManager
---------------*/

MemoryManager::MemoryManager()
{
	int32 size = 0;
	int32 tableIndex = 0;

	/* �� ����� �޸� Ǯ ����
	
	- 32 byte, 64 byte, 96 byte, 128 byte... 1024 byte ���� ��������
	 MemoryManager Pool���� ���� �����Ѵ�.
	 
	 �������
	 32 byte ���� ������ MemoryManager Pool�� 1~32 byte ���� ũ���� �޸𸮸� �Ҵ����ְ�,
	 64 byte ���� ������ MemoryManager Pool�� 33~64 byte ���� ũ���� �޸𸮸� �Ҵ����ִ� ����̴�.
	*/
	for (size = 32; size <= 1024; size += 32) // �޸� Ǯ ������ 32�� ����
	{
		MemoryPool* pool = new MemoryPool(size);
		pools.push_back(pool);

		while (tableIndex <= size)
		{
			poolTable[tableIndex] = pool;
			tableIndex++; 
			/* ����) poolTable[0~32]�� 32 bytes¥�� MemoryPool�� ���� 
					  poolTable[33~64]�� 64 bytes¥�� MemoryPool�� ���� */
		}
	}

	for (; size <= 2048; size += 128) // �޸� Ǯ ������ 128�� ����
	{
		MemoryPool* pool = new MemoryPool(size);
		pools.push_back(pool);

		while (tableIndex <= size)
		{
			poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	for (; size <= MAX_ALLOC_SIZE; size += 256) // �޸� Ǯ ������ 256�� ����
	{
		MemoryPool* pool = new MemoryPool(size);
		pools.push_back(pool);

		while (tableIndex <= size)
		{
			poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}
}

MemoryManager::~MemoryManager()
{
	for (MemoryPool* pool : pools)
		delete pool; // MemoryPool �Ҹ��� ȣ��

	pools.clear();
}

void* MemoryManager::Allocate(int32 size)
{
	MemoryHeader* header = nullptr;
	const int32 allocSize = size + sizeof(MemoryHeader);// �޸� ����� ���̱� ���� �Ҵ� ����� ���� ũ�� ������

#ifdef _STOMP_ALLOCATOR
	header = reinterpret_cast<MemoryHeader*>(StompAllocator::AllocateMemory(allocSize));
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{// �޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� �Ҵ�
		
		header = reinterpret_cast<MemoryHeader*>(
			::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		// �޸� Ǯ���� �����´�
		header = poolTable[allocSize]->Pop();
	}
#endif	

	return MemoryHeader::AttachHeader(header, allocSize);// MemoryHeader�޺κ��� �ǻ�� ������ �ּ� ��ȯ
}

void MemoryManager::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 allocSize = header->allocSize;
	ASSERT_CRASH(allocSize > 0);

#ifdef _STOMP_ALLOCATOR
	StompAllocator::ReleaseMemory(header);
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{
		// �޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� ����
		::_aligned_free(header);
	}
	else
	{
		// �޸� Ǯ�� �ݳ��Ѵ�
		poolTable[allocSize]->Push(header);
	}
#endif	
}