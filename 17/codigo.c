#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void fill_matrix(double *A, int rows, int cols){
  for (int i = 0; i < rows * cols; i++)
    A[i] = (double)(rand() % 10);
}

void fill_vector(double *x, int size){
  for (int i = 0; i < size; i++)
    x[i] = (double)(rand() % 10);
}

int main(int argc, char *argv[]){
  int rank, size;
  int M = 8; // Número total de linhas da matriz
  int N = 4; // Número de colunas da matriz = tamanho do vetor x

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // A restrição agora é no número de colunas
  if (N % size != 0){
    if (rank == 0)
      fprintf(stderr, "N (%d) deve ser divisível pelo número de processos (%d)\n", N, size);
    MPI_Finalize();
  }

  int local_cols = N / size; // Número de colunas por processo

  double *A = NULL;
  double *x = NULL;
  double *y = NULL;

  // Alocação da parte local da matriz A. É contígua.
  double *local_A = (double *)malloc(M * local_cols * sizeof(double));
  // Alocação da parte local do vetor x.
  double *local_x = (double *)malloc(local_cols * sizeof(double));
  // Cada processo calcula uma contribuição parcial para o y completo.
  double *local_y = (double *)calloc(M, sizeof(double)); // Inicializa com zeros

  if (rank == 0){
    A = (double *)malloc(M * N * sizeof(double));
    x = (double *)malloc(N * sizeof(double));
    y = (double *)malloc(M * sizeof(double));
    srand(time(NULL));
    fill_matrix(A, M, N);
    fill_vector(x, N);
  }

  // --- Criação do tipo de dado derivado para as colunas ---
  MPI_Datatype col_type, resized_col_type;

  // Cria um tipo 'vetor' que seleciona 'local_cols' colunas.
  // Uma coluna tem M elementos, com um passo (stride) de N elementos entre eles.
  MPI_Type_vector(M,          // count: M elementos por coluna
                  local_cols, // blocklength: cada processo recebe um bloco de 'local_cols' colunas
                  N,          // stride: distância entre elementos de uma mesma coluna (é o total de colunas)
                  MPI_DOUBLE, // oldtype
                  &col_type); // newtype
  MPI_Type_commit(&col_type);

  // Redimensiona o tipo para que seu 'extent' (tamanho aparente) seja o de 'local_cols' doubles.
  // Isso é crucial para o MPI_Scatter calcular corretamente o deslocamento para o próximo bloco.
  MPI_Type_create_resized(col_type,
                          0,                           // lower bound
                          local_cols * sizeof(double), // novo extent
                          &resized_col_type);          // tipo redimensionado
  MPI_Type_commit(&resized_col_type);

  // Medição de tempo começa aqui
  double start_time = MPI_Wtime();

  // 1. Distribui os blocos de colunas da matriz A
  // O processo 0 envia usando o tipo derivado; os outros recebem como um bloco contíguo.
  MPI_Scatter(A, 1, resized_col_type,
              local_A, M * local_cols, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  // 2. Distribui os segmentos correspondentes do vetor x
  MPI_Scatter(x, local_cols, MPI_DOUBLE,
              local_x, local_cols, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  // 3. Cálculo Paralelo: Cada processo calcula sua contribuição para y
  for (int i = 0; i < M; i++){
    for (int j = 0; j < local_cols; j++){
      // y[i] += A[i][j] * x[j] (versão local)
      local_y[i] += local_A[i * local_cols + j] * local_x[j];
    }
  }

  // 4. Agregação de Resultados com MPI_Reduce
  // Soma os vetores 'local_y' de todos os processos e armazena o resultado em 'y' no processo 0.
  MPI_Reduce(local_y, y, M, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  double end_time = MPI_Wtime();
  double elapsed = end_time - start_time;

  // --- Impressão e Limpeza ---
  if (rank == 0){
    printf("Tempo total de execução com %d processos (dist. por colunas) e Matriz %dX%d: %f segundos\n",
           size, M, N, elapsed);

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
  free(local_x);
  free(local_y);
  if (rank == 0){
    free(A);
    free(x);
    free(y);
  }

  MPI_Type_free(&col_type);
  MPI_Type_free(&resized_col_type);
  MPI_Finalize();
  return 0;
}