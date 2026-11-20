#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N primos_ciclico
#$ -pe mpi 4
#$ -o saida_ciclico.out
#$ -e erro_ciclico.err

N=${1:-10000000}

mpicc -O2 -Wall -o primos_gemeos_ciclico primos_gemeos_ciclico.c
if [ $? -ne 0 ]; then
    echo "Erro na compilacao." >&2
    exit 1
fi

mpirun -np $NSLOTS ./primos_gemeos_ciclico "$N"
