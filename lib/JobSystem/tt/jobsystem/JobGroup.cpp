#include "JobGroup.h"
#include "JobManager.h"
#include <mutex>

namespace tt{
JobGroup::JobGroup() :
	m_DependencyCount(0),
	m_JobManager(nullptr),
	m_JobsRemaining(0),
	m_DependenciesLeft(0)
{
	_init();
}
JobGroup::JobGroup(const char* name, int reserve) :
	m_DependencyCount(0),
	m_JobManager(nullptr),
	m_JobsRemaining(0),
	m_DependenciesLeft(0),
	m_Name(name)
{
	m_JobHandles.reserve(reserve);
	_init();
}
JobGroup::~JobGroup(){
	_destroy();
}
void JobGroup::_init(){
	m_DoneEvent = CreateEvent(nullptr, true, true, nullptr); //Manual reset: Should always return when not in the manager
	InitializeCriticalSection(&m_DependencyCritSec);
}
void JobGroup::_destroy(){
	waitUntilDone();
	for(int i=0; i<m_JobHandles.size(); ++i){
		if(m_JobHandles[i].flags & JobHandle::kDestroysJob){
			delete m_JobHandles[i].job;
		}
	}
	
	DeleteCriticalSection(&m_DependencyCritSec);
	CloseHandle(m_DoneEvent);
}
void JobGroup::addRaw(IJob* job, unsigned int flags){
	const JobHandle handle = {
		flags,
		job,
		this
	};
	m_JobHandles.push_back(handle);
	m_JobsRemaining++;
}
void JobGroup::addDependency(JobGroup* dependency){
	//Find circular dependency
	for(int i=0; i<m_DependentGroups.size(); ++i){
		if(m_DependentGroups[i] == dependency){
			printf("CIRCULAR DEPENDENCY DETECTED, I AM DISSAPOINT.\n");
			return;
		}
	}

	dependency->m_DependentGroups.push_back(this);
	m_DependencyCount++;
	m_DependenciesLeft++;
}
void JobGroup::_notifyDependencyDone(){
	//NOTE: can't use atomics, might get added to JobManager during this call
	EnterCriticalSection(&m_DependencyCritSec);
	m_DependenciesLeft--;
	if(m_DependenciesLeft == 0){
		//That was the last dependency, ready to run now
		if(m_JobManager!=nullptr){
			LeaveCriticalSection(&m_DependencyCritSec);
			m_JobManager->_notifyGroupReady(this);
		}else{
			LeaveCriticalSection(&m_DependencyCritSec);
		}
	}else{
		LeaveCriticalSection(&m_DependencyCritSec);
	}
}
void JobGroup::_notifyChildDone(){
	const unsigned int jobsRemaining = InterlockedDecrement(&m_JobsRemaining);
	if(jobsRemaining==0){
		for(int i=0; i<m_DependentGroups.size(); ++i){
			m_DependentGroups[i]->_notifyDependencyDone();
		}
		//Reset counters to allow reuse next frame
		m_DependenciesLeft = m_DependencyCount;
		m_JobsRemaining = m_JobHandles.size();

		SetEvent(m_DoneEvent);
	}
}
void JobGroup::waitUntilDone(){
	WaitForSingleObject(m_DoneEvent, INFINITE);
}
const char* JobGroup::name() const{
	return m_Name.c_str();
}
};//namespace tt