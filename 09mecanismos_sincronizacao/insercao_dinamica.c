#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// Estrutura de um nó da lista
typedef struct Node
{
  int value;
  struct Node *next;
} Node;

// Função para inserir na lista (no início)
void insert(Node **head, int value)
{
  Node *new_node = (Node *)malloc(sizeof(Node));
  new_node->value = value;
  new_node->next = *head;
  *head = new_node;
}

// Função para imprimir uma lista
void print_list(Node *head, const char *name, int id)
{
  printf("%s %d: ", name, id);
  Node *current = head;
  while (current != NULL)
  {
    printf("%d -> ", current->value);
    current = current->next;
  }
  printf("NULL\n");
}

void populate_lists_with_tasks(int N, int num_lists){
  // Alocação dinâmica de listas e locks
  Node **lists = malloc(num_lists * sizeof(Node *));
  omp_lock_t *locks = malloc(num_lists * sizeof(omp_lock_t));

  // Inicializa listas e locks
  for (int i = 0; i < num_lists; i++){
    lists[i] = NULL;
    omp_init_lock(&locks[i]);
  }

  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int i = 0; i < N; i++){
        #pragma omp task firstprivate(i)
        {
          unsigned int seed = time(NULL) ^ (omp_get_thread_num() + i);
          int chosen_list = rand_r(&seed) % num_lists;

          omp_set_lock(&locks[chosen_list]);
          insert(&lists[chosen_list], chosen_list); 
          omp_unset_lock(&locks[chosen_list]);
        }
      }
    }
  }

  // Imprime todas as listas
  for (int i = 0; i < num_lists; i++)
  {
    print_list(lists[i], "lista", i);
  }

  // Libera memória e destrói locks
  for (int i = 0; i < num_lists; i++){
    Node *tmp;
    while (lists[i]){
      tmp = lists[i];
      lists[i] = lists[i]->next;
      free(tmp);
    }
    omp_destroy_lock(&locks[i]);
  }

  free(lists);
  free(locks);
}

int main(int argc, char *argv[]){
  if (argc != 3){
    fprintf(stderr, "Uso: %s <quantidade_de_insercoes> <numero_de_listas>\n", argv[0]);
    return 1;
  }

  int insertions = atoi(argv[1]);
  int num_lists = atoi(argv[2]);

  if (insertions <= 0 || num_lists <= 0){
    fprintf(stderr, "Erro: os valores devem ser positivos.\n");
    return 1;
  }

  populate_lists_with_tasks(insertions, num_lists);

  return 0;
}
