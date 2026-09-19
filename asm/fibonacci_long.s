.data
msg: .asciz ""
separator: .space 16
hex_msg: .ascii "Hex value: "
endline: .ascii "\n"
separator2: .space 20
decimal_value: .asciz ""
.separato3: .space 64 // Estimate so nothing potentially takes this memory

.text
.global _main

/**
 A9 F4 AD 42 22 F1 27 02
 To decimals:
 b = 16
 2 * 16^0   +   0*16^1    +     7*16^2    +   2*16^3    +     1*16^4    + 15*16^5     +     2*16^6    + 2*16^7    + 2*16^8    +     4*16^9    +   13*16^10  + 10*16^11  + 4*16^12   + 15*16^13  + 9*16^14 + 10*16^15
 Or in reverse sum = first significant hex * 16 + next hex (repeat in iterations)
*/

/**
Register lifetimes and use:
  Start:
    x0 - Fibonacci buffer
    x1 - Previous fibonacci 
    --- 
  Fibonacci calculation: (do_fib)
    x3 - Next fib. buffer calculation
  Finish:
    x7 - fibonacci addr destination (msg=
    x6 loop counter
      Loop:
        w8 - Fibonnaci byte
        w9 - Fibonacci half byte
        w10 - Hex-Ascii conversion value
    Deciam lconverter:
      - w13 - Loaded first byte


  End:
    x16 - sys call
    x0 -
    x1 -  Params 
    x2 - 
*/

hex_sym_to_decimal:
cmp w9, #10
beq A
cmp w9, #11
beq B
cmp w9, #12
beq C
cmp w9, #13
beq D
cmp w9, #14
beq E
cmp w9, #15
beq F
b storemsg

A:
mov w10, #0x31

B:
mov w10, #0x32

C:
mov w10, #0x33

D:
mov w10, #0x34

E:
mov  w10, #0x35

F:
mov w10, #0x36

mov w21, #0x31
bfi w10, w21, #8, #8 // Second byte
ret

hex_to_decimal:
 mov x14, #0
 ubfx x13, x0, #0, #64
 
 ubfx x15, x13, #60, #4 // Get hex

 mov x16, #16
 mul_loop:
 cmp x14, #15
 mul x15, x15, x16
 add x14, x14, #1
 blo mul_loop 

ret

hex_to_ascii:
cmp w9, #10
bhs make_hex
mov w10, w9
add w10, w10, #0x30
b storemsg

make_hex:
add w10, w9, #0x37
b storemsg 

storemsg:
strb w10, [x7], #-1 // In reverse to undo little endian

ret


convert_to_ascii:

 ldrb w8, [sp, x6] // Load byte from fibonacci sequence in the stack
 
 ubfx w9, w8, #0, #4 // Obtain first hex digit
 bl hex_to_ascii

 ubfx w9, w8, #4, #4 // Second hex
 bl hex_to_ascii

 add x6, x6, #1
 b loop

_main:
  mov x0, #1 // Fib buffer
  mov x1, #0 // Prev

  do_fib:
  add x3, x0, x1 // Next in fib sequence
  cmp x3, x0 
  blo finish
  mov x1, x0 // Move previous t
  mov x0, x3  // Move in the calculated enxt
  b do_fib

  finish:
  sub sp, sp, #32
  str x0, [sp]
  adrp x7, msg@page // Obtain msg location
  add x7, x7, msg@pageoff
  add x7, x7, #15 // 16 bytes to be stored since one hex translates to two values (hex + 0x30 || 0x37)

  mov x6, #0 // Loop counter
  loop:
  cmp x6, #7 // (0-7) = 8 bytes in a 64-bit register
  bls convert_to_ascii

  bl hex_to_decimal
 

mov x16, #0x4 //write() syscall
mov x0, #1 //stdout
adrp x1, hex_msg@page
add x1, x1, hex_msg@pageoff
mov x2, #11
svc #0x80

mov x16, #0x4 //write() syscall
mov x0, #1 //stdout
adrp x1, msg@page
add x1, x1, msg@pageoff
mov x2, #16 // due to x7 over
svc #0x80

mov x16, #0x4 //write() syscall
mov x0, #1 //stdout
adrp x1, endline@page
add x1, x1, endline@pageoff
mov x2, #2
svc #0x80

add sp, sp, #32

mov x16, #0x1 //exit syscall
 mov x0, #0 //val
svc #0x80
