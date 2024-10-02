#pragma once

//------------------
//		Lock
//------------------

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1);
#define READ_LOCK_INDEX(index)	ReadLockGuard readLockGuard_##index(_locks[index], typeid(this).name());
#define READ_LOCK				READ_LOCK_INDEX(0);
#define WRITE_LOCK_INDEX(index)	WriteLockGuard writeLockGuard_##index(_locks[index], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_INDEX(0);

//------------------
//		Crash
//------------------

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	/*crash != nullptr 라고 가정해라*/		\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xBADEDCFE;					\
}

//조건을 만족하지 않으면 크래시
#define ASSERT_CRASH(expr)					\
{											\
	if(!expr)								\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}