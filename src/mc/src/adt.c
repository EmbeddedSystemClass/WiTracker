#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Part of some ADT file */
/******************************************************************************/

typedef struct ADT_Node {
    struct ADT_Node *next;
    void *data;
} ADT_Node;

ADT_Node *head;
ADT_Node *tail;
uint16_t size;

void adt_init(void) {
    head = NULL;
    tail = NULL;
    size = 0;
}

void enqueue(ADT_Node *node) {
    if (node == NULL) return;

    if (size == 0) {
        head = tail = node;
    }
}

/******************************************************************************/


/* Part of some networking file */
/******************************************************************************/

typedef ADT_Node Packet;

void create_packet(Packet *out, char *s) {
    //char *temp = malloc(strlen(s) + 1);
    char temp[strlen(s) + 1];
    strcpy(temp, s);
    //printf("1: %s\n", temp);
    
    Packet packet = {
        .next = NULL,
        .data = temp
    };
    //printf("2: %s\n", (char*) packet.data);

    *out = packet;
}

/******************************************************************************/

int main(void) {
    char s1[] = "This is.";
    char *s2 = "A test.";
    char *s3 = "String!";

    Packet packet;
    create_packet(&packet, s1);

    s1[3] = 'b';

    printf("%s\n", (char*) packet.data);


    return 0;
}
