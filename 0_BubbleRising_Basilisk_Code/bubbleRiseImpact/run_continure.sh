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
CC99='mpicc -std=c99' qcc -w -Wall -O2 -D_MPI=1 bubbleRise.c -o $file -lm
mpirun -np $mpinum ./$file   $Ga $Bo $domainSize $Lrise $basicLevel $radiusLevel $t_max $tsnap
pngfile="Ga$Ga-Bo$Bo-png"
mkdir -p "$pngfile"
gnuplot -c plot_script.gp "$pngfile/Ga$Ga-Bo$Bo-Lrise$Lrise-velocity.png" "$pngfile/Ga$Ga-Bo$Bo-Lrise$Lrise-position.png" "$pngfile/Ga$Ga-Bo$Bo-Lrise$Lrise-ratio.png"
cp results_shape.dat "../results/$file.csv"
cp -r "$pngfile" "../results_Picture/$file"

cd ..
mv "$file" "./models/$file"