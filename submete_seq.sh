#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N primos_seq
#$ -pe mpi 1
#$ -o saida_seq.out
#$ -e erro_seq.err

# Valor padrao de N caso nao seja passado como argumento para o script
N=${1:-1000000}

# Carregue aqui o modulo do MPI do cluster, se necessario (ex: module load openmpi)

mpicc -O2 -Wall -o primos_gemeos_seq primos_gemeos_seq.c
if [ $? -ne 0 ]; then
    echo "Erro na compilacao." >&2
    exit 1
fi

mpirun -np 1 ./primos_gemeos_seq "$N"
