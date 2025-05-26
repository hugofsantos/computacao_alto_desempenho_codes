#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

#define BASE_NX 100
#define NY 100
#define NZ 100
#define STEPS 100
#define DX 0.01
#define DY 0.01
#define DZ 0.01
#define DT 0.0001
#define NU 0.1 // viscosidade

#define INDEX(i, j, k) ((i) * NY * NZ + (j) * NZ + (k))

void apply_diffusion_3d(double *u, double *u_new, int NX, int numThreads){
#pragma omp parallel for num_threads(numThreads) collapse(3)
  for (int i = 1; i < NX - 1; i++){
    for (int j = 1; j < NY - 1; j++){
      for (int k = 1; k < NZ - 1; k++){
        int idx = INDEX(i, j, k);
        u_new[idx] = u[idx] + NU * DT * ((u[INDEX(i + 1, j, k)] - 2 * u[idx] + u[INDEX(i - 1, j, k)]) / (DX * DX) + (u[INDEX(i, j + 1, k)] - 2 * u[idx] + u[INDEX(i, j - 1, k)]) / (DY * DY) + (u[INDEX(i, j, k + 1)] - 2 * u[idx] + u[INDEX(i, j, k - 1)]) / (DZ * DZ));
      }
    }
  }
}

double run_simulation(double *u, double *u_new, int NX, int numThreads){
  struct timeval start, end;
  double elapsed = 0.0;

  for (int step = 0; step < STEPS; step++){
    gettimeofday(&start, NULL);
    apply_diffusion_3d(u, u_new, NX, numThreads);
    gettimeofday(&end, NULL);

    elapsed += (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    // Copia u_new para u
    #pragma omp parallel for num_threads(numThreads) collapse(3)
    for (int i = 0; i < NX; i++){
      for (int j = 0; j < NY; j++){
        for (int k = 0; k < NZ; k++){
          u[INDEX(i, j, k)] = u_new[INDEX(i, j, k)];
        }
      }
    }
  }

  return elapsed;
}

int main(){
  int threads[] = {1, 2, 4};
  int multipliers[] = {1, 2, 4};

  printf("Threads\tMulti\tNX\tTempo (s)\n");
  for (int t = 0; t < sizeof(threads) / sizeof(threads[0]); t++){
    for (int m = 0; m < sizeof(multipliers) / sizeof(multipliers[0]); m++){
      int numThreads = threads[t];
      int multiplier = multipliers[m];
      int NX = BASE_NX * multiplier;

      size_t total_size = (size_t)NX * NY * NZ;

      double *u = calloc(total_size, sizeof(double));
      double *u_new = calloc(total_size, sizeof(double));

      if (!u || !u_new){
        fprintf(stderr, "Erro: Falha na alocação de memória para NX=%d\n", NX);
        free(u);
        free(u_new);
        continue;
      }

      // Perturbação no centro
      int cx = NX / 2;
      int cy = NY / 2;
      int cz = NZ / 2;
      u[INDEX(cx, cy, cz)] = 1.0;

      double elapsed = run_simulation(u, u_new, NX, numThreads);
      printf("%d\t%d\t%d\t%.6f\n", numThreads, multiplier, NX, elapsed);

      free(u);
      free(u_new);
    }
  }

  return 0;
}
