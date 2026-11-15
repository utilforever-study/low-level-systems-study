#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Core types from the exercise (keep these) */
typedef struct Node {
    void* data;
    size_t size;
    struct Node* next;
} Node;

/* TODO: implement create_node */
Node* create_node(const void* data, size_t size)
{
    Node* newNode = (Node*) malloc(sizeof(Node));
    newNode->data = data;
    newNode->size = size;
    return newNode;
}

/* TODO: implement prepend */
void prepend(Node** head, const void* data, size_t size)
{
    Node* node = create_node(data, size);
    node->next = *head;
    *head = node;
}

/* TODO: implement append */
void append(Node** head, const void* data, size_t size)
{
    if (*head == NULL)
    {
        prepend(head, data, size);
        return;
    }

    Node* node = create_node(data, size);

    Node* tail;
    for(tail = *head; tail->next != NULL; tail = tail->next) { }
    
    tail->next = node;
    node->data = data;
    node->size = size;
}

/* TODO: implement find */
Node* find(Node* head, const void* key, int (*cmp)(const void*, const void*))
{
    for(Node* node = head; node != NULL; node = node->next)
    {
        if (cmp(key, node->data) == 0)
        {
            return node;
        }
    }

    return NULL;
}

/* TODO: implement free_list */
void free_list(Node** head)
{
    while (*head != NULL)
    {
        Node* next = (*head)->next;
        (*head)->next = NULL;
        free(*head);

        *head = next;
    }
}

/* TODO: implement cmp_int */
int cmp_int(const void* a, const void* b)
{
    int va = *(int*)a;
    int vb = *(int*)b;
    
    if(va < vb) return -1;
    if(va > vb) return 1;
    return 0;
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }

    if (strcmp(mode, "prepend") == 0)
    {
        Node* head = NULL;
        int arr[] = {6, 7, 8, 9, 10};
        for (int i = 0; i < 5; i++)
        {
            prepend(&head, arr + i, sizeof(int));
        }
        
        printf("Prepended:");
        Node* curr = head;
        while (curr)
        {
            printf(" %d", *(int*)curr->data);
            curr = curr->next;
        }
        printf("\n");
        free_list(&head);
    }

    if (strcmp(mode, "append") == 0)
    {
        Node* head = NULL;
        int arr[] = {3, 1, 4, 1, 5};
        for (int i = 0; i < 5; i++)
        {
            append(&head,  arr + i, sizeof(int));
        }
        
        printf("Appended:");
        Node* curr = head;
        while (curr)
        {
            printf(" %d", *(int*)curr->data);
            curr = curr->next;
        }
        printf("\n");
        free_list(&head);
    }

    if (strcmp(mode, "find") == 0)
    {
        Node* head = NULL;
        int arr[] = {3, 1, 4, 1, 5};
        for (int i = 0; i < 5; i++)
        {
            append(&head, arr + i, sizeof(int));
        }
        
        int key = 4;
        Node* res = find(head, &key, cmp_int);
        if (res)
            printf("Found: %d\n", *(int*)res->data);
        else
            printf("Found: none\n");

        free_list(&head);
    }

    if (strcmp(mode, "free") == 0)
    {
        Node* head = NULL;
        int val = 42;
        append(&head, &val, sizeof(int));
        free_list(&head);

        if (head == NULL)
            printf("Memory freed successfully\n");
    }

    if (strcmp(mode, "types") == 0)
    {
        Node* head = NULL;
        int val = 42;
        float fval = 3.5f;
        append(&head, &val, sizeof(int));
        append(&head, &fval, sizeof(float));

        int count = 0;
        Node* curr = head;
        while (curr)
        {
            count++;
            curr = curr->next;
        }
        printf("Works with int and float\nnodes=%d\n", count);
        free_list(&head);
    }

    return 0;
}