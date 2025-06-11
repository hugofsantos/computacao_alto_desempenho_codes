#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 300        // Tamanho total da barra
#define STEPS 100000 // Número de iterações
#define ALPHA 0.1f   // Constante de difusão térmica

int main(int argc, char **argv){
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (N % size != 0){
    if (rank == 0)
      fprintf(stderr, "N (%d) deve ser divisível por número de processos (%d)\n", N, size);
    MPI_Finalize();
    return 1;
  }

  int local_n = N / size;

  float *u = calloc(local_n + 2, sizeof(float));
  float *u_new = calloc(local_n + 2, sizeof(float));

  if (rank == 0){
    u[1] = 1000.0f;
  }

  double start_time = MPI_Wtime();

  for (int step = 0; step < STEPS; step++){
    MPI_Request requests[4];
    int req_count = 0;

    if (rank > 0){
      MPI_Isend(&u[1], 1, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
      MPI_Irecv(&u[0], 1, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
    }else{
      u[0] = u[1];
    }

    if (rank < size - 1){
      MPI_Isend(&u[local_n], 1, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
      MPI_Irecv(&u[local_n + 1], 1, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
    }else{
      u[local_n + 1] = u[local_n];
    }

    // Atualiza pontos internos que não dependem das bordas
    if (local_n > 2){
      for (int i = 2; i <= local_n - 1; i++){
        u_new[i] = u[i] + ALPHA * (u[i - 1] - 2 * u[i] + u[i + 1]);
      }
    }

    // Espera todas as comunicações terminarem, testando uma a uma
    int done = 0;
    int flags[4] = {0, 0, 0, 0};
    while (!done){
      done = 1;
      for (int i = 0; i < req_count; i++){
        if (!flags[i]){
          MPI_Test(&requests[i], &flags[i], MPI_STATUS_IGNORE);
        }
        if (!flags[i]){
          done = 0;
        }
      }
    }

    // Atualiza as bordas após receber dados
    if (local_n >= 1){
      u_new[1] = u[1] + ALPHA * (u[0] - 2 * u[1] + u[2]);
      u_new[local_n] = u[local_n] + ALPHA * (u[local_n - 1] - 2 * u[local_n] + u[local_n + 1]);
    }

    memcpy(&u[1], &u_new[1], local_n * sizeof(float));
  }

  double end_time = MPI_Wtime();

  if (rank == 0){
    printf(
        "Tempo de execução com MPI_Isend/Irecv + MPI_Test: barra tamanho %d, passos %d, tempo %f s\n",
        N,
        STEPS,
        end_time - start_time);
  }

  // for (int p = 0; p < size; p++){
  //   MPI_Barrier(MPI_COMM_WORLD);
  //   if (rank == p){
  //     printf("Processo %d (rank %d):\n", p, rank);
  //     for (int i = 1; i <= local_n; i++)
  //     {
  //       printf("  u[%d] = %.2f\n", i + rank * local_n - 1, u[i]);
  //     }
  //     fflush(stdout);
  //   }
  // }

  free(u);
  free(u_new);
  MPI_Finalize();
  return 0;
}
