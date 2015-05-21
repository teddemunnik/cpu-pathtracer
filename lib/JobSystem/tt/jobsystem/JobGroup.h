#pragma once
#include <vector>
#include <Windows.h>
#include "JobHandle.h"

namespace tt{
class JobManager;
class IJob;
class JobGroup{
private:
	//Handles to the jobs in this group
	std::vector<JobHandle> m_JobHandles;
	//Total number of dependencies on this group
	unsigned int m_DependencyCount;
	//Amount of dependencies the group still has to wait on
	__declspec(align(32)) unsigned int m_DependenciesLeft;
	//List of job groups that are dependent on this one
	std::vector<JobGroup*> m_DependentGroups;
	//The JobManager this group was added to, if any
	JobManager* m_JobManager;
	//Critical section for dependency management in the group
	CRITICAL_SECTION m_DependencyCritSec;
	//Number of jobs that still need to finish for this group to be done
	//Triggers dependent groups when 0
	__declspec(align(32)) unsigned int m_JobsRemaining;
	//Event used to wait on this group to be done
	HANDLE m_DoneEvent;
	//Name for identifying this group (i.e. debugging)
	std::string m_Name;

	void _init();
	void _destroy();

public:
	JobGroup();
	JobGroup(const char* name, int reserve=0);
	~JobGroup();

	//Internal: Job Group that this groups is dependent on has finished
	//Thread safe.
	void _notifyDependencyDone();
	//Internal: Job in this group has finished
	//Thread safe.
	void _notifyChildDone();

	//Adds a new job to this group, can be any type with the () operator
	template<typename FUNC>
	void add(FUNC job){
		FuncJob<FUNC>* fjob = new FuncJob<FUNC>(job);
		addRaw(fjob, JobHandle::kDestroysJob);
	}
	//Adds a raw job, prevents runtime allocation
	void addRaw(IJob* job, unsigned int flags=0);

	//Adds a dependency to this group
	void addDependency(JobGroup* dependency);
	//Waits until all jobs in this group have completed
	void waitUntilDone();
	//Getter for name of the group
	const char* name() const;

	friend JobManager;
};
};