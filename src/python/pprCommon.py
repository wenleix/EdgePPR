import numpy as np
import sys
import struct
from scipy.sparse import csc_matrix

def LoadBinVec(pprfname):
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

#   Load interpolation set for DEIM
def LoadIset(isetFname):
    isetFile = open(isetFname, 'r')
    lines = isetFile.readlines()
    iset = np.array([int(l) for l in lines if l != '\n'])
    isetFile.close()

    return iset

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

def KendallTau(idx1, idx2, n):
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


#   Used in learning to rank
def ProjectGradient(grad):
    c1 = 2.0 / 3.0
    c2 = 1.0 / 3.0
    nt1 = grad[0] * c1 - (grad[1] + grad[3]) * c2
    nt2 = grad[1] * c1 - (grad[0] + grad[3]) * c2
    nt3 = grad[3] * c1 - (grad[0] + grad[1]) * c2
    grad[0] = nt1
    grad[1] = nt2
    grad[3] = nt3

    nnt1 = grad[4] * 0.5 - grad[5] * 0.5
    nnt2 = grad[5] * 0.5 - grad[4] * 0.5
    grad[4] = nnt1
    grad[5] = nnt2



