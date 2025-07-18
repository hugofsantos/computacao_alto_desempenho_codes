#include <iostream>
// #include <vector> // Não precisamos mais deste
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

// --- Definições Globais da Simulação ---
#define NX 128
#define NY 128
#define NZ 128
#define STEPS 100

// Constantes físicas
#define DX 0.01
#define DY 0.01
#define DZ 0.01
#define DT 0.0001
#define NU 0.1

// --- Kernel CUDA (sem alterações) ---
__global__ void atualiza(double *vnew, double *vold,
                         int nx, int ny, int nz, double alpha){
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;
  int z = blockIdx.z * blockDim.z + threadIdx.z;

  if (x > 0 && x < nx - 1 && y > 0 && y < ny - 1 && z > 0 && z < nz - 1){
    int idx = z * ny * nx + y * nx + x;
    int xm = idx - 1;
    int xp = idx + 1;
    int ym = idx - nx;
    int yp = idx + nx;
    int zm = idx - nx * ny;
    int zp = idx + nx * ny;

    vnew[idx] = vold[idx] + alpha * (vold[xp] + vold[xm] +
                                     vold[yp] + vold[ym] +
                                     vold[zp] + vold[zm] - 6.0 * vold[idx]);
  }
}

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

// --- Programa Principal (Host) ---
int main(){
  // --- 1. Configuração Inicial ---
  const int nx = NX, ny = NY, nz = NZ;
  const int nt = STEPS;
  printf("Iniciando simulação em GPU com CUDA (usando malloc/free)\n");
  printf("Grade: %d x %d x %d, Passos: %d\n", nx, ny, nz, nt);

  const double alpha = NU * DT / (DX * DX);
  const size_t num_elements = (size_t) nx * ny * nz;
  const size_t size_bytes = num_elements * sizeof(double);

  // --- 2. Alocação de Memória no Host (CPU) com malloc/calloc ---
  double *h_vold, *h_vfinal;
  // Usamos calloc para h_vold para já inicializar com zeros
  h_vold = (double *)calloc(num_elements, sizeof(double));
  h_vfinal = (double *)malloc(size_bytes);

  // Verificação de erro na alocação
  if (h_vold == NULL || h_vfinal == NULL){
    fprintf(stderr, "Erro: Falha na alocação de memória do host.\n");
    free(h_vold); // Libera o que possa ter sido alocado
    free(h_vfinal);
    return 1;
  }

  // --- 3. Definindo a Condição Inicial ---
  printf("Definindo condição inicial no host...\n");
  // Como usamos calloc, h_vold já está preenchido com zeros.
  // Apenas definimos a perturbação central.
  int cx = nx / 2, cy = ny / 2, cz = nz / 2;
  h_vold[cz * ny * nx + cy * nx + cx] = 1.0;

  // --- 4. Alocação de Memória na GPU (Device) ---
  printf("Alocando memória na GPU...\n");
  double *d_vold, *d_vnew;
  cudaMalloc(&d_vold, size_bytes);
  cudaMalloc(&d_vnew, size_bytes);

  // --- 5. Cópia dos Dados Iniciais para a GPU ---
  cudaMemcpy(d_vold, h_vold, size_bytes, cudaMemcpyHostToDevice);
  cudaMemcpy(d_vnew, h_vold, size_bytes, cudaMemcpyHostToDevice);

  // --- 6. Definindo a Grade de Execução CUDA ---
  const int block_dim_x = 8;
  const int block_dim_y = 8;
  const int block_dim_z = 8;

  dim3 threadsPerBlock(block_dim_x, block_dim_y, block_dim_z);
  dim3 numBlocks((nx + threadsPerBlock.x - 1) / threadsPerBlock.x,
                 (ny + threadsPerBlock.y - 1) / threadsPerBlock.y,
                 (nz + threadsPerBlock.z - 1) / threadsPerBlock.z);
  printf("Configuração CUDA: %d blocos, %d threads/bloco\n", numBlocks.x * numBlocks.y * numBlocks.z, threadsPerBlock.x * threadsPerBlock.y * threadsPerBlock.z);

  // --- 7. Medição de Tempo e Execução do Loop ---
  printf("Iniciando loop de simulação na GPU...\n");
  cudaEvent_t start, stop;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start);

  for (int t = 0; t < nt; t++){
    atualiza<<<numBlocks, threadsPerBlock>>>(d_vnew, d_vold, nx, ny, nz, alpha);
    double *tmp = d_vold;
    d_vold = d_vnew;
    d_vnew = tmp;
  }

  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  float milliseconds = 0;

  cudaEventElapsedTime(&milliseconds, start, stop);
  printf("Simulação na GPU concluída.\n");
  printf("Tempo de execução: %.4f ms (%.6f s)\n", milliseconds, milliseconds / 1000.0);

  // --- 8. Cópia dos Resultados de Volta para o Host ---
  printf("Copiando resultados de volta para o host...\n");
  cudaMemcpy(h_vfinal, d_vold, size_bytes, cudaMemcpyDeviceToHost);

  // --- CÁLCULO DE VERIFICAÇÃO ---
  printf("Calculando soma de verificação em um bloco de %dx%dx%d...\n", block_dim_x, block_dim_y, block_dim_z);
  // Passamos a dimensão do bloco (usamos a do eixo X, assumindo que são iguais)
  double soma_gpu = calcular_soma_centro(h_vfinal, nx, ny, nz, block_dim_x);
  printf("----------------------------------------\n");
  printf("SOMA DE VERIFICAÇÃO (GPU): %.15f\n", soma_gpu);
  printf("----------------------------------------\n");

  // --- 9. Liberação de Recursos ---
  printf("Liberando memória.\n");
  
  // Libera a memória da GPU
  cudaFree(d_vold);
  cudaFree(d_vnew);
  cudaEventDestroy(start);
  cudaEventDestroy(stop);
  // Libera a memória do Host
  free(h_vold);
  free(h_vfinal);

  return 0;
}