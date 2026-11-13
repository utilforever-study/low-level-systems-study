#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): prepend, append, find, free, types
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* Core types from the exercise (keep these) */
typedef struct Node {
    void* data;
    size_t size;
    struct Node* next;
} Node;

Node* create_node(void *data, size_t size) {
    Node* node = (Node*) malloc(sizeof(struct Node));
    if(node == NULL) return NULL;

    node->data = malloc(size);
    if(node->data == NULL) {
        free(node); // important!
        return NULL;
    }
    
    memcpy(node->data, data, size);
    node->size = size;
    node->next = NULL;
    return node;
}
/* TODO: implement prepend */
Node* prepend(Node* head, void* data, int size) {
    // return the pointer of newly added node
    Node* node = create_node(data, size);
    if(node == NULL) return NULL;
    node->next = head;
    return node;
}

/* TODO: implement free_list */
void free_list(Node** head) {
    Node* cur = *head;
    while(cur != NULL) {
        Node* nxt = cur->next;
        free(cur->data);
        free(cur);
        cur = nxt;
    }
    *head = NULL;
}

/* TODO: implement append */
Node* append(Node* head, void* data, int size) {
    // return the pointer of newly added node
    Node* node = create_node(data, size);
    if(node == NULL) return NULL;
    if(head) head->next = node;
    return node;
}

/* TODO: implement find */
int cmp_int(void* l, void* r) {
    // returns 0 on match.
    return *(int*)l != *(int*)r;
}
Node* find(Node* head, void* data, int (*cmp)(const void*, const void*)) {
    while(head) {
        if(head->data != NULL && cmp(head->data, data)==0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}
/* TODO: implement print_int_list */
void print_int_list(Node* node) {
    while(node) {
        printf("%d ", *((int*)node->data));
        node=node->next;
    }
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "prepend" (sample) and other modes */
    if(strcmp(mode, "prepend") == 0) {
        int data[] = {6,7,8,9,10};
        Node* head = NULL;
        for(int i=0; i<5; i++) {
            head = prepend(head, data+i, sizeof(int));
        }
        printf("Prepended: ");
        print_int_list(head);
    }
    if(strcmp(mode, "append") == 0) {
        int data[] = {3, 1, 4, 1, 5};
        Node* head = create_node(data, sizeof(int));
        Node* cur = head;
        for(int i=1; i<5; i++) {
            cur = append(cur, data+i, sizeof(int));
        }
        printf("Appended: ");
        print_int_list(head);
    }
    if(strcmp(mode, "find") == 0) {
        int data[] = {3, 1, 4, 1, 5};
        Node* head = create_node(data, sizeof(int));
        Node* cur = head;
        for(int i=1; i<5; i++) {
            cur = append(cur, data+i, sizeof(int));
        }
        void* target = data+2;
        Node* node = find(head, target, cmp_int);
        if(node != NULL) {
            printf("Found: %d\n", *(int*)node->data);
        } else {
            printf("Found: none\n");
        }
    }
    if(strcmp(mode, "free") == 0) {
        int data[] = {1, 2};
        Node* head = create_node(data, sizeof(int));
        append(head, data+1, sizeof(int));
        free_list(&head);
        if(head == NULL) {
            printf("Memory freed successfully\n");
        }
    }
    if(strcmp(mode, "types") == 0) {
        int first = 42;
        float second = 3.5;
        Node* head = create_node(&first, sizeof(int));
        Node* appended = append(head, &second, sizeof(float));
        if(head!=NULL && appended!=NULL) {
            printf("Works with int and float\n");
            printf("nodes=2\n");
        }
    }
    return 0;
}