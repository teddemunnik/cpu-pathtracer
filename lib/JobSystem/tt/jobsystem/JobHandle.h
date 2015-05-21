#pragma once

namespace tt{
class IJob;
class JobGroup;
struct JobHandle{
	enum{
		kDestroysJob //This handle will destroy the job it holds on completion
	};
	unsigned int flags;
	IJob* job;
	JobGroup* group;
};
};//namespace tt