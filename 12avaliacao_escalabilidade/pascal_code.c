#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "./pascal-releases-master/include/pascalops.h"

#define BASE_NX 50
#define NY 50
#define NZ 50
#define STEPS 100
#define DX 0.01
#define DY 0.01
#define DZ 0.01
#define DT 0.0001
#define NU 0.1 // viscosidade

#define INDEX(i, j, k) ((i) * NY * NZ + (j) * NZ + (k))

// Aplica a equação de difusão 3D (formato 1D)
void apply_diffusion_3d(double *u, double *u_new, int NX){
  #pragma omp parallel for collapse(3)
  for (int i = 1; i < NX - 1; i++){
    for (int j = 1; j < NY - 1; j++){
      for (int k = 1; k < NZ - 1; k++){
        int idx = INDEX(i, j, k);
        u_new[idx] = u[idx] + NU * DT * ((u[INDEX(i + 1, j, k)] - 2 * u[idx] + u[INDEX(i - 1, j, k)]) / (DX * DX) + (u[INDEX(i, j + 1, k)] - 2 * u[idx] + u[INDEX(i, j - 1, k)]) / (DY * DY) + (u[INDEX(i, j, k + 1)] - 2 * u[idx] + u[INDEX(i, j, k - 1)]) / (DZ * DZ));
      }
    }
  }
}

// Loop principal da simulação
void run_simulation(double *u, double *u_new, int NX){
  for (int step = 0; step < STEPS; step++){
    apply_diffusion_3d(u, u_new, NX);

    // Copia u_new para u
    #pragma omp parallel for collapse(3)
    for (int i = 0; i < NX; i++){
      for (int j = 0; j < NY; j++){
        for (int k = 0; k < NZ; k++){
          u[INDEX(i, j, k)] = u_new[INDEX(i, j, k)];
        }
      }
    }
  }
}

int main(int argc, char *argv[]){
  if (argc != 3){
    fprintf(stderr, "Uso: %s <numero_de_threads> <multiplicador_do_problema> \n", argv[0]);
    return 1;
  }

  int numThreads = atoi(argv[1]);
  int problemMultiplier = atoi(argv[2]);

  if (problemMultiplier < 1 || numThreads < 1){
    fprintf(stderr, "Erro: O multiplicador do problema deve ser >= 1 e num_threads >= 1.\n");
    return 1;
  }

  omp_set_num_threads(numThreads);

  size_t NX = BASE_NX * problemMultiplier;
  size_t total_size = NX * NY * NZ;
  double *u = calloc(total_size, sizeof(double));
  double *u_new = calloc(total_size, sizeof(double));

  if (!u || !u_new){
    fprintf(stderr, "Erro: Falha na alocação de memória.\n");
    return 1;
  }

  // Perturbação inicial no centro do domínio
  int cx = NX / 2;
  int cy = NY / 2;
  int cz = NZ / 2;
  u[INDEX(cx, cy, cz)] = 1.0;

  run_simulation(u, u_new, NX);

  free(u);
  free(u_new);

  return 0;
}
