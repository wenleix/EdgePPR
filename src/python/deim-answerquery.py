#   DEIM
import sys
import os
from sys import argv
import numpy as np
import time
import scipy.sparse
import numpy.linalg
from scipy.sparse import csc_matrix

import pprCommon


if len(argv) != 9:
    print 'Usage %s [CSC-Prefix] [iset.txt] [isetSize] [U.npy] [Dim] [Param.txt] [TruthPrefix] [NumPaper]' % (argv[0])
    sys.exit(1)

cscPrefix = argv[1]
isetFname = argv[2]
isetSize = int(argv[3])
basisFname = argv[4]
dim = int(argv[5])
paramFname = argv[6]
pprPrefix = argv[7]
numPaper = int(argv[8])

#   Get interpolation set
Iset = pprCommon.LoadIset(isetFname)[:isetSize]

#   Load the basis
U = np.load(basisFname)[:, :dim].astype(float)
(n, _) = U.shape
#   Add the following line due to a performance issue with early version of NumPy, see 
#   http://mail.scipy.org/pipermail/scipy-user/2014-May/035694.html
U = U.flatten().reshape(U.shape)  

#   Load P_i and prepare for P[Iset, :]
Pi = []
WtPi = []
for i in range(7):
    fname = '%s%d.bin' % (cscPrefix, i)
    _Pi = pprCommon.ReadBinCscMat(fname).tocsr()
    Pi.append(_Pi)
    WtPi.append(_Pi[Iset, :])

Wt = scipy.sparse.identity(n, format='csr')[Iset, :] 
WtU = U[Iset, :]

#   Get b
b = np.zeros(n)
b[:numPaper] = 0.15    
Wtb = b[Iset]

paramFile = open(paramFname, 'r')
idx = 0

for line in paramFile.readlines():
    if line == '\n':
        continue
    w = [float(s) for s in line.split(' ')]

    #   Form the reduced system
    tic = time.time()
    WtPU = np.zeros(WtU.shape)
    for i in range(len(w)):
        WtPU += w[i] * (WtPi[i].dot(U))
    WtMU = WtU - 0.85 * WtPU
    toc = time.time()
    print >> sys.stdout,  "PROFILE: Reduced system formed, %.4f sec elapsed." % (toc - tic)

    #   Reconstruct the PageRank vector
    tic = time.time()
    if isetSize > dim: 
        (y, _, _, _) = numpy.linalg.lstsq(WtMU, Wtb)
    else:
        y = numpy.linalg.solve(WtMU, Wtb)
    toc = time.time()
    print >> sys.stdout,  "PROFILE: Reduced system solved, %.4f sec elapsed." % (toc - tic)

    #   Reconstruct the PageRank vector
    tic = time.time()
    x = U.dot(y)
    toc = time.time()
    print >> sys.stdout,  "PROFILE: PageRank vector reconstructed, %.4f sec elapsed." % (toc - tic)
 
    #   Evaluation
    xexact = pprCommon.LoadBinVec("%s-%d.bin" % (pprPrefix, idx))
    delta = x - xexact
    print >> sys.stdout, 'EVAL: NL1 =', numpy.linalg.norm(delta, 1) / numpy.linalg.norm(xexact, 1)

    idx1 = sorted(np.array(range(len(xexact))), key=lambda idx: xexact[idx], reverse=True) 
    idx2 = sorted(np.array(range(len(x))), key=lambda idx: x[idx], reverse=True) 
    print >> sys.stdout, 'EVAL: KendallTau@100 =', pprCommon.KendallTau(idx1, idx2, 100)

    sys.stdout.flush()

    idx += 1

paramFile.close()



