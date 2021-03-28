/*
 * PPRCommon.cpp
 *
 *  Created on: Mar 29, 2014
 *      Author: wenleix
 */

#include "PPRCommon.h"
#include "ParamGraph.h"

#include <cmath>
#include <cstring>
#include <cassert>
#include <cstdio>

#include <algorithm>
#include <set>

#include <sys/time.h>

using namespace std;

//	The standard Jacobi solver / Power Iteration, used to generate ground truth.
void JacobiSolver(ParamGraph *g, double *pref, double *pprValue, int numIter) {
	double *preValue = new double[g->numVertex];
	double *curValue = new double[g->numVertex];

	memset(preValue, 0, sizeof(double) * g->numVertex);

    timeval start, end;
    gettimeofday(&start, NULL);

    for (int iIter = 0; iIter < numIter; iIter++) {
    	memcpy(curValue, pref, sizeof(double) * g->numVertex);

        for (int i = 0; i < g->numVertex; i++) {
            if (preValue[i] == 0)
                continue;

            ParamVertex *vtx = g->GetVertex(i);
        	ParamEdge *outEdge = vtx->outEdge;
        	for (int j = 0; j < vtx->outDeg; j++) {
        		int dstId = outEdge[j].dstId;
        		double contrib = preValue[i] * DAMP_FACTOR * outEdge[j].weight;
        		curValue[dstId] += contrib;
        	}
        }

		double* temp = curValue;
		curValue = preValue;
		preValue = temp;
    }

    //	Calculate the L1 change in the last iteration.
    double L1Change = 0.0;
    for (int i = 0; i < g->numVertex; i++)
    	L1Change += fabs(curValue[i] - preValue[i]);

    fprintf(stderr, "DEBUG: L1Change in Last Iteration = %.4le\n", L1Change);

    //  Copy the result out
	memcpy(pprValue, preValue, sizeof(double) * g->numVertex);

    int nonZeroCount = 0;
    for (int i = 0; i < g->numVertex; i++)
        if (pprValue[i] != 0.0) nonZeroCount++;
    fprintf(stdout, "DEBUG: # non zero entries / All vertex: %d / %d\n", nonZeroCount, g->numVertex);

    gettimeofday(&end, NULL);

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    fprintf(stdout, "Running Time: %.2lf sec\n", elapsed_ms / 1000.0);

    delete [] preValue;
    delete [] curValue;
}


//	Parametric version of Jacobi Solver / Power Iteration, used to generate ground truth in parallel
//	Don't rely on edge.weight.
void ParamJacobiSolver(ParamGraph *g, double *pref, double *pprValue,
					   const double *param, double eps) {
	double *preValue = new double[g->numVertex];
	double *curValue = new double[g->numVertex];

	memset(preValue, 0, sizeof(double) * g->numVertex);

    timeval start, end;
    gettimeofday(&start, NULL);

    //	Magic number 500
    for (int iIter = 0; iIter < 500; iIter++) {
    	memcpy(curValue, pref, sizeof(double) * g->numVertex);

        for (int i = 0; i < g->numVertex; i++) {
            if (preValue[i] == 0)
                continue;

            ParamVertex *vtx = g->GetVertex(i);
            double adjustDampFactor = DAMP_FACTOR;

        	ParamEdge *outEdge = vtx->outEdge;
        	for (int j = 0; j < vtx->outDeg; j++) {
        		ParamEdge *curEdge = &outEdge[j];
        		int etype = curEdge->GetIntEType();
        		int dstId = curEdge->dstId;
        		double contrib = preValue[i] * adjustDampFactor * param[etype] / vtx->outEdgeTypeCount[etype];
        		curValue[dstId] += contrib;
        	}
        }

		double* temp = curValue;
		curValue = preValue;
		preValue = temp;

		if (iIter % 10 == 0) {
			double L1Change = 0.0;
			for (int i = 0; i < g->numVertex; i++)
				L1Change += fabs(curValue[i] - preValue[i]);
			if (L1Change < eps) {
				fprintf(stderr, "DEBUG: L1 small enough at iteration %d, early quit :)\n", iIter);
				break;
			}
		}

    }

    //	Calculate the L1 change in the last iteration.
    double L1Change = 0.0;
    for (int i = 0; i < g->numVertex; i++)
    	L1Change += fabs(curValue[i] - preValue[i]);

    fprintf(stderr, "DEBUG: L1Change in Last Iteration = %.4le\n", L1Change);

    //  Copy the result out
	memcpy(pprValue, preValue, sizeof(double) * g->numVertex);

    int nonZeroCount = 0;
    for (int i = 0; i < g->numVertex; i++)
        if (pprValue[i] != 0.0) nonZeroCount++;
    fprintf(stderr, "DEBUG: # non zero entries / All vertex: %d / %d\n", nonZeroCount, g->numVertex);

    gettimeofday(&end, NULL);

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    fprintf(stdout, "Running Time: %.2lf sec\n", elapsed_ms / 1000.0);

    delete [] preValue;
    delete [] curValue;
}


void ReadVector(double *pprValue, int allocSize, const char* filename, const char* suffix) {
	char fullname[1000];
	sprintf(fullname, "%s.%s", filename, suffix);

	if (strcmp(suffix, "txt") == 0) {
		FILE *f = fopen(fullname, "r");
		int numRead = -1;
		int size = 0;
		numRead = fscanf(f, "%d", &size);
		assert(numRead == 1 && size == allocSize);
		for (int i = 0; i < size; i++) {
			numRead = fscanf(f, "%lf", &pprValue[i]);
			assert(numRead == 1);
		}
	} else if (strcmp(suffix, "bin") == 0) {
		FILE *f = fopen(fullname, "rb");
		int numRead;
		int size;
		numRead = fread(&size, sizeof(int), 1, f);
		assert(numRead == 1);
		assert(allocSize == size);
		numRead = fread(pprValue, sizeof(double), size, f);
		assert(numRead == size);
	} else {
		assert(false);
	}
}

void WriteVector(double *pprValue, int size, char* filename, char* suffix) {
	char fullname[1000];
	sprintf(fullname, "%s.%s", filename, suffix);

	if (strcmp(suffix, "txt") == 0) {
		FILE *f = fopen(fullname, "w");
		fprintf(f, "%d\n", size);
		for (int i = 0; i < size; i++)
			fprintf(f, "%.8lf\n", pprValue[i]);
		fclose(f);
	} else if (strcmp(suffix, "bin") == 0) {
		FILE *f = fopen(fullname, "wb");
		fwrite(&size, sizeof(int), 1, f);
		fwrite(pprValue, sizeof(double), size, f);
		fclose(f);
	} else {
		assert(false);
	}
}

//	Evaluation Part

//	A helper function to avoid use lambda expression in C++11
struct IndexSortHelper {
	double *value;

	IndexSortHelper(double* _value) : value(_value) {}

	inline bool operator()(int i1, int i2) {
		return value[i1] > value[i2];
	}
};


double L1(double *trueprv, double *estprv, int n) {
	double diff = 0.0, sum = 0.0;
	for (int i = 0; i < n; i++) {
		diff += fabs(trueprv[i] - estprv[i]);
		sum += trueprv[i];
	}

	return diff / sum;
}

double KendalTau(double *trueprv, double *estprv, int n, int k) {
	int *idx1 = new int[n];
	int *idx2 = new int[n];
	for (int i = 0; i < n; i++) {
		idx1[i] = i;
		idx2[i] = i;
	}
	sort(idx1, idx1 + n, IndexSortHelper(trueprv));
	sort(idx2, idx2 + n, IndexSortHelper(estprv));

	set<int> both;
	both.insert(idx1, idx1 + k);
	both.insert(idx2, idx2 + k);
	fprintf(stdout, "INFO: Size of union for two top %d: %lu .\n", k, both.size());

	int *r1 = new int[n];
	int *r2 = new int[n];
	memset(r1, 0, sizeof(int) * n);
	memset(r2, 0, sizeof(int) * n);
	int cc1 = 0, cc2 = 0;
	for (int i = 0; i < n; i++) {
		if (both.find(idx1[i]) != both.end()) {
			cc1++;
			r1[idx1[i]] = cc1;
		}
		if (both.find(idx2[i]) != both.end()) {
			cc2++;
			r2[idx2[i]] = cc2;
		}
		if (cc1 == both.size() && cc2 == both.size())
			break;
	}
	assert(cc1 == both.size() && cc2 == both.size());

	int cor = 0;
	for (set<int>::iterator it1 = both.begin(); it1 != both.end(); it1++) {
		set<int>::iterator it2 = it1; it2++;
		for (; it2 != both.end(); it2++) {
			int id1 = *it1;
			int id2 = *it2;
			bool cmp1 = r1[id1] < r1[id2];
			bool cmp2 = r2[id1] < r2[id2];
			if (cmp1 == cmp2)
				cor++;
		}
	}

    delete [] idx1;
    delete [] idx2;
    delete [] r1;
    delete [] r2;

//    fprintf(stderr, "DEBUG: Kendau-Tau finished!.\n");


	double allp = (double)(both.size()) * (both.size() - 1) / 2.0;
	double wrong = allp - cor;
	return wrong / allp;
}

/*
double SpearmanDist(double *trueprv, double *estprv, int n, int k) {
	int *idx1 = new int[n];
	int *idx2 = new int[n];
	for (int i = 0; i < n; i++) {
		idx1[i] = i;
		idx2[i] = i;
	}
	sort(idx1, idx1 + n, IndexSortHelper(trueprv));
	sort(idx2, idx2 + n, IndexSortHelper(estprv));

	set<int> both;
	both.insert(idx1, idx1 + k);
	both.insert(idx2, idx2 + k);
	fprintf(stdout, "INFO: Size of union for two top %d: %lu .\n", k, both.size());

	int *r1 = new int[n];
	int *r2 = new int[n];
	memset(r1, 0, sizeof(int) * n);
	memset(r2, 0, sizeof(int) * n);
	int cc1 = 0, cc2 = 0;
	for (int i = 0; i < n; i++) {
		if (both.find(idx1[i]) != both.end()) {
			cc1++;
			r1[idx1[i]] = cc1;
		}
		if (both.find(idx2[i]) != both.end()) {
			cc2++;
			r2[idx2[i]] = cc2;
		}
		if (cc1 == both.size() && cc2 == both.size())
			break;
	}
	assert(cc1 == both.size() && cc2 == both.size());

	double dist = 0.0;
	for (set<int>::iterator it = both.begin(); it != both.end(); it++) {
		assert(r1[*it] != 0);
		assert(r2[*it] != 0);

		dist += fabs(r1[*it] - r2[*it]);
	}
	double maxd = both.size() * both.size() / 2;

	delete [] idx1;
	delete [] idx2;
	delete [] r1;
	delete [] r2;

	return dist / maxd;
}
*/




