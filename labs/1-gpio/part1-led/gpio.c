/*
 * write code to allow blinking using arbitrary pins.    
 * Implement:
 *	- gpio_set_output(pin) --- set GPIO <pin> as an output (vs input) pin.
 *	- gpio_set_on(pin) --- set the GPIO <pin> on.
 * 	- gpio_set_off(pin) --- set the GPIO <pin> off.
 * Use the minimal number of loads and stores to GPIO memory.  
 *
 * start.s defines a of helper functions (feel free to look at the assembly!  it's
 *  not tricky):
 *      uint32_t get32(volatile uint32_t *addr) 
 *              --- return the 32-bit value held at <addr>.
 *
 *      void put32(volatile uint32_t *addr, uint32_t v) 
 *              -- write the 32-bit quantity <v> to <addr>
 * 
 * Check-off:
 *  1. get a single LED to blink.
 *  2. attach an LED to pin 19 and another to pin 20 and blink in opposite order (i.e.,
 *     one should be on, while the other is off).   Note, if they behave weirdly, look
 *     carefully at the wording for GPIO set.
 */
#include "rpi.h"

/*
 * These routines are given by us and are in start.s
 */
// writes the 32-bit value <v> to address <addr>:   *(unsigned *)addr = v;
void put32(volatile void *addr, unsigned v);
// returns the 32-bit value at <addr>:  return *(unsigned *)addr
unsigned get32(const volatile void *addr);
// does nothing.
void nop(void);

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
volatile unsigned *gpio_fsel0 = (void*)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (void*)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (void*)(GPIO_BASE + 0x28);

// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output

// set <pin> to be an output pin.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!
void gpio_set_output(unsigned pin) {
    // pin is a value between 0 and 53
    // First you need to use the correct select register
    // There are 54 pins and 6 registers
    unsigned register_num = pin / 10; // this will give a value from 0 to 5
    volatile unsigned *addr = gpio_fsel0 + register_num; // the address of the register to write to
    // we now need to go to the address and set it
    unsigned fsel_num = pin % 10; // this gives a number from 0 to 9
    unsigned tmp = get32(addr);
    tmp &= ~(0b111 << (fsel_num * 3));
    tmp |= (1 << (fsel_num * 3));
    put32(addr, tmp);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    // use gpio_set0
    volatile unsigned *addr = (gpio_set0 + pin/32);
    unsigned setn_num = pin % 32;
    put32(addr, 1 << setn_num);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    // use gpio_clr0
    volatile unsigned *addr = (gpio_clr0 + pin/32);
    unsigned setn_num = pin % 32;
    put32(addr, 1 << setn_num);
}

// Part 2: implement gpio_set_input and gpio_read

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    // implement.
}
// return the value of <pin>
int gpio_read(unsigned pin) {
    unsigned v = 0;

    // implement.

    return v;
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
       gpio_set_on(pin);
    else
       gpio_set_off(pin);
}

void gpio_set_function(unsigned pin, gpio_func_t function) {
  if (pin >= 32) {
    return;
  }
  int fsel = pin / 10;
   volatile unsigned *gpio_fsel = gpio_fsel0 + fsel;

  unsigned data = get32(gpio_fsel);
  unsigned bit_pos = 3 * (pin % 10);
  data &= ~(0b111 << bit_pos); // clear the bits
  data |= function << bit_pos;
  put32(gpio_fsel, data);
}
