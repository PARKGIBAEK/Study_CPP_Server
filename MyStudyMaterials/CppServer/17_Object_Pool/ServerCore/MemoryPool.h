#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

/*-----------------
	MemoryHeader
------------------*/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	// [MemoryHeader][Data]
	MemoryHeader(int32 size) : allocSize(size) { }

	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size); // placement new : header��ġ�� MemoryHeader�� �����ڸ� ȣ���Ͽ� �Ҵ�
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 allocSize;
	// TODO : �ʿ��� �߰� ����
};

/*-----------------
	MemoryPool
------------------*/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	// �޸� Ǯ�� ��ȯ�ϱ�(�־��ֱ�)
	void			Push(MemoryHeader* ptr);
	// �޸� Ǯ���� ��������
	MemoryHeader*	Pop();

private:
	SLIST_HEADER	_header;//�޸� Ǯ�� ���� �����̳�(SLIST_HEADER�� MS�翡�� ���� Lock-Free Stack�� ���� ����̴�, ���ο��� ���Ǵ� ���� SLIST_ENTRY�� ����Ѵ�)
	int32			_allocSize = 0;
	atomic<int32>	_useCount = 0;//�޸� Ǯ���� ������ ��� ���� ��ü�� ����
	atomic<int32>	_reserveCount = 0;// �޸� Ǯ���� ������ ��ü�� ����
};
