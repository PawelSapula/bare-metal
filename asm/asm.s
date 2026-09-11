.align 2

.data
  msg: .asciz "Hello World!\n"

.text
.global _main

_main:
    mov x16, #0x4 //write() syscall
    mov x0, #1 //stdout
    adrp x1, msg@PAGE
    add x1, x1, msg@PAGEOFF
    mov x2, #16 // Size
    svc #0x80

    mov x3, #4
    add x3, x3, #4
    add x3, x3, #48 // Convert to ASCII
    str x3, [sp, #-16]! // Store register, move stack pointer amount (given by ! and -1)

    mov x16, #0x4
    mov x0, #1
    mov x1, sp // Move sp pointer to x1
    mov x2, #1 // Just one byte, symbol ´8´
    svc #010
    
    add sp, sp, #16 // Restore stack pointer

    mov x16, #0x1 // syscall exit
    mov x0, #0xDEAD
    svc #0x80

