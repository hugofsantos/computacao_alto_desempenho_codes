#include <iostream>
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

// --- NOVAS CONSTANTES PARA A OTIMIZAÇÃO ---
#define BLOCK_DIM 8 // Dimensão do nosso bloco de threads (8x8x8)
#define RADIUS 1    // Raio do stencil (1 para vizinhos diretos)

// --- KERNEL ANTIGO REMOVIDO ---
// O kernel __global__ void atualiza(...) foi substituído pelo abaixo.

// --- NOVO KERNEL OTIMIZADO COM MEMÓRIA COMPARTILHADA ---
__global__ void atualiza_shared(double *vnew, double *vold,
                                int nx, int ny, int nz, double alpha){
  // --- Declaração da Memória Compartilhada ("Bancada de Trabalho" 3D) ---
  // Um cubo de (8+2*1) x (8+2*1) x (8+2*1) = 10x10x10
  __shared__ double tile[BLOCK_DIM + 2 * RADIUS][BLOCK_DIM + 2 * RADIUS][BLOCK_DIM + 2 * RADIUS];

  // Índices da thread dentro do bloco (0 a 7)
  int tx = threadIdx.x;
  int ty = threadIdx.y;
  int tz = threadIdx.z;

  // Índices globais do ponto que esta thread irá calcular
  int gx = blockIdx.x * BLOCK_DIM + tx;
  int gy = blockIdx.y * BLOCK_DIM + ty;
  int gz = blockIdx.z * BLOCK_DIM + tz;
  long long gidx = (long long) gz * ny * nx + (long long) gy * nx + gx;

  // Índices para escrever na memória compartilhada (com a borda/halo)
  int tile_x = tx + RADIUS;
  int tile_y = ty + RADIUS;
  int tile_z = tz + RADIUS;

  // --- Fase 1: Carregar dados da memória GLOBAL para a COMPARTILHADA ---
  // Cada thread carrega seu ponto principal para o centro do "tile"
  if (gx < nx && gy < ny && gz < nz){
    tile[tile_z][tile_y][tile_x] = vold[gidx];
  }

  // Threads nas bordas do bloco carregam o "halo"
  // Halo no eixo X
  if (tx < RADIUS && gx >= RADIUS)
    tile[tile_z][tile_y][tile_x - RADIUS] = vold[gidx - RADIUS];
  if (tx >= BLOCK_DIM - RADIUS && gx + RADIUS < nx)
    tile[tile_z][tile_y][tile_x + RADIUS] = vold[gidx + RADIUS];

  // Halo no eixo Y
  if (ty < RADIUS && gy >= RADIUS)
    tile[tile_z][tile_y - RADIUS][tile_x] = vold[gidx - nx];
  if (ty >= BLOCK_DIM - RADIUS && gy + RADIUS < ny)
    tile[tile_z][tile_y + RADIUS][tile_x] = vold[gidx + nx];

  // Halo no eixo Z
  if (tz < RADIUS && gz >= RADIUS)
    tile[tile_z - RADIUS][tile_y][tile_x] = vold[gidx - (long long)nx * ny];
  if (tz >= BLOCK_DIM - RADIUS && gz + RADIUS < nz)
    tile[tile_z + RADIUS][tile_y][tile_x] = vold[gidx + (long long)nx * ny];

  // --- Fase 2: Sincronizar todas as threads do bloco ---
  __syncthreads();

  // --- Fase 3: Calcular usando dados da memória COMPARTILHADA ---
  if (gx > 0 && gx < nx - 1 && gy > 0 && gy < ny - 1 && gz > 0 && gz < nz - 1){
    double center_val = tile[tile_z][tile_y][tile_x];

    double sum_neighbors = tile[tile_z][tile_y][tile_x + 1] + tile[tile_z][tile_y][tile_x - 1] +
                           tile[tile_z][tile_y + 1][tile_x] + tile[tile_z][tile_y - 1][tile_x] +
                           tile[tile_z + 1][tile_y][tile_x] + tile[tile_z - 1][tile_y][tile_x];

    vnew[gidx] = center_val + alpha * (sum_neighbors - 6.0 * center_val);
  }
}

// Função de validação (sem alterações)
double calcular_soma_centro(const double *u, int nx, int ny, int nz, int tamanho_bloco){
  int start_x = (nx / 2) - (tamanho_bloco / 2);
  int start_y = (ny / 2) - (tamanho_bloco / 2);
  int start_z = (nz / 2) - (tamanho_bloco / 2);
  double soma = 0.0;
  for (int z = start_z; z < start_z + tamanho_bloco; z++)
  {
    for (int y = start_y; y < start_y + tamanho_bloco; y++)
    {
      for (int x = start_x; x < start_x + tamanho_bloco; x++)
      {
        long long idx = (long long)z * ny * nx + (long long)y * nx + x;
        soma += u[idx];
      }
    }
  }
  return soma;
}

// --- Programa Principal (Host) ---
int main(){
  // --- 1. Configuração Inicial (sem alterações) ---
  const int nx = NX, ny = NY, nz = NZ;
  const int nt = STEPS;
  printf("Iniciando simulação em GPU com CUDA (OTIMIZADO com __shared__)\n"); // Título atualizado
  printf("Grade: %d x %d x %d, Passos: %d\n", nx, ny, nz, nt);
  const double alpha = NU * DT / (DX * DX);
  const size_t num_elements = (size_t)nx * ny * nz;
  const size_t size_bytes = num_elements * sizeof(double);

  // --- 2. Alocação de Memória no Host (sem alterações) ---
  double *h_vold, *h_vfinal;
  h_vold = (double *)calloc(num_elements, sizeof(double));
  h_vfinal = (double *)malloc(size_bytes);
  if (h_vold == NULL || h_vfinal == NULL)
  {
    fprintf(stderr, "Erro: Falha na alocação de memória do host.\n");
    free(h_vold);
    free(h_vfinal);
    return 1;
  }

  // --- 3. Condição Inicial (sem alterações) ---
  printf("Definindo condição inicial no host...\n");
  int cx = nx / 2, cy = ny / 2, cz = nz / 2;
  h_vold[cz * ny * nx + cy * nx + cx] = 1.0;

  // --- 4. Alocação de Memória na GPU (sem alterações) ---
  printf("Alocando memória na GPU...\n");
  double *d_vold, *d_vnew;
  cudaMalloc(&d_vold, size_bytes);
  cudaMalloc(&d_vnew, size_bytes);

  // --- 5. Cópia dos Dados Iniciais para a GPU (sem alterações) ---
  cudaMemcpy(d_vold, h_vold, size_bytes, cudaMemcpyHostToDevice);
  cudaMemcpy(d_vnew, h_vold, size_bytes, cudaMemcpyHostToDevice);

  // --- 6. Definindo a Grade de Execução CUDA ---
  // MUDANÇA: Usando a macro para definir o tamanho do bloco
  dim3 threadsPerBlock(BLOCK_DIM, BLOCK_DIM, BLOCK_DIM);
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

  for (int t = 0; t < nt; t++)
  {
    // MUDANÇA: Chamando o novo kernel otimizado
    atualiza_shared<<<numBlocks, threadsPerBlock>>>(d_vnew, d_vold, nx, ny, nz, alpha);
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

  // --- 8. Cópia dos Resultados de Volta para o Host (sem alterações) ---
  printf("Copiando resultados de volta para o host...\n");
  cudaMemcpy(h_vfinal, d_vold, size_bytes, cudaMemcpyDeviceToHost);

  // --- CÁLCULO DE VERIFICAÇÃO ---
  // MUDANÇA: Usando a macro para consistência no tamanho do bloco de verificação
  printf("Calculando soma de verificação em um bloco de %dx%dx%d...\n", BLOCK_DIM, BLOCK_DIM, BLOCK_DIM);
  double soma_gpu = calcular_soma_centro(h_vfinal, nx, ny, nz, BLOCK_DIM);
  printf("----------------------------------------\n");
  printf("SOMA DE VERIFICAÇÃO (GPU): %.15f\n", soma_gpu);
  printf("----------------------------------------\n");

  // --- 9. Liberação de Recursos (sem alterações) ---
  printf("Liberando memória.\n");
  cudaFree(d_vold);
  cudaFree(d_vnew);
  cudaEventDestroy(start);
  cudaEventDestroy(stop);
  free(h_vold);
  free(h_vfinal);

  return 0;
}