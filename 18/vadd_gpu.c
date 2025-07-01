#include <stdio.h>
#include <sys/time.h>
#include <omp.h>

#define N 10000000
#define TOL 0.0000001

int main()
{
  printf("There are % d devices\n", omp_get_num_devices());

  float a[N], b[N], c[N], res[N];
  int err = 0;

  // fill the arrays
  #pragma omp parallel for
  for (int i = 0; i < N; i++)
  {
    a[i] = (float)i;
    b[i] = 2.0 * (float)i;
    c[i] = 0.0;
    res[i] = i + 2 * i;
  }

  struct timeval start, end;
  gettimeofday(&start, NULL);

  // add two vectors
  #pragma omp target
  #pragma omp loop
  for (int i = 0; i < N; i++)
  {
    c[i] = a[i] + b[i];
  }

  gettimeofday(&end, NULL);

// test results
  #pragma omp parallel for reduction(+ : err)
  for (int i = 0; i < N; i++)
  {
    float val = c[i] - res[i];
    val = val * val;
    if (val > TOL)
      err++;
  }

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

  printf(" vectors added with %d errors\n", err);

  printf("Compute time: %fs\n", elapsed_time);
  return 0;
}