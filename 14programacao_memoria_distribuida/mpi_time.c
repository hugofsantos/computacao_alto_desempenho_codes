#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv){
  int rank, size;
  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size != 2){
    if (rank == 0){
      printf("Este programa requer exatamente 2 processos.\n");
    }
    MPI_Finalize();
    return 0;
  }

  const int N = 1000;    // número de trocas
  const int TAM = 2; // tamanho do array de inteiros a ser enviado

  int *dados = malloc(TAM * sizeof(int));
  int *buffer = malloc(TAM * sizeof(int));

  double start_time, end_time;

  if (rank == 0){
    start_time = MPI_Wtime();

    for (int i = 0; i < N; i++){
      MPI_Send(dados, TAM, MPI_INT, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(buffer, TAM, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    end_time = MPI_Wtime();

    printf(
      "Processo 0: Tempo total para %d trocas de %d inteiros (%lu bytes) = %f segundos\n",
       N, 
       TAM, 
       TAM * sizeof(int),
       end_time - start_time
    );
  }
  else if (rank == 1){
    for (int i = 0; i < N; i++){
      MPI_Recv(buffer, TAM, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(buffer, TAM, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }

  free(dados);
  free(buffer);

  MPI_Finalize();
  return 0;
}
