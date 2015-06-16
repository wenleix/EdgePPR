#   Learning to rank with the DEIM method
from sys import argv
import numpy as np
import numpy.linalg
import scipy.linalg
import scipy.sparse
import time
import sys

import pprCommon

defaultParam = np.array([0.34, 0.33, 1.0, 0.33, 0.75, 0.25, 1.0])
LearnLambda = 1000.0
LearnRate = 1e-4
LossB = 0.2


if len(argv) != 7:
    print >> sys.stderr, 'Usage %s [CSC-Prefix] [iset.txt] [isetSize] [U.npy] [NumPaper] [TrainRank.txt]' % (argv[0])
    sys.exit(1)

cscPrefix = argv[1]
isetFname = argv[2]
isetSize = int(argv[3])
basisFname = argv[4]
numPaper = int(argv[5])
trainRankFname = argv[6]

#   Get interpolation set
Iset = pprCommon.LoadIset(isetFname)[:isetSize]

#	Load the basis
U = np.load(basisFname).astype(float)
#   Add the following line due to a performance issue with early version of NumPy, see 
#   http://mail.scipy.org/pipermail/scipy-user/2014-May/035694.html
U = U.flatten().reshape(U.shape)  

(n, dim) = U.shape

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

#   Read train rank
trainRank = pprCommon.ReadTrainRank(trainRankFname)
trainSubset = list(set(trainRank))
submap = np.zeros(n, dtype = 'int')
submap[:] = -1
for (idx, val) in enumerate(trainSubset):
    submap[val] = idx

param = np.array(defaultParam)  


for optIter in range(20):
    tic = time.time()

    WtPU = np.zeros(WtU.shape)
    for i in range(len(param)):
        WtPU += param[i] * (WtPi[i].dot(U))
    WtMU = WtU - 0.85 * WtPU

    choMM = scipy.linalg.cho_factor(WtMU.T.dot(WtMU))
    y = scipy.linalg.cho_solve(choMM, WtMU.T.dot(Wtb))
    residual = WtMU.dot(y) - Wtb

    grady = []
    for i in range(len(param)):     
        # Compute the gradient of y according to Eq 11.
        partialM = 0.85 * (WtPi[i].dot(U))
        rhs = partialM.T.dot(residual) + WtMU.T.dot(partialM.dot(y))
        gradyi = scipy.linalg.cho_solve(choMM, rhs)

        grady.append(gradyi)

    #   Reconstruct x[TrainSubset]
    xtrain = U[trainSubset, :].dot(y)
    gradxtrain = []
    for i in range(len(param)):
        gradxtraini = U[trainSubset, :].dot(grady[i])
        gradxtrain.append(gradxtraini)

    #   Compute the gradient and update parameter
    gradParam = 2.0 * LearnLambda * (param - defaultParam) #   Gradient of regularizer

    for j1 in range(len(trainRank)):
        for j2 in range(j1 + 1, len(trainRank)):
            v = trainRank[j1]
            u = trainRank[j2]
            uu = submap[u]
            vv = submap[v]
            gradFactor = max(0.0, xtrain[uu] - xtrain[vv] + LossB) 

            for i in range(len(param)):
                gradParam[i] += gradFactor * (gradxtrain[i][uu] - gradxtrain[i][vv])

    pprCommon.ProjectGradient(gradParam)
    param -= LearnRate * gradParam

    toc = time.time()

    print >> sys.stdout,  "PROFILE: Optimization iteration %d finished, %.4f sec elapsed." % (optIter+1, (toc-tic))
    print >> sys.stdout, "INFO: Param = ", ' '.join([str(x) for x in param])
    sys.stdout.flush()

