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
void print_list(Node *head, const char *name){
  printf("%s: ", name);
  Node *current = head;
  while (current != NULL){
    printf("%d -> ", current->value);
    current = current->next;
  }
  printf("NULL\n");
}

// Lógica principal de criação de tarefas e inserção com regiões críticas nomeadas
void populate_lists_with_tasks(int N){
  Node *list1 = NULL;
  Node *list2 = NULL;

  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int i = 0; i < N; i++){
        #pragma omp task firstprivate(i)
        {
          unsigned int seed = time(NULL) ^ omp_get_thread_num() ^ i;
          int chosen_list = rand_r(&seed) % 2;

          if (chosen_list == 0){
            #pragma omp critical(list1)
            insert(&list1, chosen_list);
          } else {
            #pragma omp critical(list2)
            insert(&list2, chosen_list);
          }
        }
      }
    }
  }

  print_list(list1, "Lista 1");
  print_list(list2, "Lista 2");
  free(list1);
  free(list2);
}

int main(int argc, char *argv[]){
  if (argc != 2)
  {
    fprintf(stderr, "Uso: %s <quantidade_de_insercoes>\n", argv[0]);
    return 1;
  }

  int insertions = atoi(argv[1]);

  if (insertions <= 0){
    fprintf(stderr, "Erro: o número de inserções deve ser positivo.\n");
    return 1;
  }

  populate_lists_with_tasks(insertions);

  return 0;
}
