#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

double estimativa_pi(int pontos_total)
{
  int *acertos_por_thread = NULL;
  int num_threads;

  #pragma omp parallel
  {
    // Garantir que apenas uma thread aloque o vetor e obtenha o número de threads
    #pragma omp single
    {
      num_threads = omp_get_num_threads();                          // Obter o número correto de threads
      acertos_por_thread = (int *)calloc(num_threads, sizeof(int)); // Alocar o vetor
    }

    int tid = omp_get_thread_num();
    unsigned int seed = time(NULL) ^ tid;

    #pragma omp for
    for (int i = 0; i < pontos_total; i++)
    {
      double x = (double)rand_r(&seed) / RAND_MAX;
      double y = (double)rand_r(&seed) / RAND_MAX;

      if (x * x + y * y <= 1.0)
      {
        acertos_por_thread[tid] +=1;
      }
    }
  }

  // Soma serial após a região paralela
  int pontos_dentro = 0;
  for (int i = 0; i < num_threads; i++)
  {
    pontos_dentro += acertos_por_thread[i];
  }

  free(acertos_por_thread); // Liberar a memória alocada

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
