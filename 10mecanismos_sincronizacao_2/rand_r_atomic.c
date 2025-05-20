#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

double estimativa_pi(int pontos_total){
  int pontos_dentro = 0;

  #pragma omp parallel
  {
    int pontos_dentro_local = 0;
    unsigned int seed = time(NULL) ^ omp_get_thread_num(); // Semente única por thread

    #pragma omp for
    for (int i = 0; i < pontos_total; i++)
    {
      double x = (double)rand_r(&seed) / RAND_MAX;
      double y = (double)rand_r(&seed) / RAND_MAX;

      if (x * x + y * y <= 1.0){
        pontos_dentro_local++;
      }
    }

    #pragma omp atomic
    pontos_dentro += pontos_dentro_local;
  }

  return 4.0 * (double)pontos_dentro / pontos_total;
}

int main()
{
  int total = 30000000;

  struct timeval start, end;
  gettimeofday(&start, NULL);
  double pi = estimativa_pi(total);
  gettimeofday(&end, NULL);

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

  printf("Estimativa de PI: %f\n", pi);
  printf("Tempo de execução: %f segundos\n", elapsed_time);

  return 0;
}
