#pragma once
#include "Allocator.h"

class MemoryPool;

//-----------
//	Memory
//-----------

class Memory
{
	enum
	{
		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (1024 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void*	Allocate(int32 size);
	void	Release(void* ptr);

private:
	vector<MemoryPool*> _pools;

	// 메모리 크기 <-> 메모리 풀
	// O(1)을 위한 테이블
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};


template<typename Type, typename... Args>
Type* Xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	//placement new
	new(memory)Type(forward<Args>(args)...);

	return memory;
}

template<typename Type>
void Xdelete(Type* obj)
{
	obj->~Type();
	PoolAllocator::Release(obj);
}

template<typename Type>
shared_ptr<Type> MakeShared()
{
	return shared_ptr<Type>{Xnew<Type>(), Xdelete<Type>};
}