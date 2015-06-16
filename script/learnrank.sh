#!/bin/bash

MATRIX_PREFIX="data/mat/p"
BASIS_FILE="data/basis100.npy"
NUMPAPER="2191288"
RANK_DATA="data/learn-rank.txt"
ISET_FILENAME="data/iset.txt" 

echo Learn to rank with Galerkin method .
GALERKIN_OUTPUT="result/galerkin-learnrank.txt"
python src/python/galerkin-learnrank.py $MATRIX_PREFIX $BASIS_FILE $NUMPAPER $RANK_DATA > $GALERKIN_OUTPUT

for ISET_SIZE in 200; do
    echo Learn to rank with DEIM method, with interpolation set size = $ISET_SIZE .
    DEIM_OUTPUT="result/deim$ISET_SIZE-learnrank.txt"
    python src/python/deim-learnrank.py $MATRIX_PREFIX $ISET_FILENAME $ISET_SIZE $BASIS_FILE $NUMPAPER $RANK_DATA > $DEIM_OUTPUT
done

