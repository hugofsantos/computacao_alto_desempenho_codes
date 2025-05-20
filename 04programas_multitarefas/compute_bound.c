#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

int calculos_complexos(int iterations) {
  double sum = 0;

  #pragma omp parallel for num_threads(8)
  for (int i = 0; i < iterations; i++){
    sum += tan(sin(i) + cos(i)) + sqrt(i) + log(i) + exp(i) + pow(i, 2) + tgamma(i+1);
  } // Conta complexa utilizando apenas o I para não acessar muito a memória

  return sum;
}

int main(int argc, char *argv[]){
  if (argc != 2)
  {
    printf("Uso: %s <número de iterações>\n", argv[0]);
    return 1;
  }

  int iterations = atoi(argv[1]);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  double result = calculos_complexos(iterations);
  gettimeofday(&end, NULL);

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
  
  printf("Tempo de execução: %f segundos\n", elapsed_time);
  return 0;
}