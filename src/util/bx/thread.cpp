//
// Created by Dylan Lukes on 8/16/23.
//

#include "bx/thread.h"
#include "neopad/internal/shims/bx/thread.h"

struct bx_thread_s {
    bx::Thread impl;
};

// Just to be safe...
static_assert(sizeof(bx_thread_s) == sizeof(bx::Thread), "Size mismatch");

bx_thread_t bx_thread_create() {
    return new struct bx_thread_s;
}

void bx_thread_destroy(bx_thread_t thread) {
    delete thread;
}

bool bx_thread_init(bx_thread_t thread, bx_thread_fn fn, void *user_data, uint32_t stack_size, const char *name) {
    union {
        bx_thread_fn c;
        bx::ThreadFn cpp;
    } cast_fn{.c = fn};
    return thread->impl.init(cast_fn.cpp, user_data, stack_size, name);
}


void bx_thread_shutdown(bx_thread_t thread) {
    thread->impl.shutdown();
}

bool bx_thread_is_running(bx_thread_t thread) {
    return thread->impl.isRunning();
}

int32_t bx_thread_get_exit_code(bx_thread_t thread) {
    return thread->impl.getExitCode();
}

void bx_thread_set_name(bx_thread_t thread, const char *name) {
    thread->impl.setThreadName(name);
}

void bx_thread_push(bx_thread_t thread, void *ptr) {
    thread->impl.push(ptr);
}

void *bx_thread_pop(bx_thread_t thread) {
    return thread->impl.pop();
}