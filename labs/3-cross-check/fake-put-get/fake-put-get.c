/*
 * Simple tracing memory implementation.  Used for cross-checking.
 * 
 * you will implement a simple fake memory that maps addresses to values.  
 * e.g., an array, linked list, hash table of (<addr>, <val>) tuples such 
 * that after you insert some (<addr>, <val>), you can lookup <val> using 
 * <addr> as a key.
 *
 * simplistic assumptions:
 *  - all loads/stores are 32 bits.
 *  - read returns value of last write, or random() if none.
 *  - load and store prints the addr,val
 * 
 * HINT: I woul suggest a fixed size array, that you do a simple linear scan 
 * to insert, and lookup.
 *
 * mildly formal-ish rules for fake memory:
 *  1. no duplicate entries.  
 *       if: 
 *           (<addr>, <val0>) \in memory
 *           (<addr>, <val1>) \in memory
 *      then <val0> == <val1>
 *  2. addresses are persistent: if you ever write <addr>,<val> to memory, 
 *     then <addr> stays there forever.
 *  3. values are persistent until the next write: if you ever write 
 *     (<addr>,<val> ) to memory, then until the next write, 
 *          if (<addr>, <v>) \in memory, then <v> == <val>.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "demand.h"
#include "rpi.h"
#include "pi-test.h"

/****************************************************************************
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 */

typedef struct {
    const volatile void *addr;
    unsigned val;
} mem_t;

typedef struct {
    int op;
    mem_t v;
} log_ent_t;

static mem_t mk_mem(const volatile void *addr, unsigned val) {
    return (mem_t) { .addr = addr, .val = val };
}

// don't change routine so we can compare to everyone.
static void print_write(mem_t *m) {
    printf("\tTRACE:PUT32(%p)=0x%x\n", m->addr, m->val);
}
// don't change routine so we can compare to everyone.
static void print_read(mem_t *m) {
    printf("\tTRACE:GET32(%p)=0x%x\n", m->addr, m->val);
}

#define MEM_SIZE 10000
mem_t* ssd[MEM_SIZE];
unsigned pos = 0;

int getSSDPos(const volatile void *addr) {
    int i;
    for (i = 0; i < pos; ++i) {
        if (ssd[i]->addr == addr) {
            return i;
        }
    }
    return -1;
}

mem_t* putIntoSSD(const volatile void *addr, unsigned val) {
    int ssdPos = getSSDPos(addr);
    if (ssdPos != -1) {
        // Data already exist, we just modify data.
        mem_t* data = ssd[ssdPos];
        data->val = val;
        return data;
    } else {
        // Data does not exist at addr, we create new data.
        if (pos > MEM_SIZE) {
            // TODO panic here
            return NULL;
        }

        mem_t* new_data = malloc(sizeof(mem_t));
        new_data->addr = addr;
        new_data->val = val;
        ssd[pos] = new_data;
        pos++;
        return new_data;
    }
}


unsigned get32(const volatile void *addr) {
    int ssdPos = getSSDPos(addr);
    if (ssdPos != -1) {
        print_read(ssd[ssdPos]);
        return ssd[ssdPos]->val;
    }
    unsigned garbage = rand();
    mem_t* data = putIntoSSD(addr, garbage);
    print_read(data);
    return garbage;
}

void put32(volatile void *addr, unsigned val) {
    mem_t* data = putIntoSSD(addr, val);
    print_write(data);
}

