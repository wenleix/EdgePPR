#   Learning to rank with the Galerkin method
from sys import argv
import numpy as np
import numpy.linalg
import scipy.linalg
import time
import sys

import pprCommon

defaultParam = np.array([0.34, 0.33, 1.0, 0.33, 0.75, 0.25, 1.0])
LearnLambda = 1000.0
LearnRate = 1e-4
LossB = 0.2


if len(argv) != 5:
    print >> sys.stderr, 'Usage %s [CSC-Prefix] [U.npy] [NumPaper] [TrainRank.txt]' % (argv[0])
    sys.exit(1)

cscPrefix = argv[1]
basisFname = argv[2]
numPaper = int(argv[3])
trainRankFname = argv[4]

#	Load the basis
U = np.load(basisFname).astype(float)
#   Add the following line due to a performance issue with early version of NumPy, see 
#   http://mail.scipy.org/pipermail/scipy-user/2014-May/035694.html
U = U.flatten().reshape(U.shape)  

(n, dim) = U.shape
UtU = np.eye(dim)

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
    print >> sys.stdout,  "PROFILE: UtPU[%d] formed, %.1f sec elapsed." % (i, toc - tic)
    sys.stdout.flush()

#   Get b
b = np.zeros(n)
b[:numPaper] = 0.15
Utb = U.T.dot(b)

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

    UtPU = np.zeros(UtU.shape)
    for i in range(len(param)):
        UtPU += param[i] * UtPiU[i]
    UtMU = UtU - 0.85 * UtPU

    luUtMU = scipy.linalg.lu_factor(UtMU)
    y = scipy.linalg.lu_solve(luUtMU, Utb)

    grady = []
    for i in range(len(param)):     
        rhs = 0.85 * (UtPiU[i].dot(y))
        gradyi = scipy.linalg.lu_solve(luUtMU, rhs)
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

