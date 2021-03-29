#   The preprocessing step of DEIM: given ppr_1, ppr_2, ... ppr_k, generate the orthogonal basis.
#   This is based on SVD so the basis are sorted according to the singular values (from large to small)

import sys
import os
from sys import argv
import numpy as np
import time
from scipy import linalg
import deimCommon


if len(argv) != 7:
    print 'Usage %s [Prefix] [Suffix] [NumVec] [Dim] [dtype] [V.bin]' % (argv[0])
    sys.exit(1)

prefix = argv[1]
suffix = argv[2]
numVec = int(argv[3])
dim = int(argv[4])
dtype = argv[5]
vnpyName = argv[6]

tic = time.time()

pprs = np.zeros((dim, numVec), dtype="float32")
idx = 0
for ppr in deimCommon.loadVectors(prefix, suffix, numVec):
    pprs[:, idx] = ppr
    idx += 1

print pprs.shape

toc = time.time()
print >> sys.stderr, "Read complete, %.2f sec elapsed." % (toc - tic)

tic = time.time()

(U, _, _) = linalg.svd(pprs, full_matrices=False, compute_uv=True)

toc = time.time()
print >> sys.stderr,  "SVD complete, %.2f sec elapsed." % (toc - tic)

#print >> sys.stderr, 'Matrix R = '
#print >> sys.stderr, R

#   Save V 
np.save(vnpyName, U[:, :100])

#   Save it into a C++ compatible form
#   As tofile() write the matrix in the row order, we transpose the matrix first.
#print >> sys.stdout, U.shape
#U[:, :100].T.tofile(vnpyName)


