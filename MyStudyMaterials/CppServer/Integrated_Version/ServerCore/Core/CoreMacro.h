#pragma once
#include <iostream>
#include "../Thread/Lock.h"


namespace ServerCore
{
#define OUT
// #define NAMESPACE_BEGIN(name)	namespace name {
// #define NAMESPACE_END			}

/*---------------
	  Lock
---------------*/

#define USE_MANY_LOCKS(count)	Lock locks[count]
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(locks[idx], typeid(this).name())
#define READ_LOCK				READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(locks[idx], typeid(this).name())
#define WRITE_LOCK				WRITE_LOCK_IDX(0)

/*---------------
	  Crash
---------------*/

#define CRASH(cause)						\
{											\
uint32* crash = nullptr;				\
__analysis_assume(crash != nullptr);	\
*crash = 0xDEADBEEF;					\
}

// expr이 false로 평가되면 CRASH 발생
#define ASSERT_CRASH(expr)			\
{									\
if (!(expr))					\
{								\
CRASH("ASSERT_CRASH");		\
__analysis_assume(expr);	\
}								\
}

// 디버그 모드에서 로깅 테스트
#ifdef _DEBUG
#define DEBUG_LOG(str) \
do { \
	std::cout << str << std::endl; \
} while(0)
#else
#define DEBUG_LOG(str) \
do { } while(0)
#endif

}