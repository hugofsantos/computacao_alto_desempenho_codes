#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define NX 50
#define NY 50
#define NZ 50
#define STEPS 500
#define DX 0.01
#define DY 0.01
#define DZ 0.01
#define DT 0.0001
#define NU 0.1 // viscosidade

void save_slice_to_csv(double u[NX][NY][NZ], int step, int z_slice){
  char filename[64];
  snprintf(filename, sizeof(filename), "output/vel_step_%04d_z%d.csv", step, z_slice);
  FILE *fp = fopen(filename, "w");

  for (int i = 0; i < NX; i++){
    for (int j = 0; j < NY; j++){
      fprintf(fp, "%.5f", u[i][j][z_slice]);
      if (j < NY - 1) fprintf(fp, ",");
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

// Aplica a equação de difusão 3D
void apply_diffusion_3d(double u[NX][NY][NZ], double u_new[NX][NY][NZ]){
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

    // Descomente se quiser salvar a saída (não incluir isso no tempo!)
    // if (step % 10 == 0)
    //   save_slice_to_csv(u, step, NZ / 2);
  }
}

int main()
{
  double u[NX][NY][NZ] = {{{0}}};
  double u_new[NX][NY][NZ] = {{{0}}};

  //system("mkdir -p output");

  struct timeval start, end;

  gettimeofday(&start, NULL);
  run_simulation(u, u_new); // Simulação
  gettimeofday(&end, NULL);

  double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
  printf("Tempo de simulação: %.6f segundos\n", elapsed);

  return 0;
}
