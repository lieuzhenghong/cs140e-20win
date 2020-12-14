# Additional notes for context

## Motivation

The lab notes are kind of sparse and don't give much context/
don't give the big picture of what we are doing.
These notes aim to fill that gap.

## Lab 0

In Lab 0 we wired the Pi up and made sure that everything worked.
This lab was actually really hard because a lot of the devices
are defective.

- Test that all devices (LED, raspi, tty) works.
- Install pi-install to /usr/local/bin for easier access
- Run sample code on raspi

## Lab 1

Lab 1 made us read (part of) the Broadcom documentation 
to write C code that set the state of the GPIO
(by writing the right bits to the correct memory address).
We wrote a file `gpio.c` that we compiled and sent over to the Pi
using two provided programs, `pi-install` and `bootloader.bin`.

- Using broadcom documentation to toggle GPIO on raspi
  - Set GPIO state to be input, output or read/write
- Write c code to perform the toggling.

## Lab 2

Did 14th December 2020 at 7pm with Weineng
Finished at 1030pm

Lab 2 is about building a bootloader file that allows for
bidirectional communication over the USB-TTY connection
using a provided acknowledgement protocol.
This bootloader allows us to load a program into the Raspberry Pi's RAM
over the connection and run that program.
(This replaces the functionality given by `pi-install` and `bootloader.bin`).

Previously we were only able to run programs on the Pi by using
the "magic" program `pi-install`.

In the process of writing the bootloader file we also had to write
some helper functions
as well as understand how the rest of the codebase worked.

We modified the following files:

- find-ttyusb.c (to find the TTY-USB device) `./labs/2-bootloader/unix-side/libunix/find-ttyusb.c`
- read-file.c (helper function to read a file and pad it)
- simple-boot.c (unix side listener)
- bootloader.c (raspi side bootloader)

`bootloader.c` is compiled to `kernel.img` and runs on Raspi startup.

After finishing the assignment we were able to load in binary files
that printed out "Hello World"
and blinked an LED on GPIO pin 20 respectively.

There were many unexplained things and here is my attempt to elucidate:

### How do programs run on the Raspi?

In lab 0 we copied several files like `bootcode.bin`, `start.elf`,
`config.txt` and `kernel.img` into the SD card of the Raspi.
Upon booting the Raspi the LED connected to GPIO port 20 started to blink.
How does this work? What are these files, anyway?

[The 2018 offering of CS140e](https://cs140e.sergio.bz/assignments/0-blinky/):

> **What are bootcode.bin, config.txt, and start.elf?**
>
> These specially-named files are recognized by the Raspberry Pi’s GPU on
boot-up and used to configure and boostrap the system. bootcode.bin is the
GPU’s first-stage bootloader. Its primary job is to load start.elf, the GPU’s
second-stage bootloader. start.elf initializes the ARM CPU, configuring it as
indicated in config.txt, loads kernel8.img into memory, and instructs the CPU
to start executing the newly loaded code from kernel8.img.

What this means for us:
our user-written programs will have to be saved as `kernel.img`
and saved to the SD card. 
This is how we're going to get our bootloader onto the Raspi.
We write a bootloader in C called `bootloader.c`,
compile it on our computer using the `Makefile`,
rename it `kernel.img`,
save it on the SD card,
and then put it in the Raspi.

### How do programs run on the Raspi? Part 2

I believe the `kernel.img` file is a binary file
that is being run line by line.

What calls the `notmain` function? 
To do this we have to look at `start.s`, an ARM64 assembler file:

```arm
.globl _start
_start:
    b skip
.space 0x200000-0x8004,0
skip:
    mov sp,#0x08000000
    bl notmain
hang: b reboot
```

This is a little arcane, so let's do some Googling to explain it:

#### What's in start.s?

`sp` means stack pointer. 
So we set the stack pointer to `0x080000`

`b` means unconditional branch,
so instruction `b` branches (modifies the program counter)
to the instruction at the label `skip` or `reboot`

`bl` means branch with link: 

> The BL instruction `bl label` causes a branch to `label`, 
and copies the address of the next instruction into LR (R14, the link register).

#### How does bootloader.c run the received program?

If you look at line 145 of `bootloader.c` you'll see `BRANCHTO(ARMBASE)`.
What does this function do? It turns out that this is an assembler function
in `start.s`:

```
.globl BRANCHTO
BRANCHTO:
    bx r0
```

`bx` stands for "branch and exchange":

From [thinkingeek](https://thinkingeek.com/2013/01/09/arm-assembler-raspberry-pi-chapter-1/):

> This instruction bx means branch and exchange. We do not really care at
this point about the exchange part. Branching means that we will change the
flow of the instruction execution. An ARM processor runs instructions
sequentially, one after the other, thus after the mov above, this bx will be
run (this sequential execution is not specific to ARM, but what happens in
almost all architectures). A branch instruction is used to change this
implicit sequential execution. In this case we branch to whatever lr register
says. We do not care now what lr contains. It is enough to understand that
this instruction just leaves the main function, thus effectively ending our
program.

Putting it all together:

### So how does everything come together?

This is how we receive and run any arbitrary program from our Unix computer
to the Raspi.

1. `start.s` is compiled into `start.o`
2. `bootloader.c` is compiled into `bootloader.o`
3. The contents of bootloader.o are appended into start.o
4. The entire thing (`start.o` + `bootloader.o`) is then renamed as `kernel.img`
   At this point `kernel.img` now has the `start.o` and the `bootloader.o` code containing the `notmain` function.
5. We send `kernel.img` into the SD card (using `make copy`)
6. We boot up the Raspi, `kernel.img` is run (with the GPU bootloader thingy)
7. We start running the assembler code from `_start` and so on
8. We hit `bl notmain` which branches and runs the bootloader protocol described in Lab 2 `bootloader.c`
9. `notmain` runs, writing received data from unix-side starting at memaddr `ARMBASE`.
10. Once code has been received, we call the function `BRANCHTO(ARMBASE)`
    which branches to memaddr `ARMBASE` and runs the received program.

Steps 1 to 4 might be totally wrong but steps 5 to 10 should be correct.