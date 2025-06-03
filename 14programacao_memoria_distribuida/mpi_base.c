#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
  int rank, size;
  char mensagem[100];

  // Inicializa o ambiente MPI
  MPI_Init(&argc, &argv);

  // Pega o rank (ID) do processo
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Pega o número total de processos
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size != 2){
    if (rank == 0){
      printf("Este programa requer exatamente 2 processos.\n");
    }
    MPI_Finalize();
    return 0;
  }

  if (rank == 0){
    // Processo 0 envia uma mensagem para o processo 1
    strcpy(mensagem, "Olá do processo 0!");
    printf("Processo 0: enviando mensagem para o processo 1...\n");
    MPI_Send(mensagem, strlen(mensagem) + 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);

    // Processo 0 espera resposta do processo 1
    MPI_Recv(mensagem, 100, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Processo 0: recebeu resposta -> %s\n", mensagem);
  } else if (rank == 1) {
    // Processo 1 recebe a mensagem
    MPI_Recv(mensagem, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Processo 1: recebeu mensagem -> %s\n", mensagem);

    // Processo 1 responde com a mesma mensagem
    MPI_Send(mensagem, strlen(mensagem) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    printf("Processo 1: respondeu para o processo 0.\n");
  }

  // Finaliza o ambiente MPI
  MPI_Finalize();
  return 0;
}
