#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void fillVector(int *vector, int size) {
  for (int i = 0; i < size; i++) {
    vector[i] = (i%100);
  }
}

int sum_1(int *vector, int size) {
  int sum = 0;

  for (int i = 0; i < size; i++){
    sum+=vector[i];
  }

  return sum;
}

int sum_2(int *vector, int size)
{
  int s1 = 0, s2 = 0, s3 = 0, s4 = 0;

  for (int i = 0; i < size; i += 4){
    s1 += vector[i];
    if (i + 1 < size) s2 += vector[i + 1];
    if (i + 2 < size) s3 += vector[i + 2];
    if (i + 3 < size) s4 += vector[i + 3];
  }
  return s1 + s2 + s3 + s4;
}

int main(int argc, char *argv[]){
  if (argc != 2){
    printf("Uso: %s <tamanho do vetor>\n", argv[0]);
    return 1;
  }

  const int SIZE = atoi(argv[1]);
  int *vector = malloc(sizeof(int) * SIZE);

  if(vector == NULL) {
    printf("Erro ao alocar memória para o vetor");
    return 1;
  }

  fillVector(vector, SIZE);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  int sum1 = sum_1(vector, SIZE);
  gettimeofday(&end, NULL);

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
  printf("Tempo de execução da soma 1: %f segundos\n", elapsed_time);

  gettimeofday(&start, NULL);
  int sum2 = sum_2(vector, SIZE);
  gettimeofday(&end, NULL);

  elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
  printf("Tempo de execução da soma 2: %f segundos\n", elapsed_time);

  printf("SOMA 1: %d\n", sum1);
  printf("SOMA 2: %d", sum2);

  free(vector);

  return 0;
}