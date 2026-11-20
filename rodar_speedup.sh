#!/bin/bash

N=${1:-10000000}
B=${2:-1000}
CAMPANHA="v2_B_impares"
CSV_SAIDA="resultados_speedup_N${N}_${CAMPANHA}.csv"
PROCESSOS="2 3 4 5 6 7 8 9 10 11 12 13 14 15 16"

if [ ! -f job_speedup.sh ]; then
    echo "Erro: arquivo job_speedup.sh nao encontrado no diretorio atual." >&2
    exit 1
fi

# Compila as tres versoes com as mesmas opcoes
mpicc -O2 -Wall -o primos_gemeos_seq primos_gemeos_seq.c || exit 1
mpicc -O2 -Wall -o primos_gemeos_ciclico primos_gemeos_ciclico.c || exit 1
mpicc -O2 -Wall -o primos_gemeos_mestre primos_gemeos_mestre.c || exit 1

chmod +x job_speedup.sh

# Cria o cabecalho do CSV preservando os dados da campanha anterior
echo "programa,N,P,B,repeticao,tempo_s,pares,mediana_s,speedup,eficiencia" > "$CSV_SAIDA"

# 1. Submete a versao sequencial (P = 1)
qsub -N "spd_${CAMPANHA}_seq_P1" \
     -pe mpi 1 \
     -o "saida_spd_${CAMPANHA}_seq_P1.out" \
     -e "erro_spd_${CAMPANHA}_seq_P1.err" \
     job_speedup.sh "seq" "$N" "NA" "$CSV_SAIDA"

JOB_ANTERIOR="spd_${CAMPANHA}_seq_P1"

# 2. Submete a versao ciclica para todos os P de 2 a 16
for P in $PROCESSOS; do
    NOME_JOB="spd_${CAMPANHA}_cic_P${P}"
    qsub -N "$NOME_JOB" \
         -hold_jid "$JOB_ANTERIOR" \
         -pe mpi "$P" \
         -o "saida_${NOME_JOB}.out" \
         -e "erro_${NOME_JOB}.err" \
         job_speedup.sh "ciclico" "$N" "NA" "$CSV_SAIDA"
    JOB_ANTERIOR="$NOME_JOB"
done

# 3. Submete a versao mestre-trabalhadores (B = 1000 fixo) para todos os P de 2 a 16
for P in $PROCESSOS; do
    NOME_JOB="spd_${CAMPANHA}_mes_P${P}"
    qsub -N "$NOME_JOB" \
         -hold_jid "$JOB_ANTERIOR" \
         -pe mpi "$P" \
         -o "saida_${NOME_JOB}.out" \
         -e "erro_${NOME_JOB}.err" \
         job_speedup.sh "mestre" "$N" "$B" "$CSV_SAIDA"
    JOB_ANTERIOR="$NOME_JOB"
done

echo "Bateria de speedup submetida para N = $N e B = $B (fixo)!"
echo "Acompanhe com 'qstat'. Resultados em: $CSV_SAIDA"
