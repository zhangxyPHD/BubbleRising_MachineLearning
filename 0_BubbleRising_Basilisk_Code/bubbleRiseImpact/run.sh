#!/bin/bash
if [ -f "bubbleRise" ];
then
rm bubbleRise
fi

if [ -f "dump" ];
then
rm dump
fi

if [ -d "Animations" ];
then
rm -r Animations
fi

if [ -d "Intermediate" ];
then
rm -r Intermediate
fi

if [ -d "Interfaces" ];
then
rm -r Interfaces
fi

if [ -d "Log" ];
then
rm -r Log
fi

CC99='mpicc -std=c99' qcc -w -Wall -O2 -D_MPI=1 bubbleRise.c -o bubbleRise -lm
mpirun -np 30 ./bubbleRise 100 1 32 11 5 6 30

# Galilei,  Bond,  domainSize,  centerXDrop,  basicLevel,  radiusLevel, t_max
