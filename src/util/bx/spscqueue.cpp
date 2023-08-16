//
// Created by Dylan Lukes on 8/16/23.
//

#include "bx/allocator.h"
#include "bx/spscqueue.h"
#include "neopad/internal/shims/bx/spscqueue.h"

static bx::DefaultAllocator default_allocator;

struct bx_spsc_queue_s {
    bx::SpScUnboundedQueue impl;
    bx_spsc_queue_s() : impl(&default_allocator) {}
};

// Just to be safe...
static_assert(sizeof(bx_spsc_queue_s) == sizeof(bx::SpScUnboundedQueue), "Size mismatch");

bx_spsc_queue_s *bx_spsc_queue_create() {
    return new struct bx_spsc_queue_s;
}

void bx_spsc_queue_destroy(bx_spsc_queue_s *queue) {
    delete queue;
}

void bx_spsc_queue_push(bx_spsc_queue_s *queue, void *data) {
    queue->impl.push(data);
}

void *bx_spsc_queue_pop(bx_spsc_queue_s *queue) {
    return queue->impl.pop();
}

void *bx_spsc_queue_peek(bx_spsc_queue_s *queue) {
    return queue->impl.peek();
}

