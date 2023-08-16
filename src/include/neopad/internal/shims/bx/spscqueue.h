//
// Created by Dylan Lukes on 8/16/23.
//

#ifndef NEOPAD_SHIMS_BX_SPSCQUEUE_H
#define NEOPAD_SHIMS_BX_SPSCQUEUE_H

typedef struct bx_spsc_queue_s *bx_spsc_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

bx_spsc_queue_t bx_spsc_queue_create();
void bx_spsc_queue_destroy(bx_spsc_queue_t queue);

void bx_spsc_queue_push(bx_spsc_queue_t queue, void *data);
void *bx_spsc_queue_pop(bx_spsc_queue_t queue);
void *bx_spsc_queue_peek(bx_spsc_queue_t queue);

#ifdef __cplusplus
}
#endif

#endif //NEOPAD_SHIMS_BX_SPSCQUEUE_H
