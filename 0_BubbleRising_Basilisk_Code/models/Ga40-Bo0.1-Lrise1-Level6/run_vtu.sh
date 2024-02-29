#!/bin/bash

if [ -f "dumpToVTU" ];
then
rm dumpToVTU
fi
if [ -d "paraviewResults" ];
then
rm -r paraviewResults
fi
qcc -O2 -w -fopenmp -Wall dumpToVTU.c -lm -o dumpToVTU

export OMP_NUM_THREADS=32

./dumpToVTU
