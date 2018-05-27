#ifndef QUEUE_H
#define QUEUE_H

extern void queue_init(void);
extern void queue_enqueue(void *item);
extern void *queue_dequeue(void);
extern void *queue_peek(void);

#endif /* QUEUE_H */
