#ifndef NEOPAD_SHIMS_BX_THREAD_H
#define NEOPAD_SHIMS_BX_THREAD_H

/**
 * Exposes a C99 API to bx::Thread.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer type for a thread.
typedef struct bx_thread_s *bx_thread_t;
typedef int32_t (*bx_thread_fn)(bx_thread_t, void *);

bx_thread_t bx_thread_create();
void bx_thread_destroy(bx_thread_t thread);

bool bx_thread_init(bx_thread_t thread, bx_thread_fn fn, void *user_data, uint32_t stack_size, const char *name);

void bx_thread_shutdown(bx_thread_t thread);

bool bx_thread_is_running(bx_thread_t thread);

int32_t bx_thread_get_exit_code(bx_thread_t thread);

void bx_thread_set_name(bx_thread_t thread, const char *name);

void bx_thread_push(bx_thread_t thread, void *ptr);

void *bx_thread_pop(bx_thread_t thread);

#ifdef __cplusplus
}
#endif

#endif