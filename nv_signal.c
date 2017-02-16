#include "nv.h"

void NV_signalHandler(int signum)
{
	if(signum == SIGINT){
		if(NV_globalExecFlag & NV_EXEC_FLAG_INTERRUPT){
			exit(EXIT_FAILURE);
		}
		NV_globalExecFlag |= NV_EXEC_FLAG_INTERRUPT;
	}
}
