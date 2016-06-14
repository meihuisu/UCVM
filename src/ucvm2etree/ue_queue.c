#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ue_queue.h"



int q_init(int maxlen, int size, ue_queue_t *q)
{
  if ((q == NULL) || (maxlen <= 0) || (size <= 0)) {
    return(1);
  }

  q->h = 0;
  q->t = 0;
  q->len = 0;
  q->maxlen = maxlen;
  q->size = size;
  q->buf = malloc(q->maxlen * q->size);
  if (q->buf == NULL) {
    return(1);
  }
  memset(q->buf, 0, q->maxlen * q->size);
  return(0);
}


int q_free(ue_queue_t *q)
{
  if (q == NULL) {
    return(1);
  }

  q->h = 0;
  q->t = 0;
  q->len = 0;
  q->maxlen = 0;
  q->size = 0;
  free(q->buf);
  q->buf = NULL;
  return(0);
}


int q_reset(ue_queue_t *q)
{
  if (q == NULL) {
    return(1);
  }

  q->h = 0;
  q->t = 0;
  q->len = 0;
  return(0);
}


int q_has_space(ue_queue_t *q)
{
  if (q == NULL) {
    return(0);
  }
  if (q->len < q->maxlen) {
    return(1);
  } else {
    return(0);
  }
}


int q_get_len(ue_queue_t *q)
{
  if (q == NULL) {
    return(0);
  }
  return(q->len);
}


int q_get_maxlen(ue_queue_t *q)
{
  if (q == NULL) {
    return(0);
  }
  return(q->maxlen);
}


int q_get_size(ue_queue_t *q)
{
  if (q == NULL) {
    return(0);
  }
  return(q->size);
}


int q_get_head(ue_queue_t *q, void **elem)
{
  if ((q == NULL) || (q->len == 0)) {
    return(1);
  }

  *elem = q->buf + (q->h * q->size);
  return(0);
}


int q_pop_head(ue_queue_t *q, void **elem)
{
  if ((q == NULL) || (q->len == 0)) {
    return(1);
  }

  *elem = q->buf + (q->h * q->size);
  q->h = (q->h + 1) % q->maxlen;
  (q->len)--;
  return(0);
}


int q_alloc_tail(ue_queue_t *q, void **elem)
{
  if ((q == NULL) || (q->len == q->maxlen)) {
    return(1);
  }

  *elem = q->buf + (q->t * q->size);
  q->t = (q->t + 1) % q->maxlen;
  (q->len)++;
  return(0);
}


int q_alloc_contig_tail(ue_queue_t *q, int n, void **elem)
{
  if ((q == NULL) || 
      (q->len + n > q->maxlen) || 
      (q->t + n > q->maxlen) ||
      (n <= 0)) {
    return(1);
  }

  *elem = q->buf + (q->t * q->size);
  q->t = (q->t + n) % q->maxlen;
  q->len = q->len + n;
  return(0);
}


int q_free_tail(ue_queue_t *q, int n)
{
  if ((q == NULL) || (q->len < n)) {
    return(1);
  }

  q->t = q->t - n;
  if (q->t < 0) {
    q->t = q->t + q->maxlen;
  }
  q->len = q->len - n;
  return(0);
}

