#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LEN 100
#define NUM_FILES 8

// Estrutura do nó da lista encadeada
typedef struct Node
{
  char filename[MAX_FILENAME_LEN];
  struct Node *next;
} Node;

// Função para criar um novo nó
Node *create_node(const char *filename){
  Node *new_node = (Node *)malloc(sizeof(Node));
  if (!new_node)
  {
    perror("Erro ao alocar memória para o nó");
    exit(EXIT_FAILURE);
  }
  strncpy(new_node->filename, filename, MAX_FILENAME_LEN);
  new_node->filename[MAX_FILENAME_LEN - 1] = '\0'; // Garante terminação nula
  new_node->next = NULL;
  return new_node;
}

// Função para adicionar um nó no final da lista
void append_node(Node **head_ref, const char *filename)
{
  Node *new_node = create_node(filename);

  if (*head_ref == NULL)
  {
    *head_ref = new_node;
  }
  else
  {
    Node *temp = *head_ref;
    while (temp->next != NULL)
    {
      temp = temp->next;
    }
    temp->next = new_node;
  }
}

// Função para liberar a memória da lista
void free_list(Node *head)
{
  Node *temp;
  while (head != NULL)
  {
    temp = head;
    head = head->next;
    free(temp);
  }
}

int main()
{
  Node *file_list = NULL;

  // Criando arquivos fictícios
  const char *filenames[NUM_FILES] = {
      "arquivo1.txt",
      "arquivo2.txt",
      "arquivo3.txt",
      "arquivo4.txt",
      "arquivo5.txt",
      "arquivo6.txt",
      "arquivo7.txt",
      "arquivo8.txt"
  };

  // Adicionando arquivos à lista
  for (int i = 0; i < NUM_FILES; i++){
    append_node(&file_list, filenames[i]);
  }

  // Liberando a memória
  free_list(file_list);

  return 0;
}
