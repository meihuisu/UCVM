#ifndef UE_QUEUE_H
#define UE_QUEUE_H

/* Fixed length queue module that supports efficient
   appending of new elements, and popping of head */

typedef struct ue_queue_t {
  int h;
  int t;
  int len;
  int maxlen;
  int size;
  void *buf;
} ue_queue_t;

/* Initializer and finalizer */
int q_init(int maxlen, int size, ue_queue_t *q);
int q_free(ue_queue_t *q);

/* Reset to empty queue */
int q_reset(ue_queue_t *q);

/* Returns boolean indicating if space is available */
int q_has_space(ue_queue_t *q);

/* Get queue attributes */
int q_get_len(ue_queue_t *q);
int q_get_maxlen(ue_queue_t *q);
int q_get_size(ue_queue_t *q);

/* Get head element with no removal from queue */
int q_get_head(ue_queue_t *q, void **elem);

/* Get head element with removal from queue */
int q_pop_head(ue_queue_t *q, void **elem);

/* Allocate an element at the tail pos and return pointer */
int q_alloc_tail(ue_queue_t *q, void **elem);

/* Allocate n contiguous elements at the tail pos and
   return pointer. Will fail if current tail pos + n > maxlen */
int q_alloc_contig_tail(ue_queue_t *q, int n, void **elem);

/* Free n elements from the tail pos */
int q_free_tail(ue_queue_t *q, int n);


#endif

