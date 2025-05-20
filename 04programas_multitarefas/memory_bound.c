#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
#include <stdlib.h>

void fillVector(int *vector, int size){
  for (int i = 0; i < size; i++){
    int n = i < 100 ? i : 100;
    vector[i] = n;
  }
}

int soma_vetores(int size, int* a, int* b, int* c, int* d){
  int sum = 0;

  #pragma omp parallel for num_threads(8)
  for (int i = 0; i < size; i++){
    a[i] = b[i] + c[i] + d[i];
  } 

  return sum;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Uso: %s <número de iterações>\n", argv[0]);
    return 1;
  }

  int iterations = atoi(argv[1]);

  int* a = malloc(sizeof(int) * iterations);
  int *b = malloc(sizeof(int) * iterations);
  int *c = malloc(sizeof(int) * iterations);
  int *d = malloc(sizeof(int) * iterations);

  // Preencher os vetores com valores
  fillVector(a, iterations);
  fillVector(b, iterations);
  fillVector(c, iterations);
  fillVector(d, iterations);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  int result = soma_vetores(iterations, a, b, c, d);
  gettimeofday(&end, NULL);

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

  printf("Tempo de execução: %f segundos\n", elapsed_time);
  return 0;
}