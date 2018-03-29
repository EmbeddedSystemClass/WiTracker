#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Part of some ADT file */
/******************************************************************************/

typedef struct ADT_Node {
    struct ADT_Node *next;
    void *item;
} ADT_Node;

ADT_Node *head;
ADT_Node *tail;
uint16_t size;

void adt_init(void);
void enqueue(void *item);
ADT_Node* dequeue(void);


void adt_init(void) {
    head = NULL;
    tail = NULL;
    size = 0;
}

void enqueue(void *item) {
    if (item == NULL) return;

    ADT_Node *node = malloc(sizeof(ADT_Node));
    node->next = NULL;
    node->item = item;

    if (size == 0) {
        head = tail = node;
    } else {
        tail->next = node;
    }

    size++;
}

void* dequeue(void) {
    if (size == 0) return NULL;

    ADT_Node *newHead = head->next;
    void* result = head->item;
    free(head);
    head = newHead;
    size--;

    return result;
}

/******************************************************************************/


/* Part of some networking file */
/******************************************************************************/

typedef ADT_Node Packet;
char* serialise_packet(char *s);


char* serialise_packet(char *s) {
    char *result = malloc(strlen(s) + 1);
    strcpy(result, s);

    return result;
}

/******************************************************************************/

int main(void) {
    char s1[] = "This is.";
    char *s2 = "A test.";
    char *s3 = "String!";

    char *packet1 = serialise_packet(s1);
    char *packet2 = serialise_packet(s2);
    char *packet3 = serialise_packet(s3);

    adt_init();
    enqueue(packet1);
    enqueue(packet3);
    char *data = dequeue();
    printf("%s\n", data);

    free(data);
    
    return 0;
}
