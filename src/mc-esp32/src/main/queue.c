#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "queue.h"
#include "adt.h"

static ADT_Node *head;
static ADT_Node *tail;
static uint16_t size;

void mc_queue_init(void)
{
    tail = head = NULL;
    size = 0;
}

void mc_queue_enqueue(void *item)
{
    if (item == NULL)
        return;

    ADT_Node *node = malloc(sizeof(ADT_Node));
    node->next = NULL;
    node->item = item;

    if (size == 0)
    {
        head = tail = node;
    }
    else
    {
        tail->next = node;
        tail = node;
    }

    size++;
}

void mc_queue_enqueue_at_head(void *item)
{
    if (item == NULL)
        return;

    if (size == 0)
        return mc_queue_enqueue(item);

    ADT_Node *node = malloc(sizeof(ADT_Node));
    node->next = head;
    node->item = item;

    head = node;

    size++;
}

void *mc_queue_dequeue(void)
{
    if (size == 0)
        return NULL;

    ADT_Node *newHead = head->next;
    void *result = head->item;
    printf("Dequeuing: %s\n", (char *)result);
    free(head);
    head = newHead;
    size--;

    printf("Dequeued! Size is now = %d\n", size);

    return result;
}

void *mc_queue_peek(void)
{
    if (size == 0)
        return NULL;

    return head->item;
}

uint16_t mc_queue_size(void)
{
    return size;
}

uint8_t mc_queue_is_empty(void)
{
    return size == 0;
}

uint16_t mc_queue_print(void)
{
    if (size == 0)
        return 0;

    ADT_Node *currentNode = head;
    uint16_t count = 0;

    while (currentNode != NULL)
    {
        printf("Node %d: %s\n", count + 1, (char *)currentNode->item);
        currentNode = currentNode->next;
        count++;
    }

    return count;
}
