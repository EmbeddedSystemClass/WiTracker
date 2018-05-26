#ifndef ADT_H
#define ADT_H

typedef struct ADT_Node
{
    struct ADT_Node *next;
    void *item;
} ADT_Node;

#endif /* ADT_H */
