#   Galerkin method
import sys
import os
from sys import argv
import numpy as np
import time
import scipy.sparse
import numpy.linalg
from scipy.sparse import csc_matrix
import scipy.sparse.linalg

import pprCommon

if len(argv) != 7:
    print >> sys.stderr, 'Usage %s [CSC-Prefix] [U.npy] [Dim] [Param.txt] [TruthPrefix] [NumPaper]' % (argv[0])
    sys.exit(1)

cscPrefix = argv[1]
basisFname = argv[2]
dim = int(argv[3])
paramFname = argv[4]
pprPrefix = argv[5]
numPaper = int(argv[6])

#	Load the basis
U = np.load(basisFname)[:, :dim].astype(float)
#   Add the following line due to a performance issue with early version of NumPy, see 
#   http://mail.scipy.org/pipermail/scipy-user/2014-May/035694.html
U = U.flatten().reshape(U.shape)  

UtU = np.eye(dim)
(n, _) = U.shape

#   Load P_i and prepare for U'*P_i*U
Pi = []
UtPiU = []        
for i in range(7):
    fname = '%s%d.bin' % (cscPrefix, i)
    _Pi = pprCommon.ReadBinCscMat(fname)
    Pi.append(_Pi)
    tic = time.time()
    _UtPiU = _Pi.T.dot(U).T.dot(U)
    toc = time.time()
    UtPiU.append(_UtPiU)   
    print >> sys.stdout,  "PROFILE: UtPU[%d] formed, %.4f sec elapsed." % (i, toc - tic)
    sys.stdout.flush()

#   Get b
b = np.zeros(n)
b[:numPaper] = 0.15   
Utb = U.T.dot(b)

paramFile = open(paramFname, 'r')
idx = 0

for line in paramFile.readlines():
    if line == '\n':
        continue
    w = [float(s) for s in line.split(' ')]

    #   Form the reduced system
    tic = time.time()

    UtPU = np.zeros(UtU.shape)
    for i in range(len(w)):
        UtPU += w[i] * UtPiU[i]
    UtMU = UtU - 0.85 * UtPU

    toc = time.time()
    print >> sys.stdout,  "PROFILE: Reduced system formed, %.4f sec elapsed." % (toc - tic)

    #   Solve the reduced system
    tic = time.time()
    y = numpy.linalg.solve(UtMU, Utb)
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


