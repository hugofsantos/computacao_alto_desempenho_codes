#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
//TODO: Pesquisar biblioteca chrono com função high_resolution_clock 

void fillTheMatrix(int rows, int cols, int* matrix) {
  for (size_t i = 0; i < rows * cols; i++){
    matrix[i] = (int) ((i+1)%100); // Pra não ficar um número muito grande e estourar
  }
}

void fillTheVector(int size, int *vector){
  for(int i = 0; i < size; i++) {
    vector[i] = (i+1)%100; 
  }
}

void multiplyMatrixVector(int rows, int cols, int *matrix, int *vector, int *result) {
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < cols; j++){
      result[i] += matrix[i*cols + j] * vector[j];
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    printf("Uso: %s <número de linhas> <número de colunas>\n", argv[0]);
    return 1;
  }

  int rows = atoi(argv[1]);
  int cols = atoi(argv[2]);

  if(rows <= 0 || cols <= 0) {
    printf("O valor das linhas ou colunas não podem ser menores ou iguais a 0");
    return 1;
  }

  int* matrix = malloc(sizeof(int) * rows * cols); // Aloca a matriz em um bloco sequencial
  int* vector = malloc(sizeof(int) * cols);
  int *result = calloc(rows, sizeof(int));

  if (matrix == NULL || vector == NULL || result == NULL){
    printf("Erro ao alocar memória\n");
    free(matrix);
    free(vector);
    free(result);
    return 1;
  }

  fillTheMatrix(rows, cols, matrix); 
  fillTheVector(cols, vector);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  multiplyMatrixVector(rows, cols, matrix, vector, result);
  gettimeofday(&end, NULL);

  // printf("Resultado da multiplicação:\n");
  // for (int i = 0; i < rows; i++){
  //   printf("%d \n", result[i]);
  // }

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
  printf("Tempo de execução da multiplicação: %f segundos\n", elapsed_time);

  free(matrix);
  free(vector);
  free(result);

  return 0;
}