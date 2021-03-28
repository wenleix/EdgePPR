#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include <algorithm>
#include <pthread.h>

#include "ParamGraph.h"
#include "PPRCommon.h"

using namespace std;

//	Parameterized PPR based on ObjectRank mechanism.
//	Read the parameters from the file.

struct ThreadConfig {
	int threadId;

	double *params;
	int start, end;
	int numUseVtx;
	char* outputPrefix;
	double *pref;
	int *paramIdx;
	char* outputFormat;
	ParamGraph *g;
};

void *WorkerThread(void *__conf) {
	ThreadConfig* conf = (ThreadConfig*)__conf;

	fprintf(stdout, "INFO: Thread %d start!\n", conf->threadId);

	double *value = new double[conf->g->numVertex];
	for (int i = conf->start; i < conf->end; i++) {
		int paramId = conf->paramIdx[i];
		double *curParam = conf->params + ParamGraph_NumEdgeTypes * paramId;
		fprintf(stderr, "DEBUG: Param:");
		for (int j = 0; j < ParamGraph_NumEdgeTypes; j++)
			fprintf(stderr, " %.2lf", curParam[j]);
		fprintf(stderr, "\n");

		ParamJacobiSolver(conf->g, conf->pref, value, curParam);

		//	Print out the useful vertices
		char filenameBuf[1000];
		sprintf(filenameBuf, "%s-%d", conf->outputPrefix, paramId);
		WriteVector(value, conf->g->numVertex, filenameBuf, conf->outputFormat);
		fprintf(stderr, "DEBUG: Thread %d finished param %d!\n", conf->threadId, paramId);
	}

	fprintf(stdout, "INFO: Thread %d done!\n", conf->threadId);
	pthread_exit(NULL);
}

int main(int argc, char** argv) {
	if (argc != 7) {
		fprintf(stderr, "Usage: %s [ParamGraph.txt] [NumUseVtx] [ParameterFile] "
				"[OutputPrefix] [OutFormat: bin/txt] [NumThread]\n",
			    argv[0]);
		return 1;
	}

	FILE *gfile = fopen(argv[1], "r");
	int numUseVtx = atoi(argv[2]);	//	# Useful Vertices
	FILE *paramFile = fopen(argv[3], "r");
	char* outputPrefix = argv[4];
	char *outFormat = argv[5];
	int numThread = atoi(argv[6]);

	assert(strcmp(outFormat, "bin") == 0 || strcmp(outFormat, "txt") == 0);
	ParamGraph *g = ParamGraph::ReadTextFile(gfile);

	int paramId = 0;
	double *pref = new double[g->numVertex];
	memset(pref, 0, sizeof(double) * g->numVertex);
	for (int i = 0; i < numUseVtx; i++)
		pref[i] = 1.0 - DAMP_FACTOR;

	vector<double> params;
	//	Read all the parameters
	while (true) {
		double p;
		if (fscanf(paramFile, "%lf", &p) == EOF)
			break;
		params.push_back(p);
	}

	assert(params.size() % ParamGraph_NumEdgeTypes == 0);
	int numParam = params.size() / ParamGraph_NumEdgeTypes;
	fprintf(stdout, "INFO: # Params = %lu. \n", params.size() / ParamGraph_NumEdgeTypes);
	int *paramIdx = new int[numParam];		//	Random shuffle the parameter indices assigned to each thread, to get better workload balance.
	for (int i = 0; i < numParam; i++)
		paramIdx[i] = i;
	random_shuffle(paramIdx, paramIdx + numParam);

	ThreadConfig *conf = new ThreadConfig[numThread];
	pthread_t *threads = new pthread_t[numThread];
	for (int i = 0; i < numThread; i++) {
		conf[i].threadId = i;
		conf[i].params = &params[0];
		conf[i].start = i * numParam / numThread;
		conf[i].end = (i + 1) * numParam / numThread;
		conf[i].numUseVtx = numUseVtx;
		conf[i].outputPrefix = outputPrefix;
		conf[i].pref = pref;
		conf[i].g = g;
		conf[i].paramIdx = paramIdx;
		conf[i].outputFormat = outFormat;

		//	Start thread
		int rc = pthread_create(&threads[i], NULL /* attr */, WorkerThread, (void*)&conf[i]);
		assert(rc == 0);
	}

	for (int i = 0; i < numThread; i++)
		pthread_join(threads[i], NULL /* retval */);



	delete g;
	delete [] conf;
	delete [] threads;
	delete [] paramIdx;
	fclose(gfile);
	fclose(paramFile);

	//	Finish
	pthread_exit(NULL);

	return 0;
}



