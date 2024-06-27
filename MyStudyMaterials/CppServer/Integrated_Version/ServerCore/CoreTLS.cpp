#include "Types.h"
#include "CoreTLS.h"
#include "SendBufferChunk.h"
#include "JobQueue.h"

thread_local uint32				tls_ThreadId = 0;
thread_local uint64				tls_EndTickCount = 0;
thread_local std::stack<int32>	tls_LockStack;
thread_local std::shared_ptr<SendBufferChunk> 	tls_SendBufferChunk;
thread_local JobQueue*			tls_CurrentJobQueue = nullptr;

