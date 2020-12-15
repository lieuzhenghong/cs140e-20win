// engler, cs140e: trivial example of how to log GET32.  
//
// part 1: add get32, put32, PUT32.  
//    - you'll have to modify the makefile --- search for GET32 and add there.
//    - simply have put32 call PUT32, get32 call GET32 so its easier to compare output.
//
// part 2: add a simple log for capturing
//
//  without capture it's unsafe to call during UART operations since infinitely
//  recursive and, also, will mess with the UART state.  
//
//  record the calls in a log (just declare a statically sized array until we get malloc)
//  for later emit by trace_dump()
//
// part 3: keep adding more functions!  
//      often it's useful to trace at a higher level, for example, doing 
//      uart_get() vs all the put's/get's it does, since that is easier for
//      a person to descipher.  or, you can do both:
//          -  put a UART_PUTC_START in the log.
//          - record the put/gets as normal.
//          - put a UART_PUTC_STOP in the log.
//  
// XXX: may have to use wdiff or similar to match the outputs up exactly.  or strip out
// spaces.
//

#include "rpi.h"
#include "trace.h"

// gross that these are not auto-consistent with GET32 in rpi.h
unsigned __wrap_GET32(unsigned addr);
unsigned __real_GET32(unsigned addr);

unsigned __wrap_get32(const volatile void *addr);
unsigned __real_get32(const volatile void *addr);

void __wrap_PUT32(unsigned addr, unsigned v);
void __real_PUT32(unsigned addr, unsigned v);

void __wrap_put32(volatile void *addr, unsigned v);
void __real_put32(volatile void *addr, unsigned v);

static int tracing_p = 0;
static int logging_p = 0;
static int in_trace = 0;

// Logs' buffer for storing logs before printing them as a dump
// when tracing is completed.
#define LOG_MAXLEN 256
#define LOGS_MAXLEN 1000
static char LOGS[LOGS_MAXLEN][LOG_MAXLEN];
static int POS = 0;

void trace_start(int capture_p) {
    demand(!tracing_p, "trace already running!");
    demand(!in_trace, "invalid in_trace");
    if (capture_p) {
        logging_p = 1;
    }
    tracing_p = 1;
    return;
}

// Add to the end of the logs. If buffer is full, no-op is done.
void add_to_log(char* s) {
    if (POS == LOGS_MAXLEN) {
        // TODO: consider panic here.
        return;
    }
    memcpy(LOGS[POS], s, strlen(s));
    POS++;
    return;
}

// the linker will change all calls to GET32 to call __wrap_GET32
unsigned __wrap_GET32(unsigned addr) { 
    // the linker will change the name of GET32 to __real_GET32,
    // which we can then call ourselves.
    unsigned v = __real_GET32(addr); 

    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    // Note that logging_p and in_trace can never both be 1
    if (logging_p && tracing_p) {
        // TODO: Figure out a way to inject address and value to log.
        add_to_log("TRACE:GET32\n");
    }
    else if (!in_trace && tracing_p) {
        in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk("\tTRACE:GET32(0x%x)=0x%x\n", addr, v);
        in_trace = 0;
    }
    return v;
}

// the linker will change all calls to get32 to call __wrap_get32
unsigned __wrap_get32(const volatile void *addr) { 
    // the linker will change the name of GET32 to __real_get32,
    // which we can then call ourselves.
    unsigned v = __real_get32(addr); 
    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    // Note that logging_p and in_trace can never both be 1
    if (logging_p && tracing_p) {
        // TODO: Figure out a way to inject address and value to log.
        add_to_log("TRACE:get32\n");
    }
    else if (!in_trace && tracing_p) {
        in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk("\tTRACE:get32(0x%x)=0x%x\n", addr, v);
        in_trace = 0;
    }
    return v;
}

void __wrap_PUT32(unsigned addr, unsigned v) { 
    __real_PUT32(addr, v); 
    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    if (!in_trace && tracing_p) {
        // TODO: Figure out a way to inject address and value to log.
        add_to_log("TRACE:PUT32\n");
    }
    // Note that logging_p and in_trace can never both be 1
    else if (logging_p && tracing_p) {
        in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk("\tTRACE:PUT32(0x%x)=0x%x\n", addr, v);
        in_trace = 0;
    }
    return;
}

// the linker will change all calls to put32 to call __wrap_put32
void __wrap_put32(volatile void *addr, unsigned v) { 
    // the linker will change the name of put32 to __real_put32,
    // which we can then call ourselves.
    __real_put32(addr, v); 

    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    if (!in_trace && tracing_p) {
        // TODO: Figure out a way to inject address and value to log.
        add_to_log("TRACE:put32\n");
    }
    // Note that logging_p and in_trace can never both be 1
    else if (logging_p && tracing_p) {
                in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk("\tTRACE:put32(0x%x)=0x%x\n", addr, v);
        in_trace = 0;
    }
    return;
}


void trace_stop(void) {
    demand(tracing_p, "trace already stopped!\n");
    tracing_p = 0;
    logging_p = 0;
    trace_dump(1);
}


void trace_dump(int reset_p) { 
    printk("STARTING LOG DUMP\n");
    int i, j;
    for (i = 0; i < POS; ++i) {
        printk(LOGS[i]);
    }
    if (reset_p) {
        for (i = 0; i < POS; i++) {
            for (j = 0; j < LOG_MAXLEN; ++j) {
                LOGS[i][j] = 0;
            }
        }
        POS = 0;
    }
    printk("DUMP COMPLETED\n");
}
