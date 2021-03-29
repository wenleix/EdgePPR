import numpy as np
import sys
import struct
from scipy.sparse import csc_matrix

#   Wenlei: np.loadtxt is so damn slow!!
def quickloadtxt(pprfname):
    pprfile = open(pprfname, "r")
    lines = pprfile.readlines()
    pprvector = np.array([float(l) for l in lines])
    pprfile.close()

    return pprvector

def loadbin(pprfname):
    pprfile = open(pprfname, 'rb')
    nbff = pprfile.read(4)
    (n, ) = struct.unpack('i', nbff)
    pprvector = np.fromfile(pprfile, np.dtype('d'), n)
    pprfile.close()

    return pprvector


def ReadTrainRank(trainRankFname):
    trFile = open(trainRankFname, 'r')
    lines = [l for l in trFile.readlines() if l.rstrip() != '']
    n = int(lines[0])
    assert n + 1 == len(lines)

    trRank = np.array([int(l) for l in lines[1:]])
    trFile.close()
    return trRank


#   Load a family of vectors
#   As those vectors are generated from C/C++, not packed as a single npz file.
def loadVectors(prefix, suffix, numVec):
    for i in range(numVec):
        fname = '%s-%d.%s' % (prefix, i, suffix)
        if suffix == 'txt':
            ppr = quickloadtxt(fname)
        elif suffix == 'bin':
            ppr = loadbin(fname)
        else:
            raise
        if (i % 100 == 0):
            sys.stderr.write('file %s loaded.\n' % (fname))
        yield ppr   # Generator ! :) 


#   Load interpolation set
def interpload(interpname):
    interpfile = open(interpname, 'r')
    lines = interpfile.readlines()
    interp = np.array([int(l) for l in lines if l != '\n'])
    interpfile.close()

    return interp

#   Read matrices
def ReadCscMat(filename):
    f = open(filename, 'r')
    nmStr = f.readline()
    invdataStr = f.readline()
    indicesStr = f.readline()
    indptrStr = f.readline()
    f.close()

    nm = nmStr.split(' ')
    n = int(nm[0])
    m = int(nm[1])
    invdata = np.array([int(x) for x in invdataStr.split(' ')])
    indices = np.array([int(x) for x in indicesStr.split(' ')])
    indptr = np.array([int(x) for x in indptrStr.split(' ')])
    
    data = 1.0 / invdata
    return csc_matrix( (data, indices, indptr), shape = (n, m) )

def ReadBinCscMat(filename):
    bsmfile = open(filename, 'rb')
    nbff = bsmfile.read(8)
    (n, m) = struct.unpack('ii', nbff)
    nbff = bsmfile.read(4)
    (numEntry, ) = struct.unpack('i', nbff)

    data = np.fromfile(bsmfile, np.dtype('d'), numEntry)
    indices = np.fromfile(bsmfile, np.dtype('i'), numEntry)

    nbff = bsmfile.read(4)
    (numIndPtr, ) = struct.unpack('i', nbff)
    assert(numIndPtr == m + 1)
    indptr = np.fromfile(bsmfile, np.dtype('i'), numIndPtr)

    bsmfile.close()
    return csc_matrix( (data, indices, indptr), shape = (n, m) )

#   Evaluation functions
def Prec(idx1, idx2, n):
    t1 = set(idx1[:n])
    t2 = set(idx2[:n])
    cor = len(t1.intersection(t2))
    return float(cor) / n

def KendalTau(idx1, idx2, n):
    t1 = set(idx1[:n])
    t2 = set(idx2[:n])
    both = np.array(list(t1.union(t2)))
    bt = set(both)
    r1 = np.zeros(len(idx1))
    r2 = np.zeros(len(idx1))

    cc1 = 0
    cc2 = 0
    for i in range(len(idx1)):
        if idx1[i] in bt:
            cc1 += 1
            r1[idx1[i]] = cc1
        if idx2[i] in bt:
            cc2 += 1
            r2[idx2[i]] = cc2
        if cc1 == len(both) and cc2 == len(both):
            break

    cor = 0
    for idd in both:
        assert r1[idd] != 0
        assert r2[idd] != 0

    for i in range(len(both)):
        for j in range(i + 1, len(both)):
            id1 = both[i]
            id2 = both[j]
            cmp1 = r1[id1] < r1[id2]
            cmp2 = r2[id1] < r2[id2]
            if cmp1 == cmp2:
                cor += 1

    allp = len(both) * (len(both) - 1) / 2
    wrong = allp - cor
    return float(wrong) / allp

def SpearmanDist(idx1, idx2, n):
    t1 = set(idx1[:n])
    t2 = set(idx2[:n])
    both = np.array(list(t1.union(t2)))
    bt = set(both)
    r1 = np.zeros(len(idx1))
    r2 = np.zeros(len(idx1))

    cc1 = 0
    cc2 = 0
    for i in range(len(idx1)):
        if idx1[i] in bt:
            cc1 += 1
            r1[idx1[i]] = cc1
        if idx2[i] in bt:
            cc2 += 1
            r2[idx2[i]] = cc2
        if cc1 == len(both) and cc2 == len(both):
            break

    for idd in both:
        assert r1[idd] != 0
        assert r2[idd] != 0

    dist = 0
    for i in range(len(both)):
        idd = both[i]
        x = r1[idd]
        y = r2[idd]
        dist += abs(x - y)
    
    ll = len(both)
    maxd = ll * ll / 2 if ll % 2 == 0 else (ll - 1) * (ll + 1) / 2
    return dist / maxd


