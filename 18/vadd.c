#include <stdio.h>
#include <sys/time.h>

#define N 10000000
#define TOL 0.0000001

int main(){

  float a[N], b[N], c[N], res[N];
  int err = 0;

  // fill the arrays
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
  for (int i = 0; i < N; i++)
  {
    c[i] = a[i] + b[i];
  }

  gettimeofday(&end, NULL);

  // test results
  for (int i = 0; i < N; i++)
  {
    float val = c[i] - res[i];
    val = val * val;
    if (val > TOL)
      err++;
  }

  // test_time += omp_get_wtime();
  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

  printf(" vectors added with %d errors\n", err);

  // printf("Init time:    %.3fs\n", init_time);
  // printf("Compute time: %.3fs\n", compute_time);
  printf("Compute time: %fs\n", elapsed_time);
  //printf("Total time:   %.3fs\n", init_time + compute_time + test_time);

  return 0;
}