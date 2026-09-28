arm-none-eabi-as -mcpu=cortex-m7 -mthumb blinky_h755.s -o blinky_h755.o
arm-none-eabi-ld -T linker.ld blinky_h755.o -o blinky_h755.elf
arm-none-eabi-objdump -d blinky_h755.elf
