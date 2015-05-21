#pragma once
#include <Windows.h>

namespace tt{
class JobGroup;
class IJobSystemLogger{
public:
	typedef unsigned int groupid_t;

	//Adds a job to this logger
	virtual groupid_t addJob(const JobGroup& group, unsigned int threadId, LARGE_INTEGER startTime, LARGE_INTEGER endTime)=0;
};
};