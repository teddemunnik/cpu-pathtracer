#include <tt/jobsystem/JobSystem.h>

JobManager g_JobManager;

int main(int argc, char** argv){
	g_JobManager.initialize(4);

	//Setup jobs
	const int numJobs = g_JobManager.maxConcurrency();
	JobGroup prepareJobs(numJobs);
	JobGroup updateJobs(numJobs);
	updateJobs.addDependency(&prepareJobs);
	for(int i=0; i<numJobs; ++i){
		prepareJobs.add([i](){
			printf("Preparing job %i\n", i);
		});
		updateJobs.add([i](){
			printf("Updating job %i, LAMBDAS ARE AWESOMEEE!!!!!\n", i);
		});
	}

	while(true){
		g_JobManager.spawn(&prepareJobs);
		g_JobManager.spawn(&updateJobs);
		prepareJobs.waitUntilDone(); //Implicit, since updateJobs depends on it
		updateJobs.waitUntilDone();
		printf("All jobs done.\n");
		getchar();
	}
}