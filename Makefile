CC = g++
LD = g++

CFLAGS=-O2 
CFLAGS11=-O2 -std=c++0x
LFLAGS=-O2

.PHONY : clean 

all: bin/CreateObjGraph bin/ParamPPR bin/GenCSRMatrix 

# Objective Files
obj/ParamGraph.o: include/ParamGraph.h src/ParamGraph.cpp
	$(CC) -c $(CFLAGS) src/ParamGraph.cpp -o obj/ParamGraph.o -I include/

obj/PPRCommon.o: include/PPRCommon.h src/PPRCommon.cpp include/ParamGraph.h obj/ParamGraph.o
	$(CC) -c $(CFLAGS) src/PPRCommon.cpp -o obj/PPRCommon.o -I include/


# Executables
bin/CreateObjGraph: src/CreateObjGraph.cpp 
	$(CC) $(CFLAGS11) src/CreateObjGraph.cpp -o bin/CreateObjGraph

bin/ParamPPR: src/ParamPPR.cpp include/ParamGraph.h include/PPRCommon.h obj/ParamGraph.o obj/PPRCommon.o 
	$(CC) $(CFLAGS) src/ParamPPR.cpp obj/ParamGraph.o obj/PPRCommon.o \
		  -o bin/ParamPPR -I include/ -lpthread

bin/GenCSRMatrix: src/GenCSRMatrix.cpp include/ParamGraph.h obj/ParamGraph.o
	$(CC) $(CFLAGS) src/GenCSRMatrix.cpp obj/ParamGraph.o  \
		  -o bin/GenCSRMatrix -I include/

clean:
	rm bin/* obj/*


