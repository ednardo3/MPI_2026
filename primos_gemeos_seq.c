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

/* Conta pares (i, i + 2) de primos gemeos com i + 2 <= N */
long long int contar_pares_gemeos(long long int N) {
    long long int i;
    long long int pares = 0;

    for (i = 3; i <= N - 2; i += 2) {
        if (eh_primo(i) && eh_primo(i + 2)) {
            pares++;
        }
    }

    return pares;
}

int main(int argc, char *argv[]) {
    long long int N;
    long long int total_pares;
    char *fim;
    double inicio, fim_tempo;

    MPI_Init(&argc, &argv);

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    N = strtoll(argv[1], &fim, 10);
    if (*fim != '\0' || N < 1) {
        fprintf(stderr, "Erro: N deve ser um inteiro positivo.\n");
        MPI_Finalize();
        return 1;
    }

    inicio = MPI_Wtime();
    total_pares = contar_pares_gemeos(N);
    fim_tempo = MPI_Wtime();

    printf("N = %lld\n", N);
    printf("Pares de primos gemeos: %lld\n", total_pares);
    printf("Tempo de execucao: %.6f s\n", fim_tempo - inicio);

    MPI_Finalize();
    return 0;
}
