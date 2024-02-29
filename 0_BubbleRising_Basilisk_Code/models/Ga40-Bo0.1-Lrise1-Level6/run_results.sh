#!/bin/bash

domainSize=64
basicLevel=6
t_max=100
tsnap=0.1
mpinum=30  

Lrise=60
Ga=62.82
Bo=0.0736
radiusLevel=7
file="Ga$Ga-Bo$Bo-Lrise$Lrise-Level$radiusLevel"
qcc -O2 -w -fopenmp -Wall getResults.c -lm -o getResults
export OMP_NUM_THREADS=1
./getResults $Ga $Bo $domainSize $Lrise $basicLevel $radiusLevel $t_max $tsnap

