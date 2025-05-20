#include <omp.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

int count_number_primes(int maxNumber){
  int count = 0;

  #pragma omp parallel for
  for (int i = 2; i <= maxNumber; i++){
    int is_prime = 1;
    int limit = (int) floor(sqrt(i)); // Só precisa verificar até a raiz quadrada

    for (int j = 2; j <= limit; j++){
      if (i % j == 0){
        is_prime = 0;
        break;
      }
    }

    if (is_prime)
      count++;
  }

  return count;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Uso: %s <número máximo>\n", argv[0]);
    return 1;
  }

  int maxNumber = atoi(argv[1]);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  int result = count_number_primes(maxNumber);
  gettimeofday(&end, NULL);

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

  printf("Quantidade de primos encontrados: %d", result);
  printf("Tempo de execução: %f segundos\n", elapsed_time);
  return 0;
}