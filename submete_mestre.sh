#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N primos_mestre
#$ -pe mpi 4
#$ -o saida_mestre.out
#$ -e erro_mestre.err

N=${1:-10000000}

mpicc -O2 -Wall -o primos_gemeos_mestre primos_gemeos_mestre.c
if [ $? -ne 0 ]; then
    echo "Erro na compilacao." >&2
    exit 1
fi

mpirun -np $NSLOTS ./primos_gemeos_mestre "$N"
