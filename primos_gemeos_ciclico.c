#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* Verifica se n e primo usando divisao por tentativa */
int eh_primo(long long int n) {
    long long int d;

    if (n < 2) {
        return 0;
    }
    if (n == 2) {
        return 1;
    }
    if (n % 2 == 0) {
        return 0;
    }

    for (d = 3; d * d <= n; d += 2) {
        if (n % d == 0) {
            return 0;
        }
    }

    return 1;
}

/* Conta os pares atribuidos ciclicamente a este processo */
long long int contar_pares_local(long long int N, int rank, int size) {
    long long int i;
    long long int pares_locais = 0;
    long long int inicio = 3 + 2LL * rank;
    long long int passo = 2LL * size;

    for (i = inicio; i <= N - 2; i += passo) {
        if (eh_primo(i) && eh_primo(i + 2)) {
            pares_locais++;
        }
    }

    return pares_locais;
}

int main(int argc, char *argv[]) {
    int rank, size;
    long long int N;
    long long int pares_locais, total_pares = 0;
    char *fim;
    double inicio_tempo, fim_tempo;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    N = strtoll(argv[1], &fim, 10);
    if (*fim != '\0' || N < 1) {
        if (rank == 0) {
            fprintf(stderr, "Erro: N deve ser um inteiro positivo.\n");
        }
        MPI_Finalize();
        return 1;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    inicio_tempo = MPI_Wtime();

    pares_locais = contar_pares_local(N, rank, size);
    MPI_Reduce(&pares_locais, &total_pares, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    fim_tempo = MPI_Wtime();

    if (rank == 0) {
        printf("N = %lld | Processos = %d\n", N, size);
        printf("Pares de primos gemeos: %lld\n", total_pares);
        printf("Tempo de execucao: %.6f s\n", fim_tempo - inicio_tempo);
    }

    MPI_Finalize();
    return 0;
}
