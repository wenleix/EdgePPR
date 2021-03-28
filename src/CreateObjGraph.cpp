//  Create the object graph from Wenlei Format data.

//  graph.txt
//  # Vertices, # Edges
//  VertexID1, VertexType1, OutDeg1, Name1
//  DstID1, DstLabel1
//  DstID2, DstLabel2
//  ...
//  VertexID2, VertexType2, OutDeg2, Name2
//  ...

//      VertexType: 
//          0 - Paper
//          1 - Author
//          2 - Conf
//			3 - Forum
//      EdgeType:
//          0 - cite
//          1 - written_by
//          2 - author_write
//          3 - published_in
//          4 - conf_publish_paper
//			5 - conf_calledby_forum
//			6 - forum_call_conf

//  ID Assignment Sequence:
//  Paper, Author, Conf, Forum

#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>

using namespace std;

const int MaxLineLen = 1000000;
const int MaxNumVertex = 10000000;  //  10M vertices at this moment, could definitely be more.
char lineBuf[MaxLineLen];

unordered_map<string, int> author_id;     //  Author ID, start from 0
unordered_map<string, int> conf_id;   	  //  Conference ID, start from 0
unordered_map<string, int> forum_id;      //  Forum ID, start from 0
vector<string> author_name;               //  Get author name from the authorID
vector<string> conf_name;                 //  Get the conference name from confId, Conference = Forum + Year!
vector<string> forum_name;				  //  Get the forum name

vector<int> paper_newid;                  //  Map from old paper id to new id.

struct Publication {
    string title;
    string authors;
    int year;
    string forum;
    string conf;	//	forum+Year
    int id;

    vector<int> refs;
};

struct Edge {
    int dstID;
    int type;

    Edge(int _dstID, int _type) {
        dstID = _dstID;
        type = _type;
    }
};

vector<Publication*> papers;
vector<Edge> outEdge[MaxNumVertex];   //  Out edges for a specific vertex.


void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    assert(elems.empty());

    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

//	Refactor, parse those attribute lines
char* ParseAttrLine(char *lineBuf, const char *attr) {
	int attrLen = strlen(attr);
    assert(strncmp(lineBuf, attr, attrLen) == 0);
    lineBuf[strlen(lineBuf) - 1] = 0;   //  Delete the last '\n'

    return lineBuf + attrLen + 2;
}

char* ParseNextLine(FILE* file, const char *attr) {
    assert(fgets(lineBuf, MaxLineLen, file) != NULL);
    return ParseAttrLine(lineBuf, attr);
}

Publication* readNewPaper(FILE* file) {
    if (fgets(lineBuf, MaxLineLen, file) == NULL)
        return NULL;

    Publication *p = new Publication();
    p->title = string(ParseAttrLine(lineBuf, "Title"));
    p->id = atoi(ParseNextLine(file, "ID"));
    p->authors = string(ParseNextLine(file, "Authors"));
    p->year = atoi(ParseNextLine(file, "Year"));
    p->forum = string(ParseNextLine(file, "Conf"));		//	TODO: hmm
    p->refs.resize(atoi(ParseNextLine(file, "NumRef")));

    istringstream iss(ParseNextLine(file, "Refs"));
    for (int i = 0; i < p->refs.size(); i++) {
    	iss >> p->refs[i];
//        assert((iss >> p->refs[i]) != NULL);
    }

    assert(fgets(lineBuf, MaxLineLen, file) != NULL);
    lineBuf[strlen(lineBuf) - 1] = 0;   //  Delete the last '\n'
    assert(lineBuf[0] == 0);

    char yearbuf[100];
    sprintf(yearbuf, "@%d", p->year);
    p->conf = p->forum + yearbuf;

    return p;
}

int GetOrCreateID(const string& st, unordered_map<string, int>& dict, vector<string>& namelist) {
	auto it = dict.find(st);
	if (it == conf_id.end()) {
		int newId = dict.size();
		dict[st] = newId;
		namelist.push_back(st);
		return newId;
	} else {
		return it->second;
	}

	assert(false);
	return -1;
}

void WriteOutEdges(FILE *graphFile, const vector<Edge>& out) {
    for (const Edge& e : out) {
        fprintf(graphFile, "%d %d\n", e.dstID, e.type);
    }
}

int main(int argv, char** argc) {
    paper_newid.reserve(10000000);

    if (argv != 3) {
        fprintf(stderr, "Usage: %s [BigRedFormatFile] [GraphName]\n", argc[0]);
        return 1;
    }

    FILE *wenleiFile = fopen(argc[1], "r");
    FILE *graphFile = fopen(argc[2], "w");

    while (true) {
        Publication *paper = readNewPaper(wenleiFile);
        if (paper == NULL) 
            break;
        papers.push_back(paper);
        if (paper->id >= paper_newid.size()) {
            paper_newid.resize(paper->id + 1, -1);
        }
        paper_newid[paper->id] = papers.size() - 1;
    }

    vector< pair<int, int> > paper_author;
    vector< pair<int, int> > paper_conf;        //  Note, paper_id to conf_id, need offset to generate global id.
    vector< pair<int, int> > conf_forum;

    //  Set up (paper, author) and (paper, venueyear) pairs.
    for (int i = 0; i < papers.size(); i++) {
        Publication* p = papers[i];
        int confId = GetOrCreateID(p->conf, conf_id, conf_name);
        int forumId = GetOrCreateID(p->forum, forum_id, forum_name);
        paper_conf.push_back(make_pair(i, confId));
        conf_forum.push_back(make_pair(confId, forumId));

        vector<string> authors;
        split(p->authors, ',', authors);
        for (string& a : authors) {
        	int aid = GetOrCreateID(a, author_id, author_name);
            paper_author.push_back(make_pair(i, aid));
        }
    }

    //	Unique Conf - Forum edges
    sort(conf_forum.begin(), conf_forum.end());
    conf_forum.resize(unique(conf_forum.begin(), conf_forum.end()) - conf_forum.begin());

    fprintf(stdout, "INFO: # Papers = %lu.\n", papers.size());
    fprintf(stdout, "INFO: # Authors = %lu.\n", author_name.size());
    fprintf(stdout, "INFO: # Conf = %lu.\n", conf_name.size());
    fprintf(stdout, "INFO: # Forum = %lu.\n", forum_name.size());

    fprintf(stdout, "INFO: #Paper-Author = %lu.\n", paper_author.size());
    fprintf(stdout, "INFO: #Paper-Conf = %lu.\n", paper_conf.size());
    fprintf(stdout, "INFO: #Conf-Forum = %lu.\n", conf_forum.size());

    //  Citation edges
    int missingCite = 0;
    int existCite = 0;
    int selfCite = 0;
    for (int i = 0; i < papers.size(); i++) {
        Publication *p = papers[i];
        for (auto rid : p->refs) {
            int newID = paper_newid[rid];
            if (i == newID) {
//            	fprintf(stderr, "Ignore self-loop: %d. \n", i);
            	selfCite++;
            	continue;
            }
            if (newID == -1) {
//                fprintf(stderr, "Debug: Paper ID %d is not occurred.\n", rid);
                missingCite++;
                continue;
            }
            outEdge[i].push_back(Edge(newID, 0 /* Cite */));
            existCite++;
        }
    }
    fprintf(stdout, "INFO: missingCite = %d, existCite = %d, selfCite = %d.\n", missingCite, existCite, selfCite);

    //  Make up the edges!
    int author_offset = papers.size();
    int conf_offset = author_offset + author_name.size();
    int forum_offset = conf_offset + conf_name.size();

    //  written_by and author_write
    for (pair<int, int> pa : paper_author) {
        int paperID = pa.first;
        int g_authorID = author_offset + pa.second;			//	GlobalID for this author
        outEdge[paperID].push_back(Edge(g_authorID, 1 /* written_by */));
        outEdge[g_authorID].push_back(Edge(paperID, 2 /* author_write */));
    }

    //  published_in and conf_publish_paper
    for (pair<int, int> pc : paper_conf) {
        int paperID = pc.first;
        int g_confID = conf_offset + pc.second;				//	GlobalID for this conf
        outEdge[paperID].push_back(Edge(g_confID, 3 /* published_in */ ));
        outEdge[g_confID].push_back(Edge(paperID, 4 /* conf_publish_paper */ ));
    }

    //	conf_calledby_forum and forum_call_conf
    for (pair<int, int> cf : conf_forum) {
    	int g_confID = conf_offset + cf.first;
    	int g_forumID = forum_offset + cf.second;
    	outEdge[g_confID].push_back(Edge(g_forumID, 5 /* conf_calledby_forum */ ));
    	outEdge[g_forumID].push_back(Edge(g_confID, 6 /* forum_call_conf */ ));
    }

    int numVertex = papers.size() + author_name.size() + conf_name.size() + forum_name.size();
    int numEdge = 0;
    for (int i = 0; i < numVertex; i++)
        numEdge += outEdge[i].size();

    fprintf(stderr, "# Vertices = %d, # Edges = %d.\n", numVertex, numEdge);
    fprintf(graphFile, "%d %d\n", numVertex, numEdge);
    //  Paper vertex
    for (int i = 0; i < papers.size(); i++) {
        fprintf(graphFile, "%d 0 %lu %s\n", i, outEdge[i].size(), papers[i]->title.c_str());
        WriteOutEdges(graphFile, outEdge[i]);
    }
    //  Author vertex
    for (int i = 0; i < author_name.size(); i++) {
        int g_authorID = author_offset + i;
        fprintf(graphFile, "%d 1 %lu %s\n", g_authorID, outEdge[g_authorID].size(), 
                                            author_name[i].c_str());
        WriteOutEdges(graphFile, outEdge[g_authorID]);
    }
    //  Conf vertex
    for (int i = 0; i < conf_name.size(); i++) {
        int g_confID = conf_offset + i;
        fprintf(graphFile, "%d 2 %lu %s\n", g_confID, outEdge[g_confID].size(),
                                            conf_name[i].c_str());
        WriteOutEdges(graphFile, outEdge[g_confID]);
    }
    //  Forum vertex
    for (int i = 0; i < forum_name.size(); i++) {
        int g_forumID = forum_offset + i;
        fprintf(graphFile, "%d 3 %lu %s\n", g_forumID, outEdge[g_forumID].size(),
                                            forum_name[i].c_str());
        WriteOutEdges(graphFile, outEdge[g_forumID]);
    }

    fclose(wenleiFile);
    fclose(graphFile);

    for (Publication* p : papers) 
        delete p;

    return 0;
}
