/*
 * ParamGraph.cpp
 *
 *  Created on: Mar 27, 2014
 *      Author: wenleix
 */

#include "ParamGraph.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <sys/time.h>

using namespace std;

ParamVertex* ParamGraph::GetVertex(int vertexID) {
    return vertexList + vertexID;
}

void ParamVertex::SetVType(int intvtype) {
	assert(0 <= intvtype && intvtype < ParamGraph_NumVertexTypes);
	vtype = (ParamGraph::VertexType)(intvtype);
}

void ParamEdge::SetEType(int intetype) {
	assert(0 <= intetype && intetype < ParamGraph_NumEdgeTypes);
	etype = (ParamGraph::EdgeType)(intetype);
}


//  graph.txt format:
//  # Vertices, # Edges
//  VertexID1, VertexType1, OutDeg1, Name1
//  DstID1, EdgeLabel1
//  DstID2, EdgeLabel2
//  ...
//  VertexID2, VertexType2, OutDeg2, Name2
//  ...
ParamGraph* ParamGraph::ReadTextFile(FILE* txtFile) {
	const int MaxLineLen = 10000;
	char lineBuf[MaxLineLen];

    timeval start, end;
    gettimeofday(&start, NULL);

    ParamGraph *g = new ParamGraph();
	int numRead;
    numRead = fscanf(txtFile, "%d%d", &g->numVertex, &g->numEdge);
    assert(numRead == 2);

    fprintf(stdout, "INFO: # Vertex: %d, # Edge: %d\n", g->numVertex, g->numEdge);

    g->vertexList = new ParamVertex[g->numVertex];
	for (int i = 0; i < g->numVertex; i++)
		g->vertexList[i].inDeg = 0;
	g->edgeList = new ParamEdge[g->numEdge];

    int currentOffset = 0;
    for (int i = 0; i < g->numVertex; i++) {
        int vtxID, vtxType, outDeg;
        numRead = fscanf(txtFile, "%d%d%d ", &vtxID, &vtxType, &outDeg);
        assert(numRead == 3);
        assert(vtxID == i);
        assert(currentOffset + outDeg <= g->numEdge);

        char* fgetsRet = fgets(lineBuf, MaxLineLen, txtFile);
        assert(fgetsRet != NULL);
        lineBuf[strlen(lineBuf) - 1] = 0;   //  Delete the last '\n'
        g->vertexList[i].vertexID = i;
        g->vertexList[i].outDeg = outDeg;
        g->vertexList[i].SetVType(vtxType);
        g->vertexList[i].name = string(lineBuf);
        for (int j = 0; j < ParamGraph_NumEdgeTypes; j++)
        	g->vertexList[i].outEdgeTypeCount[j] = 0;

        ParamEdge *edgeStart = g->edgeList + currentOffset;
        for (int j = 0; j < outDeg; j++) {
        	int dstID, edgeType;
        	numRead = fscanf(txtFile, "%d%d", &dstID, &edgeType);
            assert(numRead == 2);
            edgeStart[j].dstId = dstID;
            edgeStart[j].SetEType(edgeType);
            g->vertexList[i].outEdgeTypeCount[edgeType]++;	//	Update the edge type count
        }

        //  Check self-loop and update inDeg
        for (int j = 0; j < outDeg; j++) {
			int dstID = edgeStart[j].dstId;
			if (i == dstID) {
				fprintf(stderr, "Self-loop: %d.\n", i);
			}
            assert(i != dstID);
            g->vertexList[dstID].inDeg++;
		}

        g->vertexList[i].outEdge = edgeStart;
        currentOffset += outDeg;
    }

    assert(currentOffset == g->numEdge);

    gettimeofday(&end, NULL);

    double elapse_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    fprintf(stdout, "INFO: Graph load done, time elapsed: %.2f sec.\n", elapse_ms / 1000.0);
	fflush(stdout);

	return g;
}

//	Set up the edge-weight based on the authority-flow parameter
void ParamGraph::SetWeight(double param[7]) {
	for (int i = 0; i < numVertex; i++) {
		ParamVertex *v = GetVertex(i);
		for (int j = 0; j < v->outDeg; j++) {
			ParamEdge *e = &(v->outEdge[j]);
			int etype = e->GetIntEType();
			e->weight = param[etype] / v->outEdgeTypeCount[etype];
		}
	}
}


ParamGraph::~ParamGraph() {
	delete [] vertexList;
	delete [] edgeList;
}


