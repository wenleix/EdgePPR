#ifndef PARAMGRAPH_H_
#define PARAMGRAPH_H_

#include <string>


//	A simple structure for parameterized graph, not necessary cache efficient.

struct ParamVertex;
struct ParamEdge;

#define ParamGraph_NumVertexTypes 4
#define ParamGraph_NumEdgeTypes 7

//	A simple struct
struct Parameter {
	double v[ParamGraph_NumEdgeTypes];
};


struct ParamGraph {
	enum VertexType { Paper = 0, Author = 1, Conf = 2, Forum = 3 };
	enum EdgeType { Cite = 0, WrittenBy = 1, AuthorWrite = 2, PublishedIn = 3, ConfContainsPaper = 4, ConfCalledByForum = 5, ForumCalledConf = 6 };

    int numVertex;
    int numEdge;

    ParamVertex* vertexList;
    ParamEdge* edgeList;

    ParamVertex* GetVertex(int vertexID);

    //	Constructed from text file
    //  graph.txt format:
    //  # Vertices, # Edges
    //  VertexID1, VertexType1, OutDeg1, Name1
    //  DstID1, DstLabel1
    //  DstID2, DstLabel2
    //  ...
    //  VertexID2, VertexType2, OutDeg2, Name2
    //  ...
    static ParamGraph* ReadTextFile(FILE* txtFile);

    ~ParamGraph();

    //	Set up the edge-weight based on the authority-flow parameter
    void SetWeight(double param[ParamGraph_NumEdgeTypes]);

private:
    ParamGraph() {
    	numVertex = -1;
    	numEdge = -1;
    	vertexList = NULL;
    	edgeList = NULL;
    }

};

struct ParamVertex {
    int vertexID;		//  This is redundant, but I guess it is fine to be a bit redundant on vertex data :P
    std::string name;	//	The overloaded "name".
    ParamGraph::VertexType vtype;

	int inDeg;
    int outDeg;
    ParamEdge* outEdge;  	//  The outgoing edge list (a simple reference to the edge list).

    int outEdgeTypeCount[ParamGraph_NumEdgeTypes];

    void SetVType(int intvtype);
};

struct ParamEdge {
    int dstId;
    ParamGraph::EdgeType etype;
    double weight;	//	The parameterized weight, based on user's preference and the edge type

    void SetEType(int intetype);
    inline int GetIntEType() {
    	return (int)(etype);
    }

};


#endif /* PARAMGRAPH_H_ */
