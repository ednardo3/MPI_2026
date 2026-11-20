#!/bin/bash

N=${1:-10000000}
CAMPANHA="v2_B_impares"
CSV_SAIDA="resultados_matriz_B_N${N}_${CAMPANHA}.csv"
PROCESSOS="4 8 16"

if [ ! -f job_matriz_B.sh ]; then
    echo "Erro: arquivo job_matriz_B.sh nao encontrado no diretorio atual." >&2
    exit 1
fi

mpicc -O2 -Wall -o primos_gemeos_mestre primos_gemeos_mestre.c
if [ $? -ne 0 ]; then
    echo "Erro na compilacao." >&2
    exit 1
fi

chmod +x job_matriz_B.sh

# Cria o cabecalho do CSV preservando os dados da campanha anterior
echo "programa,N,P,B,repeticao,tempo_s,pares,mediana_s,speedup,eficiencia" > "$CSV_SAIDA"

JOB_ANTERIOR=""
for P in $PROCESSOS; do
    NOME_JOB="matB_${CAMPANHA}_P${P}"
    if [ -z "$JOB_ANTERIOR" ]; then
        qsub -N "$NOME_JOB" \
             -pe mpi "$P" \
             -o "saida_${NOME_JOB}.out" \
             -e "erro_${NOME_JOB}.err" \
             job_matriz_B.sh "$N" "$CSV_SAIDA"
    else
        qsub -N "$NOME_JOB" \
             -hold_jid "$JOB_ANTERIOR" \
             -pe mpi "$P" \
             -o "saida_${NOME_JOB}.out" \
             -e "erro_${NOME_JOB}.err" \
             job_matriz_B.sh "$N" "$CSV_SAIDA"
    fi
    JOB_ANTERIOR="$NOME_JOB"
done

echo "Matriz de B submetida! Acompanhe com 'qstat'. Resultados em: $CSV_SAIDA"
