#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>
#include "./pascal-releases-master/include/pascalops.h"

#define NX 50
#define NY 50
#define NZ 50
#define STEPS 100
#define DX 0.01
#define DY 0.01
#define DZ 0.01
#define DT 0.0001
#define NU 0.1 // viscosidade

// Aplica a equação de difusão 3D
void apply_diffusion_3d(double u[NX][NY][NZ], double u_new[NX][NY][NZ]){
  #pragma omp parallel for collapse(3) schedule(guided, 1)
  for (int i = 1; i < NX - 1; i++){
    for (int j = 1; j < NY - 1; j++){
      for (int k = 1; k < NZ - 1; k++){
        u_new[i][j][k] = u[i][j][k] + NU * DT * ((u[i + 1][j][k] - 2 * u[i][j][k] + u[i - 1][j][k]) / (DX * DX) + (u[i][j + 1][k] - 2 * u[i][j][k] + u[i][j - 1][k]) / (DY * DY) + (u[i][j][k + 1] - 2 * u[i][j][k] + u[i][j][k - 1]) / (DZ * DZ));
      }
    }
  }
}

// Função que executa toda a simulação (loop de steps + cópia)
void run_simulation(double u[NX][NY][NZ], double u_new[NX][NY][NZ]){
  for (int step = 0; step < STEPS; step++){
    apply_diffusion_3d(u, u_new);

    // Copia u_new para u
    for (int i = 0; i < NX; i++){
      for (int j = 0; j < NY; j++){
        for (int k = 0; k < NZ; k++){
          u[i][j][k] = u_new[i][j][k];
        }
      }
    }
  }
}

int main(){
  double u[NX][NY][NZ] = {{{0}}};
  double u_new[NX][NY][NZ] = {{{0}}};

  // Introduz pequena perturbação no centro
  u[NX / 2][NY / 2][NZ / 2] = 1.0;

  run_simulation(u, u_new); // Simulação

  return 0;
}
