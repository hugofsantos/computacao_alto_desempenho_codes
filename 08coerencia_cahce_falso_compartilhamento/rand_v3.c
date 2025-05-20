#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

double estimativa_pi(int pontos_total){
  int num_threads;
  int *acertos_por_thread;

  // Inicializa a semente do gerador de números aleatórios
  srand(time(NULL));

  #pragma omp parallel
  {
    int tid = omp_get_thread_num();

    // Alocar o vetor uma única vez por thread 0
    #pragma omp single
    {
      num_threads = omp_get_num_threads();
      acertos_por_thread = (int *)calloc(num_threads, sizeof(int));
    }

    #pragma omp for
    for (int i = 0; i < pontos_total; i++)
    {
      double x = (double)rand() / RAND_MAX;
      double y = (double)rand() / RAND_MAX;

      if (x * x + y * y <= 1.0)
      {
        acertos_por_thread[tid] += 1;
      }
    }
  }

  // Região serial: soma os acertos
  int pontos_dentro = 0;
  for (int i = 0; i < num_threads; i++)
  {
    pontos_dentro += acertos_por_thread[i];
  }

  free(acertos_por_thread);
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
