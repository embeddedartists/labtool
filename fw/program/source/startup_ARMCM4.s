/* File: startup_ARMCM4.S
 * Purpose: startup file for Cortex-M4 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V1.3
 * Date: 08 Feb 2012
 *
 * Copyright (c) 2012, ARM Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the ARM Limited nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ARM LIMITED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
    .syntax unified
    .arch armv7-m

    .section .stack
    .align 3
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0x800
#endif
    .globl    __StackTop
    .globl    __StackLimit
__StackLimit:
    .space    Stack_Size
    .size __StackLimit, . - __StackLimit
__StackTop:
    .size __StackTop, . - __StackTop

    .section .heap
    .align 3
#ifdef __HEAP_SIZE
    .equ    Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0x000
#endif
    .globl    __HeapBase
    .globl    __HeapLimit
__HeapBase:
    .if    Heap_Size
    .space    Heap_Size
    .endif
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    .size __HeapLimit, . - __HeapLimit

    .section .isr_vector
    .align 2
    .globl __isr_vector
__isr_vector:
    .long    __StackTop            /* Top of Stack */
    .long    Reset_Handler             	/* 1 Reset Handler */
    .long    NMI_Handler               	/* 2 NMI Handler */
    .long    HardFault_Handler         	/* 3 Hard Fault Handler */
    .long    MemManage_Handler         	/* 4 MPU Fault Handler */
    .long    BusFault_Handler          	/* 5 Bus Fault Handler */
    .long    UsageFault_Handler        	/* 6 Usage Fault Handler */
    .long    Sign_Value                	/* 7 Reserved */
    .long    0                         	/* 8 Reserved */
    .long    0                         	/* 9 Reserved */
    .long    0                         	/* 10 Reserved */
    .long    SVC_Handler               	/* 11 SVCall Handler */
    .long    DebugMon_Handler          	/* 12 Debug Monitor Handler */
    .long    0                         	/* 13 Reserved */
    .long    PendSV_Handler            	/* 14 PendSV Handler */
    .long    SysTick_Handler           	/* 15 SysTick Handler */

    /* External interrupts */
    .long    DAC_IRQHandler	 			/* 16 D/A Converter */
    .long    M0CORE_IRQHandler			/* 17 M0 Core */
    .long    DMA_IRQHandler				/* 18 General Purpose DMA */
    .long    EZH_IRQHandler				/* 19 EZH/EDM */
    .long    FLASH_EEPROM_IRQHandler	/* 20 Reserved for Typhoon */
    .long    ETH_IRQHandler				/* 21 Ethernet */
    .long    SDIO_IRQHandler			/* 22 SD/MMC */
    .long    LCD_IRQHandler				/* 23 LCD */
    .long    USB0_IRQHandler			/* 24 USB0 */
    .long    USB1_IRQHandler			/* 25 USB1 */
    .long    SCT_IRQHandler				/* 26 State Configurable Timer */
    .long    RIT_IRQHandler				/* 27 Repetitive Interrupt Timer */
    .long    TIMER0_IRQHandler			/* 28 Timer0 */
    .long    TIMER1_IRQHandler			/* 29 Timer1 */
    .long    TIMER2_IRQHandler			/* 30 Timer2 */
    .long    TIMER3_IRQHandler			/* 31 Timer3 */
    .long    MCPWM_IRQHandler			/* 32 Motor Control PWM */
    .long    ADC0_IRQHandler			/* 33 A/D Converter 0 */
    .long    I2C0_IRQHandler			/* 34 I2C0 */
    .long    I2C1_IRQHandler			/* 35 I2C1 */
    .long    SPI_IRQHandler				/* 36 SPI */
    .long    ADC1_IRQHandler			/* 37 A/D Converter 1 */
    .long    SSP0_IRQHandler			/* 38 SSP0 */
    .long    SSP1_IRQHandler			/* 39 SSP1 */
    .long    UART0_IRQHandler			/* 40 UART0 */
    .long    UART1_IRQHandler			/* 41 UART1 */
    .long    UART2_IRQHandler			/* 42 UART2 */
    .long    UART3_IRQHandler			/* 43 UART3 */
    .long    I2S0_IRQHandler			/* 44 I2S0 */
    .long    I2S1_IRQHandler			/* 45 I2S1 */
    .long    SPIFI_IRQHandler			/* 46 SPI Flash Interface */
    .long    SGPIO_IRQHandler			/* 47 SGPIO */
    .long    GPIO0_IRQHandler			/* 48 GPIO0 */
    .long    GPIO1_IRQHandler			/* 49 GPIO1 */
    .long    GPIO2_IRQHandler			/* 50 GPIO2 */
    .long    GPIO3_IRQHandler			/* 51 GPIO3 */
    .long    GPIO4_IRQHandler			/* 52 GPIO4 */
    .long    GPIO5_IRQHandler			/* 53 GPIO5 */
    .long    GPIO6_IRQHandler			/* 54 GPIO6 */
    .long    GPIO7_IRQHandler			/* 55 GPIO7 */
    .long    GINT0_IRQHandler			/* 56 GINT0 */
    .long    GINT1_IRQHandler			/* 57 GINT1 */
    .long    EVRT_IRQHandler			/* 58 Event Router */
    .long    CAN1_IRQHandler			/* 59 C_CAN1 */
    .long    0							/* 60 Reserved */
    .long    VADC_IRQHandler			/* 61 VADC */
    .long    ATIMER_IRQHandler			/* 62 ATIMER */
    .long    RTC_IRQHandler				/* 63 RTC */
    .long    0							/* 64 Reserved */
    .long    WDT_IRQHandler				/* 65 WDT */
    .long    M0s_IRQHandler				/* 66 M0s */
    .long    CAN0_IRQHandler			/* 67 C_CAN0 */
    .long    QEI_IRQHandler				/* 68 QEI */

    .size    __isr_vector, . - __isr_vector

    .text
    .thumb
    .thumb_func
    .align 2
    .globl    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

#if 1
/* Here are two copies of loop implemenations. First one favors code size
 * and the second one favors performance. Default uses the first one.
 * Change to "#if 0" to use the second one */
.flash_to_ram_loop:
    cmp     r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt    .flash_to_ram_loop
#else
/*    subs    r3, r2
    ble    .flash_to_ram_loop_end
.flash_to_ram_loop:
    subs    r3, #4
    ldr    r0, [r1, r3]
    str    r0, [r2, r3]
    bgt    .flash_to_ram_loop
.flash_to_ram_loop_end:*/
#endif

#ifndef __NO_SYSTEM_INIT
/*    ldr    r0, =SystemInit
    blx    r0*/
#endif

    ldr    r0, =_start
    bx    r0
    .pool
    .size Reset_Handler, . - Reset_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro    def_irq_handler    handler_name
    .align 1
    .thumb_func
    .weak    \handler_name
    .type    \handler_name, %function
\handler_name :
    b    .
    .size    \handler_name, . - \handler_name
    .endm

    def_irq_handler    NMI_Handler
    def_irq_handler    HardFault_Handler
    def_irq_handler    MemManage_Handler
    def_irq_handler    BusFault_Handler
    def_irq_handler    UsageFault_Handler
    def_irq_handler    Sign_Value
    def_irq_handler    SVC_Handler
    def_irq_handler    DebugMon_Handler
    def_irq_handler    PendSV_Handler
    def_irq_handler    SysTick_Handler
    def_irq_handler    Default_Handler

    def_irq_handler    DAC_IRQHandler
    def_irq_handler    M0CORE_IRQHandler
    def_irq_handler    DMA_IRQHandler
    def_irq_handler    EZH_IRQHandler
    def_irq_handler    FLASH_EEPROM_IRQHandler
    def_irq_handler    ETH_IRQHandler
    def_irq_handler    SDIO_IRQHandler
    def_irq_handler    LCD_IRQHandler
    def_irq_handler    USB0_IRQHandler
    def_irq_handler    USB1_IRQHandler
    def_irq_handler    SCT_IRQHandler
    def_irq_handler    RIT_IRQHandler
    def_irq_handler    TIMER0_IRQHandler
    def_irq_handler    TIMER1_IRQHandler
    def_irq_handler    TIMER2_IRQHandler
    def_irq_handler    TIMER3_IRQHandler
    def_irq_handler    MCPWM_IRQHandler
    def_irq_handler    ADC0_IRQHandler
    def_irq_handler    I2C0_IRQHandler
    def_irq_handler    I2C1_IRQHandler
    def_irq_handler    SPI_IRQHandler
    def_irq_handler    ADC1_IRQHandler
    def_irq_handler    SSP0_IRQHandler
    def_irq_handler    SSP1_IRQHandler
    def_irq_handler    UART0_IRQHandler
    def_irq_handler    UART1_IRQHandler
    def_irq_handler    UART2_IRQHandler
    def_irq_handler    UART3_IRQHandler
    def_irq_handler    I2S0_IRQHandler
    def_irq_handler    I2S1_IRQHandler
    def_irq_handler    SPIFI_IRQHandler
    def_irq_handler    SGPIO_IRQHandler
    def_irq_handler    GPIO0_IRQHandler
    def_irq_handler    GPIO1_IRQHandler
    def_irq_handler    GPIO2_IRQHandler
    def_irq_handler    GPIO3_IRQHandler
    def_irq_handler    GPIO4_IRQHandler
    def_irq_handler    GPIO5_IRQHandler
    def_irq_handler    GPIO6_IRQHandler
    def_irq_handler    GPIO7_IRQHandler
    def_irq_handler    GINT0_IRQHandler
    def_irq_handler    GINT1_IRQHandler
    def_irq_handler    EVRT_IRQHandler
    def_irq_handler    CAN1_IRQHandler
/*     def_irq_handler    RESERVED3			*/
    def_irq_handler    VADC_IRQHandler
    def_irq_handler    ATIMER_IRQHandler
    def_irq_handler    RTC_IRQHandler
/*     def_irq_handler    RESERVED4			*/
    def_irq_handler    WDT_IRQHandler
    def_irq_handler    M0s_IRQHandler
    def_irq_handler    CAN0_IRQHandler
    def_irq_handler    QEI_IRQHandler

    .end
