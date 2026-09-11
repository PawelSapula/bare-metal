
.data
  msg: .ascii "Hello World!\n"

.text
.global _main

// Arm64 - [syscall - x8], x0 - return, arg0, x1 - arg1, x2 - arg2

_main:
  mov x8, #0x40
  mov x0, #0x1 // fd
  adrp x1, msg@PAGE
  add x1, x1, msg@PAGEOFF
  mov x2, #14
  svc 0x80

  mov x8, #0x1
  mov x0, #0
  svc 0x80
