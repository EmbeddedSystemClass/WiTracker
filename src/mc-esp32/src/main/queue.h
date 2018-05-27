#ifndef QUEUE_H
#define QUEUE_H

// initialises the queue CDT
extern void mc_queue_init(void);
// Adds and item to the tail of the queue
extern void mc_queue_enqueue(void *item);
// Adds an item to the head of the queue
extern void mc_queue_enqueue_at_head(void *item);
// Dequeues an item from the head of the queue
extern void *mc_queue_dequeue(void);
// Returns the next dequeueable item without removing it from the queue
extern void *mc_queue_peek(void);

#endif /* QUEUE_H */
