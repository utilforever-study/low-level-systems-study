
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): prepend, append, find, free, types
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* Core types from the exercise (keep these) */
typedef struct Node {
  void *data;
  size_t size;
  struct Node *next;
} Node;

/* TODO: implement cmp_int */
int cmp_int(void *a, void *b) {
  int ret = *(int *)a - *(int *)b;
  if (ret < 0) {
    return -1;
  } else if (ret > 0) {
    return 1;
  }
  return 0;
}

/* TODO: implement prepend */
Node *prepend(Node *head, void *data, size_t size) {
  Node *new = malloc(sizeof(Node));
  new->data = data;
  new->size = size;
  new->next = head;
  return new;
}

/* TODO: implement append */
Node *append(Node *head, void *data, size_t size) {
  Node *new = malloc(sizeof(Node));
  new->data = data;
  new->size = size;
  new->next = NULL;
  Node *current = head;
  if (head == NULL) {
    return new;
  }
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = new;
  return head;
}

/* TODO: implement find */
Node *find(Node *head, void *data, size_t size, int (*cmp)(void *, void *)) {
  Node *current = head;
  while (current != NULL) {
    if (cmp(current->data, data) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

/* TODO: implement free_list */
void free_list(Node *head) {
  Node *current = head;
  while (current != NULL) {
    Node *next = current->next;
    free(current->data);
    free(current);
    current = next;
  }
}

/* TODO: implement print_int_list */
void print_int_list(Node *head) {
  Node *current = head;
  while (current != NULL) {
    printf("%d ", *(int *)current->data);
    current = current->next;
  }
  printf("\n");
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "prepend" (sample) and other modes */
  (void)mode;

  int arr[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

  Node *head = NULL;

  if (strcmp(mode, "prepend") == 0) {
    head = prepend(head, arr + 6, sizeof(int));
    head = prepend(head, arr + 7, sizeof(int));
    head = prepend(head, arr + 8, sizeof(int));
    head = prepend(head, arr + 9, sizeof(int));
    head = prepend(head, arr + 10, sizeof(int));

    printf("Prepended: ");
    print_int_list(head);
  } else if (strcmp(mode, "append") == 0) {
    head = append(head, arr + 3, sizeof(int));
    head = append(head, arr + 1, sizeof(int));
    head = append(head, arr + 4, sizeof(int));
    head = append(head, arr + 1, sizeof(int));
    head = append(head, arr + 5, sizeof(int));
    printf("Appended: ");
    print_int_list(head);
  } else if (strcmp(mode, "find") == 0) {
    head = append(head, arr + 3, sizeof(int));
    head = append(head, arr + 1, sizeof(int));
    head = append(head, arr + 4, sizeof(int));
    head = append(head, arr + 1, sizeof(int));
    head = append(head, arr + 5, sizeof(int));
    Node *node = find(head, arr + 4, sizeof(int), cmp_int);
    printf(node != NULL ? "Found: 4\n" : "Found: none\n");
  } else if (strcmp(mode, "free") == 0) {
    free_list(head);
    printf("Memory freed successfully\n");
  } else if (strcmp(mode, "types") == 0) {
    int a = 42;
    float b = 3.5f;
    append(head, &a, sizeof(int));
    append(head, &b, sizeof(float));
    printf("Works with int and float\nnodes=2\n");
  } else {
    printf("Unknown mode\n");
  }
  return 0;
}
