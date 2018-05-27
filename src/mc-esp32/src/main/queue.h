#ifndef QUEUE_H
#define QUEUE_H

extern void mc_queue_init(void);
extern void mc_queue_enqueue(void *item);
extern void mc_queue_enqueue_at_head(void *item);
extern void *mc_queue_dequeue(void);
extern void *mc_queue_peek(void);

#endif /* QUEUE_H */
