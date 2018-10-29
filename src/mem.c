/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018 Sean Baggaley.

    Obsidian is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Obsidian is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Obsidian.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdbool.h>
#include <string.h>

#define NUM_ALLOCINFOS 8192

typedef struct allocinfo_s {
    void *pointer;
    size_t size;
} allocinfo_t;

allocinfo_t **allocs = NULL;

bool disable_debug_output = false;
static bool mem_inited = false;

void *__real_malloc(size_t size);
void *__real_calloc(size_t num, size_t size);
void __real_free(void *ptr);

static void mem_init() {
    if (mem_inited) {
        return;
    }

    allocs = __real_calloc(NUM_ALLOCINFOS, sizeof(*allocs));
    for (int i = 0; i < NUM_ALLOCINFOS; i++) {
        allocs[i] = __real_malloc(sizeof(allocinfo_t));
        memset(allocs[i], 0, sizeof(allocinfo_t));
    }

    mem_inited = true;
}

static int mem_find_free() {
    for (int i = 0; i < NUM_ALLOCINFOS; i++) {
        if (allocs[i]->pointer == NULL) {
            return i;
        }
    }

    fprintf(stderr, "mem_find_free: no available IDs, bump NUM_ALLOCINFOS in %s\n", __FILE__);
    abort();
    return -1;
}

static int mem_find_from_ptr(void *ptr) {
    for (int i = 0; i < NUM_ALLOCINFOS; i++) {
        if (allocs[i]->pointer == ptr) {
            return i;
        }
    }

    fprintf(stderr, "mem_find_free: couldn't find allocinfo with ptr %p\n", ptr);
    abort();
    return -1;
}

void *__wrap_malloc(size_t size) {
    if (!mem_inited) mem_init();

    void *block = __real_malloc(size);

    if (block == NULL) {
        fprintf(stderr, "malloc(%zu) returned NULL!\n", size);
        abort();
        return NULL;
    }

    allocinfo_t *ai = allocs[mem_find_free()];
    ai->pointer = block;
    ai->size = size;

    if (disable_debug_output) return block;

#ifdef DEBUG
    printf("malloc of %zu bytes at %p\n", size, block);
#endif

    return block;
}

void *__wrap_calloc(size_t num, size_t size) {
    if (!mem_inited) mem_init();
    
    void *block = __real_calloc(num, size);

    if (block == NULL) {
        fprintf(stderr, "calloc(%zu, %zu) returned NULL!\n", num, size);
        abort();
        return NULL;
    }

    allocinfo_t *ai = allocs[mem_find_free()];
    ai->pointer = block;
    ai->size = size * num;

    if (disable_debug_output) return block;

#ifdef DEBUG
    printf("calloc of %zu * %zu = %zu bytes at %p\n", num, size, num * size, block);
#endif

    return block;
}

void __wrap_free(void *ptr) {
    if (!mem_inited) mem_init();
    
    __real_free(ptr);

    allocinfo_t *ai = allocs[mem_find_from_ptr(ptr)];
    ai->pointer = NULL;
    ai->size = 0;

    if (disable_debug_output) return;

#ifdef DEBUG
    printf("freed %p\n", ptr);
#endif
}

int mem_get_used() {
    int total = 0;

    for (int i = 0; i < NUM_ALLOCINFOS; i++) {
        allocinfo_t *ai = allocs[i];
        if (ai->pointer == NULL) continue;

        total += ai->size;
    }

    return total;
}