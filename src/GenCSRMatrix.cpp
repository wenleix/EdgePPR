//	Hmm, generate the standard CSC representation from the graph file as it is too painful to do it in Python...
//	The CSC the chosen since the "from-vertex" representation easily maps to CSC (an edge (i, j) corresponds to
//	the entry A_{j, i}


#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>

#include "ParamGraph.h"

using namespace std;

//	Generate the CSC file for the specified edge type.
void GenCscFile(ParamGraph* g, int etype, FILE *cscFile) {
	//	Print the matrix size
	fprintf(cscFile, "%d %d\n", g->numVertex, g->numVertex);

	vector<int> invdata;
	vector<int> indices;


	for (int i = 0; i < g->numVertex; i++) {
		ParamVertex *vtx = g->GetVertex(i);
		int invw = vtx->outEdgeTypeCount[etype];
		for (int j = 0; j < vtx->outDeg; j++) {
			ParamEdge *e = &vtx->outEdge[j];
			if (e->GetIntEType() == etype) {
				invdata.push_back(invw);
				indices.push_back(e->dstId);
			}
		}
	}

	//	inv-data
	for (int i = 0; i < invdata.size(); i++) {
		if (i != 0) fprintf(cscFile, " ");
		fprintf(cscFile, "%d", invdata[i]);
	}
	fprintf(cscFile, "\n");

	//	indices
	for (int i = 0; i < indices.size(); i++) {
		if (i != 0) fprintf(cscFile, " ");
		fprintf(cscFile, "%d", indices[i]);
	}
	fprintf(cscFile, "\n");
	//	indptr
	fprintf(cscFile, "0");
	int acc = 0;
	for (int i = 0; i < g->numVertex; i++) {
		ParamVertex *vtx = g->GetVertex(i);
		acc += vtx->outEdgeTypeCount[etype];
		fprintf(cscFile, " %d", acc);
	}
	fprintf(cscFile, "\n");

	assert(acc == invdata.size());
}

//	Generate the Binary CSC file for the specified edge type.
void GenBinCscFile(ParamGraph* g, int etype, FILE *cscFile) {
	//	Print the matrix size
    fwrite(&g->numVertex, sizeof(int), 1, cscFile);
    fwrite(&g->numVertex, sizeof(int), 1, cscFile);

	vector<double> data;
	vector<int> indices;

	for (int i = 0; i < g->numVertex; i++) {
		ParamVertex *vtx = g->GetVertex(i);
		double w = 1.0 / vtx->outEdgeTypeCount[etype];
		for (int j = 0; j < vtx->outDeg; j++) {
			ParamEdge *e = &vtx->outEdge[j];
			if (e->GetIntEType() == etype) {
				data.push_back(w);
				indices.push_back(e->dstId);
			}
		}
	}

    int numEntry = data.size();
    assert(data.size() == indices.size());

	//	data
    fwrite(&numEntry, sizeof(int), 1, cscFile);
    fwrite(&data[0], sizeof(double), numEntry, cscFile);

	//	indices
    fwrite(&indices[0], sizeof(int), numEntry, cscFile);
	
    //	indptr
    int numIndPtr = g->numVertex + 1;   //  Temporal var for output
    int* tmpIndPtr = new int[numIndPtr];
	int acc = 0;
    tmpIndPtr[0] = 0;
    for (int i = 0; i < g->numVertex; i++) {
		ParamVertex *vtx = g->GetVertex(i);
		acc += vtx->outEdgeTypeCount[etype];
        tmpIndPtr[i + 1] = acc;
    }

    fwrite(&numIndPtr, sizeof(int), 1, cscFile);
    fwrite(tmpIndPtr, sizeof(int), numIndPtr, cscFile);
    delete [] tmpIndPtr;
	assert(acc == data.size());
}


int main(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s [ParamGraph.txt] [CSC-Prefix] [CSC-Suffix]\n", argv[0]);
		return 1;
	}

	FILE *gfile = fopen(argv[1], "r");
	char *cscPrefix = argv[2];
    char *suffix = argv[3];

	ParamGraph *g = ParamGraph::ReadTextFile(gfile);
	for (int i = 0; i < ParamGraph_NumEdgeTypes; i++) {
		char fnameBuf[1000];
		sprintf(fnameBuf, "%s%d.%s", cscPrefix, i, suffix);
        if (strcmp(suffix, "txt") == 0) {
		    FILE *cscFile = fopen(fnameBuf, "w");
		    GenCscFile(g, i, cscFile);
            fclose(cscFile);
        } else if (strcmp(suffix, "bin") == 0) {
		    FILE *cscFile = fopen(fnameBuf, "wb");
		    GenBinCscFile(g, i, cscFile);
            fclose(cscFile);
        } else {
            fprintf(stderr, "Unsupported file format: %s !\n", suffix);
            assert(false);
        }
	}


	return 0;
}



