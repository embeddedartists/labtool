/****************************************************************************
 *   $Id:: lpc8xx_gpio.h 5137 2012-10-31 00:15:18Z nxp28433                        $
 *   Project: NXP LPC8xx software example
 *
 *   Description:
 *     This file contains definition and prototype for GPIO.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation is hereby granted, under NXP Semiconductors' 
 * relevant copyright in the software, without fee, provided that it 
 * is used in conjunction with NXP Semiconductors microcontrollers. This 
 * copyright, permission, and disclaimer notice must appear in all copies of 
 * this code.
 
****************************************************************************/
#ifndef __LPC8XX_GPIO_H 
#define __LPC8XX_GPIO_H

#define CHANNEL0	0
#define CHANNEL1	1
#define CHANNEL2	2
#define CHANNEL3	3
#define CHANNEL4	4
#define CHANNEL5	5
#define CHANNEL6	6
#define CHANNEL7	7

#define SLICE0	0
#define SLICE1	1
#define SLICE2	2
#define SLICE3	3
#define SLICE4	4
#define SLICE5	5
#define SLICE6	6
#define SLICE7	7

#define SEL_PMATCH				(0x1<<0)
#define ENA_PXEV					(0x1<<1)

#define BIT_SLICE0	8
#define BIT_SLICE1	11
#define BIT_SLICE2	14
#define BIT_SLICE3	17
#define BIT_SLICE4	20
#define BIT_SLICE5	23
#define BIT_SLICE6	26
#define BIT_SLICE7	29

#define PATTERN_CONST_1		(0x0UL)
#define PATTERN_RISING		(0x1UL)
#define PATTERN_FALLING		(0x2UL)
#define PATTERN_R_OR_F		(0x3UL)
#define PATTERN_LEVEL_HI	(0x4UL)
#define PATTERN_LEVEL_LO	(0x5UL)
#define PATTERN_CONST_0		(0x6UL)
#define PATTERN_EVENT			(0x7UL)

#define INT_CHANNEL_NUM		8

#define PORT0			0

#define DGF_CHANNEL0	6
#define DGF_CHANNEL1	5
#define DGF_CHANNEL2	4
#define DGF_CHANNEL3	3
#define DGF_CHANNEL4	2
#define DGF_CHANNEL5	1
#define DGF_CHANNEL6	0

#define DGF_CHANNEL_NUM		7

void PININT_Handler ( uint32_t irq_num );
void PININT0_IRQHandler(void);
void PININT1_IRQHandler(void);
void PININT2_IRQHandler(void);
void PININT3_IRQHandler(void);
void PININT4_IRQHandler(void);
void PININT5_IRQHandler(void);
void PININT6_IRQHandler(void);
void PININT7_IRQHandler(void);

void GPIOInit( void );
void GPIOSetPinInterrupt( uint32_t channelNum, uint32_t portNum, uint32_t bitPosi,
		uint32_t sense, uint32_t event );
void GPIOPinIntEnable( uint32_t channelNum, uint32_t event );
void GPIOPinIntDisable( uint32_t channelNum, uint32_t event );
uint32_t GPIOPinIntStatus( uint32_t channelNum );
void GPIOPinIntClear( uint32_t channelNum );

uint32_t GPIOGetPinValue( uint32_t portNum, uint32_t bitPosi );
void GPIOSetBitValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal );
void GPIOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir );

void GPIOPatternMatchInit( void );
void GPIOSetPatternMatchInput( uint32_t channelNum, uint32_t portNum, uint32_t bitPosi );
void GPIOSetPatternMatchSlice( uint32_t channelNum, uint32_t sliceNum, uint32_t condition, uint32_t isLast );		
void GPIOPatternMatchEnable( uint32_t PMatchSel, uint32_t RxEvEna );

#endif /* end __LPC8XX_GPIO_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
