#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdbool.h>

bool disable_debug_output = false;

void *__real_malloc(size_t size);
void *__real_calloc(size_t num, size_t size);
void __real_free(void *ptr);

char *get_last_function() {
    void *bt[5];
    backtrace(bt, 5);
    char **syms = backtrace_symbols(bt, 5);

    return syms[2];
}   

void *__wrap_malloc(size_t size) {
    void *block = __real_malloc(size);

    if (block == NULL) {
        fprintf(stderr, "malloc(%zu) returned NULL!\n", size);
        abort();
        return NULL;
    }

    if (disable_debug_output) return block;

#ifdef DEBUG
    printf("%s: malloc of %zu bytes at %p\n", get_last_function(), size, block);
#endif

    return block;
}

void *__wrap_calloc(size_t num, size_t size) {
    void *block = __real_calloc(num, size);

    if (block == NULL) {
        fprintf(stderr, "calloc(%zu, %zu) returned NULL!\n", num, size);
        abort();
        return NULL;
    }

    if (disable_debug_output) return block;

#ifdef DEBUG
    printf("%s: calloc of %zu * %zu = %zu bytes at %p\n", get_last_function(), num, size, num * size, block);
#endif

    return block;
}

void __wrap_free(void *ptr) {
    __real_free(ptr);

    if (disable_debug_output) return;

#ifdef DEBUG
    printf("%s: freed %p\n", get_last_function(), ptr);
#endif
}