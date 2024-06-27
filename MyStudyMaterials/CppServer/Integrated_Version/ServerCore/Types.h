#pragma once
#include <mutex>
#include <atomic>

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;
using WCHAR = wchar_t;
template<typename T>
using Atomic = std::atomic<T>;
using Mutex = std::mutex;
using CondVar = std::condition_variable;
using UniqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::lock_guard<std::mutex>;
constexpr uint64 UINF=0xffff'ffff;
enum class ALIGNMENT
{
    SLIST_ALIGNMENT = 16,
};
/*			shared_ptr			*/
//template<typename T>
//using shared_ptr = std::shared_ptr<T>;
//
//#define USING_SHARED_PTR(name)	using name##Ref = std::shared_ptr<class name>;
//
//USING_SHARED_PTR(IocpService);
//USING_SHARED_PTR(ISession);
//USING_SHARED_PTR(Session);
//USING_SHARED_PTR(PacketSession);
//USING_SHARED_PTR(Listener);
//USING_SHARED_PTR(ServerService);
//USING_SHARED_PTR(ClientService);
//USING_SHARED_PTR(SendBuffer);
//USING_SHARED_PTR(SendBufferChunk);
//USING_SHARED_PTR(Job);
//USING_SHARED_PTR(JobQueue);


/*	Size Checker	*/
#define size16(val)		static_cast<int16>(sizeof(val))
#define size32(val)		static_cast<int32>(sizeof(val))
#define len16(arr)		static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)		static_cast<int32>(sizeof(arr)/sizeof(arr[0]))

//#define _STOMP_ALLOCATOR