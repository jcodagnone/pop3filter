#ifndef FAAFC2C44AF1B13A59EE25D44A856DB1
#define FAAFC2C44AF1B13A59EE25D44A856DB1

typedef struct queueHead * queue_t;

queue_t queue_new(void);
void queue_destroy(queue_t q);

int queue_is_valid(queue_t q);
int queue_is_empty(queue_t q);

int    queue_enqueue(queue_t q, const void *data, size_t len);
void * queue_block_dequeue(queue_t q, size_t *len, size_t block);

#endif
