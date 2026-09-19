.data
  msg: .asciz "Hello World!\n"
  data_pointer: .long 0x00000001

/*

Aarch64 Instruction size: 32 bits -> ex. ADD: 31 bit [opcode |24| sf |23| sh |22| imm12 |10| rN |5| rD] 0 bit
                                      Remember!! Memory stored as Little Eudian (Reversed)
                                          Here we see we have imm12 for our intermediate value. These blueprints can be found in the ARM manual.

# Register types:
 GPR - General purpose registers
 FPR - Floating point registers
 SPR - SPecial Purpose registers

  Higher bits - Lefthand side
  Lower bits - Righthand side

 Extended registers: xD - 64 bits
 Non-Extended registers: wD - 32 bits 
  - Utilize only the lower bits of the GPR.
 
# Signed & Unsigned values:
  Signed - negative & positive
  Unsigned - positive and 0.

How are negative numbers represented?
    - The first bit contained in a register is 1
    - Value is treated as signed 

 Example: Non-Extended registers
 Range (Signed):   0x80000000 <-> 0xFFFFFFFF < 0x0 < 0x00000001 <-> 0x7FFFFFFF  (Remember 8 in Binary = 1000, F = 1111)
 Range (Unsigned): 0x0 < 0x00000001 <-> 0xFFFFFFFF

 Immidiate values:
 Numerical value that is written from "scratch"
 Comes with flavours:
 SIMM(12,32,64) = Signed x-bit value
 UIMM(12,32,84) = Unsigned x-bit value

 To allow instructions to be 32 bit, we can extend immediate values to fit the size.
  Saying: In operations, the processor converts smaller sizes to their full bit range by adding trailing 0's (positive) or F's negative.

                                    Showing with extended here        Showing without extension here    -> Was like this in the course, 0x7FFF should be written also as an extension, 0x00007FFF
Signed Immediate Value (32-bit): 0xFFFF8000 <-> 0xFFFFFFFF < 0x0 < 0x0001 <-> 0x7FFF (Remember zeros on start can be removed)
Signed Immediate Value (64-bit): 0xFFFFFFFF80000000 <-> 0xFFFFFFFFFFFFFFFFF < 0x0 < 0x0000001 <-> x7FFFFFFFFFF

Unsigned Immediate Value (32-bit): 0x0000 <-> 0xFFFF
Unsigned Immediate Value (64-bit): 0x00000000 <-> 0xFFFFFFFF

12-bit Singed values come with some quirks:
  - Negatives:
    - Still will ber a WORD when writing as Non-Extended usage.
    - For extended usage 12-bit values must be written as 64 bit.

Signed Immediate Value (12-bit) - For Non-Extended use:
  - 0x800 <-> 0xFFF < 0x0 < 0x001 <-> 0x7FF
Signed Immediate Value (12-bit) - For Extended Use:
  - 0x800 <-> 0xFFF < 0x0 < 0x001 <-> 0x7FF
Unsigned Immediate Value (12-bit): 0x000 <-> 0xFFF


lsl - leftward bit shift

movk - move then keep ex. movk x1, #0xFFFF, lsl #32 -> 0x0000FFFF00000000
movz - move then zero (move and remove old)
  - mov x1, 0xFFFF00000000000 can be written as movz x1, 0xFFFF, lsl #48
  - Most likely converted to a standard move upon compilation

half word = 2 b = 16 bits  
word = 4 b = 32 bits
double word = 8b = 64 bits


str - Store register (word/double word)
strh - Store register half word
strb - Store byte

*/

.text
.global _main

increment:
  add x3, x3, #1
  ret

ascii_convert:
  add x3, x3, #48
  ret

_main:
    nop
    adrp x5, data_pointer@PAGE
    add x5, x5, data_pointer@PAGEOFF
    ldr w0, [x5, #4]
    add w0, w0, #2
    str w0, [x5, #4]

    //mov x16, #0x4 //write() syscall
    //mov x0, #1 //stdout
    //adrp x1, msg@page
    //add x1, x1, msg@pageoff
    //mov x2, #16 // size
    //svc #0x80

    mov x3, #-1 // Start value
    mov x4, #8 // Loop max range

    loop:
    bl increment // Call to incrmeent on x3
    bl ascii_convert
    str x3, [sp, #-16]! // Store register, move stack pointer amount (given by ! and -1)

    mov x16, #0x4
    mov x0, #1
    mov x1, sp // Move sp pointer to x1
    mov x2, #1 // Just one byte, symbol char.
    svc #0x80
    
    sub x3, x3, #48
    cmp x3, x4 
    bls loop
    
    add sp, sp, #16 // Restore stack pointer

    mov x16, #0x1 // syscall exit
    mov x0, #0xDEAD
    svc #0x80

