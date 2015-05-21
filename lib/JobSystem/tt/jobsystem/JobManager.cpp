#include "JobManager.h"
#include <assert.h>
#include <stdio.h>
#include "settings.h"
#include "IJobSystemLogger.h"
#include "JobGroup.h"

namespace tt{
DWORD WINAPI JobManager::ThreadProc(LPVOID lpParam){
	assert(lpParam != nullptr);
	const ThreadParams& params = *reinterpret_cast<ThreadParams*>(lpParam);
	params.jobManager->runThread(params.threadId);
	return 0;
}

JobManager::JobManager() :
	m_ThreadCount(0)
	,m_ThreadHandles(0)
#if TT_JOBSYS_PROFILING_ON 
	,m_Logger(nullptr)
#endif
{
}
JobManager::~JobManager(){
	SetEvent(m_QuitEvent);

	//Wait for all threads to close before destroying job manager
	WaitForMultipleObjects(m_ThreadCount, m_ThreadHandles, true, INFINITE);
	DeleteCriticalSection(&m_JobQueueCritSec);
	for(int i=0; i<m_ThreadCount; ++i){
		CloseHandle(m_ThreadHandles[i]);
	}

	CloseHandle(m_QuitEvent);
	CloseHandle(m_JobsAvailableSemaphore);

#if TT_JOBSYS_PROFILING_ON
	DeleteCriticalSection(&m_LoggerCritSec);
#endif

}

void JobManager::initialize(){
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	initialize(systemInfo.dwNumberOfProcessors);
}
void JobManager::initialize(int threadCount){
	m_ThreadCount = threadCount;
	m_ThreadHandles = new HANDLE[m_ThreadCount];
	m_ThreadParams = new ThreadParams[m_ThreadCount];

	//Spawn threads
	InitializeCriticalSection(&m_JobQueueCritSec);
	m_JobsAvailableSemaphore = CreateSemaphore(nullptr, 0, kMaxQueuedJobs, nullptr);
	m_QuitEvent = CreateEvent(nullptr, true, false, nullptr);
	for(int i=0; i<m_ThreadCount; ++i){
		m_ThreadParams[i].jobManager = this;
		m_ThreadParams[i].threadId = i;
		m_ThreadHandles[i] =  CreateThread(nullptr, 0, &JobManager::ThreadProc, &m_ThreadParams[i], 0, nullptr);
	}

#if TT_JOBSYS_PROFILING_ON
	InitializeCriticalSection(&m_LoggerCritSec);
#endif
}

void JobManager::runThread(unsigned int threadId){
	const HANDLE waitHandles[2] = {
		m_JobsAvailableSemaphore,
		m_QuitEvent
	};
	while(true){
		//Wait for jobs to become available
		const int result = WaitForMultipleObjects(2, waitHandles, false, INFINITE);
		if(result == WAIT_OBJECT_0+1) return;

#if TT_JOBSYS_PROFILING_ON
		LARGE_INTEGER profileJobStartTime;
		QueryPerformanceCounter(&profileJobStartTime);
#endif

		//Claim the next job
		EnterCriticalSection(&m_JobQueueCritSec);
		assert(!m_JobQueue.empty());
		JobHandle* jobHandle  =  m_JobQueue.back();
		m_JobQueue.pop_back();
		LeaveCriticalSection(&m_JobQueueCritSec);

		//run actual task
		jobHandle->job->run();
		if(jobHandle->group) jobHandle->group->_notifyChildDone();

#if TT_JOBSYS_PROFILING_ON
		LARGE_INTEGER profileJobEndTime;
		QueryPerformanceCounter(&profileJobEndTime);
		EnterCriticalSection(&m_LoggerCritSec);
		if(m_Logger) m_Logger->addJob(*jobHandle->group, threadId, profileJobStartTime, profileJobEndTime);
		LeaveCriticalSection(&m_LoggerCritSec);
#endif
	}
}
void JobManager::_enqueueHandle(JobHandle* handle){
	EnterCriticalSection(&m_JobQueueCritSec);
	m_JobQueue.push_back(handle);
	LeaveCriticalSection(&m_JobQueueCritSec);

	BOOL result = ReleaseSemaphore(m_JobsAvailableSemaphore, 1, nullptr);
	assert(result); //assertion fails when semaphore limit is exceeded
}
void JobManager::spawn(JobGroup* group){
	EnterCriticalSection(&group->m_DependencyCritSec);
	group->m_JobManager = this;
	ResetEvent(group->m_DoneEvent);
	if(group->m_DependenciesLeft== 0){
		_notifyGroupReady(group);
	}
	LeaveCriticalSection(&group->m_DependencyCritSec);
}

int JobManager::maxConcurrency() const{
	return m_ThreadCount;
}
void JobManager::setLogger(IJobSystemLogger* logger){
#if TT_JOBSYS_PROFILING_ON
	m_Logger = logger;
#endif
}
void JobManager::_notifyGroupReady(JobGroup* group){
	for(int i=0; i<group->m_JobHandles.size(); ++i){
		_enqueueHandle(&group->m_JobHandles[i]);
	}
}
};//namespace tt