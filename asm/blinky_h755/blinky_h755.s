/*
GPIOB Address range: 0x58020400 - 0x580207FF
Offsets (GPIOx_MODER) 0x0 (Value 10 for bit 1, 0 for output mode) (GPIOx_ODR) 0x14, flip first bit;
GPIOB 0 - least significant bit

gpiob: gpio@58020400 {
				compatible = "st,stm32-gpio";
				gpio-controller;
				#gpio-cells = <2>;
				reg = <0x58020400 0x400>;
				clocks = <&rcc STM32_CLOCK(AHB4, 1)>;       <----------- REMEMBER DEPENDENCY
			};


From zephyr devicetree: (m7)

flash0: flash@8000000 {
				reg = <0x08000000 DT_SIZE_K(1024)>;
				ranges = <0 0x8000000 DT_SIZE_K(1024)>;
			};
		};

sram0: memory@24000000 {
		reg = <0x24000000 DT_SIZE_K(512)>;
		compatible = "zephyr,memory-region", "mmio-sram";
		zephyr,memory-region = "SRAM0";
	};


Ram : 512, 0x240000000
Flash : 1024 0x080000000

brew install arm-none-eabi-gcc
Assembly: arm-none-eabi-as -mcpu=cortex-m7 -mthumb blinky_h755.s -o blinky_h755.o
Dissasemble: arm-none-eabi-objdump -d blinky_h755.o
Link: arm-none-eabi-ld -T linker.ld blinky_h755.o -o blinky_h755.elf 
Make it a binary: arm-none-eabi-objcopy -O binary blinky_h755.elf blinky_h755.bin
*/

.cpu cortex-m7
.thumb

.data
  clock_cycle_count: .word 0
  led_state: .word 0

.text 

// Due to manual "booting" instead of a high level access, we have to create a vector table for instructions (etc etc. interrupts are also here)
// To tell the CPU where it should start executing and what after the boot hardware mechanism alerts it.
// Reference: https://github.com/STMicroelectronics/cmsis-device-h7/blob/master/Source/Templates/gcc/startup_stm32h755xx.s
//        Startup routine used by STCubeProgrammer  (aka. Im to unskilled to know how to implement a ARM vector table myself)


.global g_vectorTable
g_vectorTable:
  .word _estack
  .word Reset_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word SysTick_Handler // https://support.arm.com/documentation/dui0471/m/handling-processor-exceptions/configuring-systick


.thumb_func // Very specific for thumb instruction set, these have their least significant bit set.
Reset_Handler:
//  bl ExitRun0Mode // Configure PSU
//  bl SystemInit // Clock system initialization function

  ldr r0, =_estack // Set stack pointer
  mov sp, r0

  // GPIOB depends on AHB4 bus's clock - So i have to enable it 
  // bit 1 - RCC_AHB4ENR (AHB Clock Register) (GPIOB) - 0x0E0, chapter 9.7.42
  // RCC address: 0x58024400
  ldr r0, =0x58024400
  ldr r2, =0x0E0
  mov r1, #1
  lsl r1, r1, #1
  str r1, [r0, r2]

  ldr r3, =0xF42400 // Assuming clock is at full speed (480Mhz), this is a value of 16 million, meaning 3*100 exception counts should give delay of approx 1 second.
  // By trial and error i found out that 3 is type a second, so by the estimates the clock runs at 48Mhz? Im confused.

  ldr r0, =0xE000E014 // Systick reload value register
  strh r3, [r0]

  ldr r0, =0xE000E010 // Systick control and status register
  mov r1, #0
  add r1, r1, #1 // Enable processor clock [3]
  lsl r1, r1, #1

  add r1, r1, #1 // Enable SysTick exception assert [2]
  lsl r1, r1, #1
  
  add r1, r1, #1 // Enable counter
  str r1, [r0]

  ldr r0, =0x58020400 //GPIOB Address start 
  mov r1, #1
  str r1, [r0] // Set to output mode 

  add r0, r0, #0x14 // Offset for GPIOx Data Register

  main_loop:
  ldr r1, =clock_cycle_count
  ldr r2, [r1]
  cmp r2, #3  // Se over on r3 for a comment
  blo continue_process
  ldr r2, =led_state
  mov r4, #1  // Immediate cuz i get some weird error
  ldr r3, [r2]
  eor r3, r3, r4
  str r3, [r2]
  mov r5, #0 // Reset counter
  str r5, [r1]

  continue_process:
  ldr r1, =led_state
  ldr r2, [r1]
  str r2, [r0]
  b main_loop

.thumb_func
SysTick_Handler:
  ldr r0, =clock_cycle_count
  ldr r1, [r0]
  add r1, r1, #1
  str r1, [r0]
  bx lr // jump back to pc before exception
