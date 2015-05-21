#pragma once
#include <functional>
#include <Windows.h>
#include <vector>
#include "JobHandle.h"
#include "JobType.h"
#include "settings.h"

namespace tt{
class IJob{
public:
	virtual void run() = 0;
	virtual ~IJob(){}
};

//Template job to allow either Functors, Functions, or lambda expressions to be passed as jobs
template<typename FUNC>
class FuncJob : public IJob{
private:
	FUNC m_Func;
public:
	FuncJob(FUNC f) : m_Func(f){}
	virtual void run(){
		m_Func();
	}
};

class JobManager{
private:
	//Maximum number of actively queued jobs
	static const unsigned int kMaxQueuedJobs=1024;

	//Thread management
	int m_ThreadCount;
	HANDLE* m_ThreadHandles;
	struct ThreadParams{
		JobManager* jobManager;
		unsigned int threadId;
	};
	ThreadParams* m_ThreadParams;

	//Job list
	std::vector<JobHandle*> m_JobQueue;
	CRITICAL_SECTION m_JobQueueCritSec;
	HANDLE m_JobsAvailableSemaphore;
	HANDLE m_QuitEvent;

	//User defined class to handle log messages
#if TT_JOBSYS_PROFILING_ON
	class IJobSystemLogger* m_Logger;
	CRITICAL_SECTION m_LoggerCritSec;
#endif

	static DWORD WINAPI ThreadProc(LPVOID lpParam);
	void runThread(unsigned int threadId);
	JobHandle* _createHandle();
	void _enqueueHandle(JobHandle* handle);
public:
	JobManager();
	~JobManager();
	
	void initialize(int threadCount);
	void initialize();

	void setLogger(class IJobSystemLogger* logger);

	void spawn(JobGroup* group);
	void _notifyGroupReady(JobGroup* group);

	//Informative
	//Returns the maximum number of threads available by this job manager
	int maxConcurrency() const;
};
};