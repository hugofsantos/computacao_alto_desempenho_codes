#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

#define BASE_NX 128
#define NY 128
#define NZ 128
#define STEPS 100
#define DX 0.01
#define DY 0.01
#define DZ 0.01
#define DT 0.0001
#define NU 0.1 // viscosidade

#define INDEX(i, j, k) ((i) * NY * NZ + (j) * NZ + (k))

double run_simulation(double *u, double *u_new, int NX, int numThreads, double alpha){
  struct timeval start, end;
  gettimeofday(&start, NULL);

  // A macro INDEX que você já tem está correta para esse layout
  // #define INDEX(i, j, k) ((i) * NY * NZ + (j) * NZ + (k))

  #pragma omp parallel num_threads(numThreads)
  {
    for (int step = 0; step < STEPS; step++){
      // Aplicar difusão com a mesma fórmula do CUDA
      #pragma omp for collapse(3)
      for (int i = 1; i < NX - 1; i++){
        for (int j = 1; j < NY - 1; j++){
          for (int k = 1; k < NZ - 1; k++){
            int idx = INDEX(i, j, k);

            // Índices dos vizinhos (mesma lógica do kernel CUDA)
            // Note que seu INDEX está (i, j, k) enquanto o do kernel está (z, y, x).
            // Para usar a mesma lógica de +1, -1, o layout de memória precisa ser o mesmo.
            // O layout do kernel (z*ny*nx + y*nx + x) é o padrão C/C++ row-major.
            // O seu INDEX ((i)*NY*NZ + (j)*NZ + (k)) é diferente. Vamos ajustar para ele.
            int xp = INDEX(i + 1, j, k);
            int xm = INDEX(i - 1, j, k);
            int yp = INDEX(i, j + 1, k);
            int ym = INDEX(i, j - 1, k);
            int zp = INDEX(i, j, k + 1);
            int zm = INDEX(i, j, k - 1);

            u_new[idx] = u[idx] + alpha * (u[xp] + u[xm] +
                                           u[yp] + u[ym] +
                                           u[zp] + u[zm] - 6.0 * u[idx]);
          }
        }
      }

      #pragma omp barrier

      #pragma omp for collapse(3)
      for (int i = 0; i < NX; i++){
        for (int j = 0; j < NY; j++){
          for (int k = 0; k < NZ; k++){
            u[INDEX(i, j, k)] = u_new[INDEX(i, j, k)];
          }
        }
      }

      #pragma omp barrier // Adicionar barreira aqui para sincronizar antes do próximo passo
    } // fim do loop de steps
  } // fim da região paralela

  gettimeofday(&end, NULL);
  return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

// Função parametrizada para calcular a soma de um bloco central
double calcular_soma_centro(const double *u, int nx, int ny, int nz, int tamanho_bloco){
  // Calcula as coordenadas de início do cubo central
  int start_x = (nx / 2) - (tamanho_bloco / 2);
  int start_y = (ny / 2) - (tamanho_bloco / 2);
  int start_z = (nz / 2) - (tamanho_bloco / 2);

  double soma = 0.0;

  // Loop triplo para percorrer o cubo central
  for (int z = start_z; z < start_z + tamanho_bloco; z++){
    for (int y = start_y; y < start_y + tamanho_bloco; y++){
      for (int x = start_x; x < start_x + tamanho_bloco; x++){
        long long idx = (long long)z * ny * nx + (long long)y * nx + x;
        soma += u[idx];
      }
    }
  }
  return soma;
}

int main(){
  const int numThreads = 64;
  const int NX = BASE_NX * 1; // Ex: usando um multiplicador de 1x

  printf("Iniciando simulação com parâmetros fixos:\n");
  printf("  - Threads OpenMP: %d\n", numThreads);
  printf("  - Dimensões da grade: %d x %d x %d\n", NX, NY, NZ);
  printf("----------------------------------------\n");

  const size_t total_size = (size_t)NX * NY * NZ;
  double *u = (double *)calloc(total_size, sizeof(double));
  double *u_new = (double *)calloc(total_size, sizeof(double));

  if (!u || !u_new)
  {
    fprintf(stderr, "Erro: Falha na alocação de memória.\n");
    free(u);
    free(u_new);
    return 1; // Retorna um código de erro
  }

  int cx = NX / 2;
  int cy = NY / 2;
  int cz = NZ / 2;
  u[INDEX(cx, cy, cz)] = 1.0;

  double alpha = NU * DT / (DX * DX);

  printf("Executando a simulação da CPU...\n");
  double tempo_decorrido = run_simulation(u, u_new, NX, numThreads, alpha);

  printf("Simulação concluída.\n");
  printf("----------------------------------------\n");
  printf("Tempo de execução: %.6f segundos\n", tempo_decorrido);

  // --- CÁLCULO DE VERIFICAÇÃO ---
  const int block_size_check = 8;
  printf("Calculando soma de verificação em um bloco de %dx%dx%d...\n", block_size_check, block_size_check, block_size_check);
  double soma_cpu = calcular_soma_centro(u_new, NX, NY, NZ, block_size_check);
  printf("----------------------------------------\n");
  printf("SOMA DE VERIFICAÇÃO (CPU): %.15f\n", soma_cpu);
  printf("----------------------------------------\n");

  free(u);
  free(u_new);

  return 0;
}
