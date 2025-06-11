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
  }

  int local_n = N / size; // Número de células reais por processo

  // Aloca e inicializa com zero (células reais + 2 fantasmas)
  float *u = calloc(local_n + 2, sizeof(float));
  float *u_new = calloc(local_n + 2, sizeof(float));

  if (rank == 0){
    u[1] = 1000.0f;
  }

  double start_time = MPI_Wtime();
  for (int step = 0; step < STEPS; step++){
    int left = rank - 1;
    int right = rank + 1;

    // Estratégia: pares enviam primeiro, ímpares recebem primeiro
    if (rank % 2 == 0){
      // Envia para esquerda e direita
      if (left >= 0) MPI_Send(&u[1], 1, MPI_FLOAT, left, 0, MPI_COMM_WORLD);
      if (right < size) MPI_Send(&u[local_n], 1, MPI_FLOAT, right, 1, MPI_COMM_WORLD);

      // Recebe da esquerda e direita
      if (left >= 0) MPI_Recv(&u[0], 1, MPI_FLOAT, left, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      else u[0] = u[1]; // Borda esquerda fixa

      if (right < size) MPI_Recv(&u[local_n + 1], 1, MPI_FLOAT, right, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      else u[local_n + 1] = u[local_n]; // Borda direita fixa
    } 
    else {
      // Recebe da esquerda e direita
      if (left >= 0) MPI_Recv(&u[0], 1, MPI_FLOAT, left, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      else u[0] = u[1];

      if (right < size) MPI_Recv(&u[local_n + 1], 1, MPI_FLOAT, right, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      else u[local_n + 1] = u[local_n];

      // Envia para esquerda e direita
      if (left >= 0) MPI_Send(&u[1], 1, MPI_FLOAT, left, 0, MPI_COMM_WORLD);
      if (right < size) MPI_Send(&u[local_n], 1, MPI_FLOAT, right, 1, MPI_COMM_WORLD);
    }

    // Atualiza as células reais
    for (int i = 1; i <= local_n; i++) {
      u_new[i] = u[i] + ALPHA * (u[i - 1] - 2 * u[i] + u[i + 1]);
    }

    memcpy(&u[1], &u_new[1], local_n * sizeof(float));
  }
  double end_time = MPI_Wtime();

  if (rank == 0){
    printf("Tempo de execução com barra total tamanho %d e %d passos de tempo: %f segundos\n",
           N, STEPS, end_time - start_time);
  }

  // Impressão final dos resultados por processo
  // for (int p = 0; p < size; p++) {
  //   MPI_Barrier(MPI_COMM_WORLD);
  //   if (rank == p) {
  //     printf("Processo %d (rank %d):\n", p, rank);
  //     for (int i = 1; i <= local_n; i++) {
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
