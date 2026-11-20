#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TAG_TAREFA 1
#define TAG_RESULTADO 2
#define TAG_FIM 3

/* Quantidade padrao de candidatos impares por tarefa caso B nao seja informado */
#define TAM_BLOCO_PADRAO 10000LL

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

/* Conta pares (i, i + 2) dentro do subintervalo [ini, fim] */
long long int contar_pares_bloco(long long int ini, long long int fim) {
    long long int i;
    long long int pares = 0;

    if (ini < 3) {
        ini = 3;
    }
    if (ini % 2 == 0) {
        ini++;
    }

    for (i = ini; i <= fim; i += 2) {
        if (eh_primo(i) && eh_primo(i + 2)) {
            pares++;
        }
    }

    return pares;
}

/* Logica do processo mestre (rank 0) */
long long int executar_mestre(long long int N, int size, long long int tam_bloco) {
    long long int prox_ini = 3;
    long long int limite = N - 2;
    long long int total_pares = 0;
    long long int parcial;
    long long int tarefa[2] = {0, 0};
    long long int deslocamento;
    int trabalhadores_ativos = 0;
    int worker;
    MPI_Status status;

    /* Distribuicao inicial: uma tarefa de ate B candidatos impares por trabalhador */
    for (worker = 1; worker < size; worker++) {
        if (prox_ini <= limite) {
            tarefa[0] = prox_ini;
            deslocamento = 2LL * (tam_bloco - 1);
            if (deslocamento > limite - prox_ini) {
                tarefa[1] = limite;
            } else {
                tarefa[1] = prox_ini + deslocamento;
            }
            prox_ini = tarefa[1] + 2;

            MPI_Send(tarefa, 2, MPI_LONG_LONG, worker, TAG_TAREFA, MPI_COMM_WORLD);
            trabalhadores_ativos++;
        } else {
            tarefa[0] = 0;
            tarefa[1] = 0;
            MPI_Send(tarefa, 2, MPI_LONG_LONG, worker, TAG_FIM, MPI_COMM_WORLD);
        }
    }

    /* Recebe resultados parciais e envia novas tarefas sob demanda */
    while (trabalhadores_ativos > 0) {
        MPI_Recv(&parcial, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, TAG_RESULTADO,
                 MPI_COMM_WORLD, &status);
        total_pares += parcial;
        worker = status.MPI_SOURCE;

        if (prox_ini <= limite) {
            tarefa[0] = prox_ini;
            deslocamento = 2LL * (tam_bloco - 1);
            if (deslocamento > limite - prox_ini) {
                tarefa[1] = limite;
            } else {
                tarefa[1] = prox_ini + deslocamento;
            }
            prox_ini = tarefa[1] + 2;

            MPI_Send(tarefa, 2, MPI_LONG_LONG, worker, TAG_TAREFA, MPI_COMM_WORLD);
        } else {
            tarefa[0] = 0;
            tarefa[1] = 0;
            MPI_Send(tarefa, 2, MPI_LONG_LONG, worker, TAG_FIM, MPI_COMM_WORLD);
            trabalhadores_ativos--;
        }
    }

    return total_pares;
}

/* Logica dos processos trabalhadores (rank > 0) */
void executar_trabalhador(void) {
    long long int tarefa[2] = {0, 0};
    long long int parcial;
    MPI_Status status;

    while (1) {
        MPI_Recv(tarefa, 2, MPI_LONG_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_FIM) {
            break;
        }

        parcial = contar_pares_bloco(tarefa[0], tarefa[1]);
        MPI_Send(&parcial, 1, MPI_LONG_LONG, 0, TAG_RESULTADO, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    long long int N;
    long long int tam_bloco = TAM_BLOCO_PADRAO;
    long long int total_pares = 0;
    char *fim;
    double inicio_tempo, fim_tempo;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "Erro: o modelo mestre-trabalhador exige pelo menos 2 processos.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (argc < 2 || argc > 3) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <N> [B]\n", argv[0]);
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

    if (argc == 3) {
        tam_bloco = strtoll(argv[2], &fim, 10);
        if (*fim != '\0' || tam_bloco < 1) {
            if (rank == 0) {
                fprintf(stderr, "Erro: B deve ser um inteiro positivo.\n");
            }
            MPI_Finalize();
            return 1;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    inicio_tempo = MPI_Wtime();

    if (rank == 0) {
        total_pares = executar_mestre(N, size, tam_bloco);
    } else {
        executar_trabalhador();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    fim_tempo = MPI_Wtime();

    if (rank == 0) {
        printf("N = %lld | Processos = %d (1 mestre, %d trabalhadores) | B = %lld\n",
               N, size, size - 1, tam_bloco);
        printf("Pares de primos gemeos: %lld\n", total_pares);
        printf("Tempo de execucao: %.6f s\n", fim_tempo - inicio_tempo);
    }

    MPI_Finalize();
    return 0;
}
