//	PPR Functions

#ifndef PPRCOMMON_H_
#define PPRCOMMON_H_

struct ParamGraph;

const double DAMP_FACTOR = 0.85;

void JacobiSolver(ParamGraph *g, double *pref, double *pprValue, int numIter);
void ParamJacobiSolver(ParamGraph *g, double *pref, double *pprValue,
					   const double *param, double eps = 1e-10);

void ReadVector(double *pprValue, int allocSize, const char* fname, const char* suffix);
void WriteVector(double *pprValue, int size, char* fname, char* suffix);

//	Evaluation

double L1(double *trueprv, double *estprv, int n);
double KendalTau(double *trueprv, double *estprv, int n, int k);



#endif /* PPRCOMMON_H_ */
