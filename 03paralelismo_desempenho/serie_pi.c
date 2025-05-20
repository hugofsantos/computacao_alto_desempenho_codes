#include <stdio.h>
#include <sys/time.h>
#include <math.h>

double leibniz_pi(int n_terms){ // PI/4 = 1 - 1/3 + 1/5 - 1/7 + 1/9...
  double pi_approx = 0.0;
  for (int k = 0; k < n_terms; k++){
    pi_approx += (k % 2 == 0 ? 1.0 : -1.0) / (2 * k + 1);
  }
  return 4 * pi_approx; 
}

int main(int argc, char *argv[]){
  if (argc != 2){
    printf("Uso: %s <número de iterações>\n", argv[0]);
    return 1;
  }

  int iterations = atoi(argv[1]);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  double PI = leibniz_pi(iterations);
  gettimeofday(&end, NULL);

  double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
  printf("Tempo de execução da série: %f segundos\n", elapsed_time);
  printf("π calculado= %.15f\n", PI);
  printf("Valor real de π: %.15f\n", M_PI);
  printf("Erro absoluto: %.15f\n", fabs(M_PI - PI));

  return 0;
}