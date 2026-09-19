# ASM aarch64 (ARM 64) Guide
Might contain ARM64 XNU specifics because all code i write is written on MacOS.

Aarch64 Instruction size: 32 bits -> ex. ADD: 31 bit [opcode |24| sf |23| sh |22| imm12 |10| rN |5| rD] 0 bit
                                      Remember!! Memory stored as Little Eudian (Reversed)
                                          Here we see we have imm12 for our intermediate value. These blueprints can be found in the ARM manual.

Some instructions might have a suffix of 's'.
    - ex. sub wD, wD, imm -> subs wD, wD, imm
    - Will perform automatically a comparison:  cmp wD, 0

# Basic Vocabulary
half word = 2 b = 16 bits  
word = 4 b = 32 bits
double word = 8b = 64 bits

# Register types
 GPR - General purpose registers
 FPR - Floating point registers
 SPR - SPecial Purpose registers

  Higher bits - Lefthand side
  Lower bits - Righthand side

 Extended registers: xD - 64 bits
 Non-Extended registers: wD - 32 bits 
  - Utilize only the lower bits of the GPR.

# Signed & Unsigned values
  Signed - negative & positive
  Unsigned - positive and 0.

How are negative numbers represented?
    - The first bit contained in a register is 1
    - Value is treated as signed 

 Example: Non-Extended registers
 Range (Signed):   0x80000000 <-> 0xFFFFFFFF < 0x0 < 0x00000001 <-> 0x7FFFFFFF  (Remember 8 in Binary = 1000, F = 1111)
 Range (Unsigned): 0x0 < 0x00000001 <-> 0xFFFFFFFF

# Immidiate values
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

### Scaled Immediate values
Example: sSIMM12 - Scaled Signed 12-but Value
Means that the immediate value must be of a certain multiple.
    - Example: Extended value (Double WORD) - sSIMM12 must be a value that is a multiple of 8. (Lets say value 8: 8x8 = 64)
This is regularly used for value offsets (memory offsets) depending on bit context.

# Moving 

lsl - leftward bit shift

movk - move then keep ex. movk x1, #0xFFFF, lsl #32 -> 0x0000FFFF00000000
movz - move then zero (move and remove old)
  - mov x1, 0xFFFF00000000000 can be written as movz x1, 0xFFFF, lsl #48
  - Most likely converted to a standard move upon compilation


# Basic Stores and Loads

- Effective address (Architecture dependant! ARM64 - Always 64-bit): Addition of two source registers or addition of source register with an immediate value.
    - Think about this as address + offset. Written as [base, offset] <- brackets means to think of it as a memory address (dereferencing).

- Little Endian: Bytes are stored in reverse order. ARM64 uses this.
    - Despite this, loading them back reverses the bytes again.

- Store = Copy-paste from register to memory.
- Load = Copy-paste from memory to register.

Both stores and loads overwrite their destination.

## Pre & Post-indexing

Loads and stores can have additional operations done to them to update the source register's address right after the instruction.

 - Pre-indexing: Load/store to the EA, then increase/decrease the source register address/value by the IMM present in the instruction.
    - Example: str x0, [x1, 0x4]! <-- Notice the exclamation mark.
        EA = x1+0x4
        x0 stored in EA
        x1 incremented by 0x4.

 - Post-Indexing: Load/Store to the Source Register Address, then increase the source register address/value by the IMM present in the instruction.
    - Example: str x0, [x1], #0x4 
        EA = x1
        x0 stored in EA
        x1 incremented by 0x4.

## Vocabulary:
 - EA: Effective address

 - str: Store register
 - strh: Store register half word
 - strb: Store register byte

 - ldr: Load register
 - ldrh: Load register half word
 - ldrb: Load register byte

## Store Double-Word
    - str xD, [xA, UIMM12]
    - str xD, [xA, sSIMM12] -- Offset multiple of 8
    - str xD, [xA, xB]

## Load Double-Word
    - ldr xD, [xA, UIMM12]
    - ldr xD, [xA, sSIMM12] -- Offset multiple of 8
    - ldr xD, [xA, xB]

## Store Word
    - str wD, [xA, UIMM12]
    - str wD, [xA, sSIMM12] -- Offset multiple of 4
    - str wD, [xA, xB]

    Stores the ENTIRE 32 bits of the non-extended wD register.

## Load Word
    - ldr wD, [xA, UIMM12]
    - ldr wD, [xA, sSIMM12] -- Offset multiple of 4.
    - ldr wD, [xA, xB]

    Destination is non-extended, therefore the upper bits (destination register wD = xD) are ALWAYS set to zero.

## Store Halfword
    - strh wD, [xA, UIMM12]
    - strh wD, [xA, sSIMM12] -- Offset multiple of 2.
    - strh wD, [xA, xB]

    Stores the LOWER 16 bits of the non-extended wD register.

## Load Halfword
    - ldrh wD, [xA, UIMM12]
    - ldrh wD, [xA, sSIMM12] -- Offset multiple of 2.
    - ldrh wD, [xA, xB]

    Upper bits of xD are set to zero, aswell as the upper 16-bits of wD. Will result to 0x0000XXXX.

## Store Byte
    - strb wD, [xA, UIMM12]
    - strb wD, [xA, SIMM12] -- No scaled offset because its a multiple of 1.
    - strb wD, [xA, xB]

    Stores the LOWER 8 bits of the non-extended wD register.
    NOTICE: Little endian has no effect here.

## Load Byte
    - ldrb wD, [xA, UIMM12]
    - ldrb wD, [xA, sSIMM12] -- No scaled offset because its a multiple of 1.
    - ldrb wD, [xA, xB]

    Upper bits of xD are set to zero, asawell as the 24 upper bits of wD. Will result to 0x000000XX.


# Compares and branches

## Branches
Unconditional Branch: [b, SIMM]
 - Will always execute.
 - Includes itself, meaning $"b, 0x4"$ is just useless.

Labels:
    - Can omit calculating the jump address yourself.
    - Declared just like a function, ex. _main

Conditional branches

Branch if equal:
    - Syntax: beq <label>
    - Executes if and only if the previous condition was met (most recent compare).
    - Requires a comparison instruction beforehand.

And many more:

    beq = Equal
    bne = Not Equal
    bgt = Greater Than (signed)
    blt = Less Than (signed)
    bge = Greater Than (signed) or Equal
    ble = Less Than (signed) or Equal
    bhs = Unsigned Higher or Same (aka Unsigned Greater Than or Equal)
    blo = Unsigned Lower Than (aka Unsigned Less Than)
    bmi = Negative
    bpl = Positive or Zero (aka Not Negative)
    bvs = Signed Overflow
    bvc = Not Signed Overflow
    bhi = Unsigned Higher (aka Unsigned Greater Than)
    bls = Unsigned Lower or Same (aka Unsigned Less Than or Equal)
    bcs = Same thing as bhs (branch Carry Set)
    bcc = Same thing as blo (branch Carry Clear)
    bal = Always (same as just a unconditional branch) 
    
## Compares
   - Only two integer-based compare instructions for ARM64.

    - cmp xD, aimm
    - cmp wD, aimm
    - cmp xD, xA
    - cmp xD, wA

# Loops

Prerequesites:
 1. Start address where the contents are
 2. Start address where you want contents to be copied into
 3. Length of the contents which involve a GPR being used as a loop tracker
 4. Conditional branch
