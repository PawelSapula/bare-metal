.data
msg: .ascii "Write a 128 bit number (32-hex digits): "
input: .ascii ""
spacer1: .space 256
separator: .ascii "\n"
spacer2: .space 4
decimal_form: .ascii ""
spacer3: .space 256
rev_decimal_form: .ascii ""

.text
.global _main 

finish:
adrp x2, decimal_form@page
add x2, x2, decimal_form@pageoff
mov x3, #1 //(Not null value)
mov x5, #0

conversion_decimal:
mov x13, #10
cmp x3, 0

/* 
To get the 65th bit remainder into the division, we have to go a little unconvential way.
2**64 = 0x1 (trailed by 64 0´s) -> meaning a 65 bit value
Divided by 10 we get the hex 0x19999999(trail) as the quotient and remainder of 6.

From mathematics we can write the equivalent
2**64 = 10Q + 6

Our next value is then
R * 2**64 + x0
== R * (10Q + 6) + x0
== 10RQ * 6R + x0
  When divided we get:
== RQ + (6R + x0) / 10  <-- Calculate the 65 bit with this!

Now, due to division by 10, we know that the remainder can be 1-9.
Lets say remainder is R_max = 9:
  R_max * Q = 9 * 0x19999999(trail) = 0xE6666666(trail)

Now to the important details: Overflow
  From RQ (No overflow possible, see over)  +   (6R + x0) (Here overflow possible) / 10 (No overflow)
  Therefore we try adds (6R + x0) to see if overflow is possible.
                    ^ Standard add that might activate carry flag on overflow.

  Now when the value overflows, we know the following:
  x_0 + 6R = x_over + 2**64
  So the division looks like this:
  (x_0+6R)/10 = (x_over + 2**64) / 10
  Substitute for our remainder problem:
  (x_0+6R)/10 = (x_over + 10Q+6)/10     (2**64 = 10Q+6)
  (x_0)+R)/10 = Q + (x_over+6)/10

*/

beq done
udiv x3, x1, x13
// Xd = Xa - (Xn * Xm)
msub x4, x3, x13, x1 //x3 - quotient, x4 - remainder
mov x1, x3

// Handle 65th bit, see over //////////////////
mov x15, #0x9999  // 2**64/10
movk x15, #0x9999, lsl #16
movk x15, #0x9999, lsl #32
movk x15, #0x1999, lsl #48


mov x16, x4 // Move remainder
mov x17, #6 // Store remainder of 2**64/10

mul x19, x16, x17 // 6R //x19 to store OG value.
adds x18, x19, x0 // x0 + 6R

b.cs overflow // Branch if carry set
udiv x14, x18, x13 // (x0 + 6R) / 10
msub x4, x14, x13, x18  // Remainder
b finish_65

overflow:
add x18, x18, #6 // x_over + 6
udiv x14, x18, x13 // Divide (x_over + 6) / 10
msub x4, x14, x13, x18 // Remainder
add x14, x14, x15 // Add Q
b finish_65

finish_65:
mul x19, x16, x15 // RQ
add x14, x19, x14 // Full term -> RQ + (x0 + 6R) / 10
mov x0, x14 // Set to the new lower 64 bit value.
mov x3, x14

//////////////////////////////////////////////

//udiv x3, x0, x13 <-- The previous calculation now takes the lower bytes too.
//msub x4, x3, x13, x0
//mov x0, x3
lsl x5, x5, #8 // Ascii 
ubfx x6, x5, #56, #8 // Due to ascii conversion
cmp x6, 0
add x4, x4, #0x30 // ASCII
add x5, x5, x4
beq conversion_decimal 
rev x5, x5
str x5, [x2], #8
mov x5, #0
b conversion_decimal

done:
rev x5, x5
str x5, [x2], #8 // Store the remainder´s when they still havent filled out put computation os over

mov x3, x2 // Obtain size, mov finished addr here
adrp x1, decimal_form@page
add x1, x1, decimal_form@pageoff
mov x5, x1 // Copy addr
sub x3, x3, x1 // Get relative size
add x1, x1, x3  // Set to finish
sub x1, x1, #1 // Go back to final byte

adrp x4, rev_decimal_form@page
add x4, x4, rev_decimal_form@pageoff

reverse_loop:
ldrb w6, [x1], #-1
strb w6, [x4], #1

cmp x1, x5
blo done_reversing
b reverse_loop

done_reversing:

mov x16, #4
mov x0, #1
adrp x1, rev_decimal_form@page
add x1, x1, rev_decimal_form@pageoff
mov x2, x3
svc 0x80

mov x16, #4
mov x0, #1
adrp x1, separator@page
add x1, x1, separator@pageoff
mov x2, #2
svc 0x80

add sp, sp, #64

mov x16, #1
mov x0, #0
svc 0x80

ret


_main:
// write(4) / read(3) - int fd, user_addr_t cbuf, user_size_t nbyte
mov x16, #4 
mov x0, #1 // stdout
adrp x1, msg@page
add x1, x1, msg@pageoff
mov x2, #40 
svc 0x80

mov x16, #3 
mov x0, #0 // stdin
adrp x1, input@page
add x1, x1, input@pageoff
mov x2, #256 
svc 0x80

mov x0, #0
adrp x1, input@page
add x1, x1, input@pageoff
mov x2, x1 // Copy, for checking
add x1, x1, #31 // Move to last byte
b repeat

repeat:
cmp x1, x2
blo loaded
ldrb w0, [x1], #-1
cmp x0, 0x40
bhi sub_hex_val
sub x0, x0, #0x30
b cont

sub_hex_val: 
sub x0, x0, #0x40 
add x0, x0, #9
b cont

cont:
strb w0, [x1, #1] 
b repeat

loaded:
 // Here shrinking memory
 sub sp, sp, #16
 mov x3, sp
 mov x4, sp // Destination ref
 add x4, x4, #16
 adrp x1, input@page
 add x1, x1, input@pageoff
 
 shrink_loop:
 cmp x3, x4
 bhs main_2
 ldrb w0, [x1], #1
 ubfx w2, w0, #0, #4
 lsl w2, w2, #4
 ldrb w0, [x1], #1
 bfxil w2, w0, #0, #4 // Bitfield extract, insert low
 strb w2, [x3], #1
 b shrink_loop

main_2:
mov x0, #0 // Storage
mov x1, #0 // Storage Carry
mov x10, #17 // Number of bytes + 1

//mov x2, #0x2702 // Load F93
//movk x2, #0x22F1, lsl #16
//movk x2, #0xAD42, lsl #32
//movk x2, #0xA9F4, lsl #48

#sub sp, sp, #64
#str x2, [sp] 


mov x11, 0 // Loop counter

mov x7, sp
mov x8, sp // Last byte address
add x8, x8, #15 // 
bl loop

loop:
cmp x8, x7 // Safeguard for address outside the allocation
blo finish
add x11, x11, #1
cmp x11, x10
bhs finish
b read_hexes

read_hexes:
ldrb w3, [x7]
add x7,x7, #1 // Next byte

ubfx x5, x3, #4, #4 // First hex

bl mult
b loop

get_next:
ubfx x5, x3, #0, #4 // Second hex
ret


store_carry:
add x1, x1, x4
ret

mult_carry:
lsl x1, x1, #4
ret


mult:
ubfx x4, x0, #60, #4
lsl x0, x0, #4
add x0, x0, x5

bl mult_carry
bl store_carry

bl get_next
ubfx x4, x0, #60, #4
lsl x0, x0, #4
add x0, x0, x5


bl mult_carry
bl store_carry

bl loop




