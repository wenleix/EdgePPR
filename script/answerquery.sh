#!/bin/bash

PYTHON_CMD="python2.7"

DIM=100
MATRIX_PREFIX="data/mat/p"
BASIS_FILE="data/basis100.npy"
TEST_PARAMS="data/test-params.txt"
TEST_VEC_PREFIX="data/test-vecs/value"
NUMPAPER="2191288"
ISET_FILENAME="data/iset.txt"

echo Answer query with Galerkin method .
GALERKIN_OUTPUT="result/galerkin-answerquery.txt"
$PYTHON_CMD src/python/galerkin-answerquery.py $MATRIX_PREFIX $BASIS_FILE $DIM $TEST_PARAMS $TEST_VEC_PREFIX $NUMPAPER > $GALERKIN_OUTPUT

for ISET_SIZE in 100 120 200; do
    echo Answer query with DEIM method, with interpolation set size = $ISET_SIZE .
    DEIM_OUTPUT="result/deim$ISET_SIZE-answerquery.txt"
    $PYTHON_CMD src/python/deim-answerquery.py $MATRIX_PREFIX $ISET_FILENAME $ISET_SIZE $BASIS_FILE $DIM $TEST_PARAMS $TEST_VEC_PREFIX $NUMPAPER > $DEIM_OUTPUT
done

