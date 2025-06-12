#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void fill_matrix(double *A, int rows, int cols){
  for (int i = 0; i < rows * cols; i++)
    A[i] = rand() % 10;
}

void fill_vector(double *x, int size){
  for (int i = 0; i < size; i++)
    x[i] = rand() % 10;
}

int main(int argc, char *argv[]){
  int rank, size;
  int M = 8; // número total de linhas da matriz
  int N = 4; // número de colunas da matriz = tamanho do vetor x

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(M % size != 0) {
    if (rank == 0)
      fprintf(stderr, "M (%d) deve ser divisível por número de processos (%d)\n", M, size);
    MPI_Finalize();
  }

  int local_rows = M / size;

  double *A = NULL; // Matriz (só no processo 0)
  double *x = malloc(N * sizeof(double)); // Vetor (em todos os processos)
  double *local_A = malloc(local_rows * N * sizeof(double)); // Parte local da matriz (a matriz está sendo tratada como um grande vetor)
  double *local_y = malloc(local_rows * sizeof(double)); // Parte local do vetor resultante
  double *y = NULL; // Vetor resultante completo (só no processo 0)

  if (rank == 0){
    A = malloc(M * N * sizeof(double));
    y = malloc(M * sizeof(double));
    srand(time(NULL));
    fill_matrix(A, M, N);
    fill_vector(x, N);
  }

  // Medição de tempo começa aqui
  double start_time = MPI_Wtime();

  MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  MPI_Scatter(A, local_rows * N, MPI_DOUBLE,
              local_A, local_rows * N, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  for (int i = 0; i < local_rows; i++){
    local_y[i] = 0.0;
    for (int j = 0; j < N; j++){
      local_y[i] += local_A[i * N + j] * x[j];
    }
  }

  MPI_Gather(local_y, local_rows, MPI_DOUBLE,
             y, local_rows, MPI_DOUBLE,
             0, MPI_COMM_WORLD);

  double end_time = MPI_Wtime();
  double elapsed = end_time - start_time;

  if (rank == 0){
    printf("Tempo total de execução com %d processos e Matriz %dX%d: %f segundos\n",
      size,
      M,
      N,
      elapsed);

    // printf("\nMatriz A:\n");
    // for (int i = 0; i < M; i++){
    //   for (int j = 0; j < N; j++)
    //     printf("%.1f ", A[i * N + j]);
    //   printf("\n");
    // }

    // printf("\nVetor x:\n");
    // for (int i = 0; i < N; i++)
    //   printf("%.1f ", x[i]);
    // printf("\n");

    // printf("\nResultado y = A * x:\n");
    // for (int i = 0; i < M; i++)
    //   printf("%.1f\n", y[i]);
  }

  free(local_A);
  free(local_y);
  free(x);
  if (rank == 0){
    free(A);
    free(y);
  }

  MPI_Finalize();
  return 0;
}
