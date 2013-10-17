/****************************************************************************
 *   $Id:: bod.h 4309 2012-10-31 01:02:28Z usb00423                         $
 *   Project: NXP LPC8xx BOD(Brown-OUt Detect) example
 *
 *   Description:
 *     This file contains headers for BOD related modules and definition.
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
#ifndef __BOD_H 
#define __BOD_H

#define DEEPSLEEP_BOD_WAKEUP	0
#define BOD_DEBUG				0

#define BOD_RESET_LED					7
#define POR_RESET_LED					8
#define BOD_INTERRUPT_LED			9
#define PWR_DOWN_BUTTON				15

#define BOD_INT_LVL0	(0<<2)		/* 1.65~1.80 */
#define BOD_INT_LVL1	(1<<2)		/* 2.22~2.35 */
#define BOD_INT_LVL2	(2<<2)		/* 2.52~2.66 */
#define BOD_INT_LVL3	(3<<2)		/* 2.80~2.90 */

#define BOD_RST_LVL0	0			/* 1.46~1.63 */
#define BOD_RST_LVL1	1			/* 2.06~2.15 */
#define BOD_RST_LVL2	2			/* 2.35~2.43 */
#define BOD_RST_LVL3	3			/* 2.63~2.71 */

#define BOD_RST_ENABLE	(0x1<<4)

/* For System Reset Source Identification or "SYSRSTSTAT" */
#define POR_RESET		(0x1<<0)
#define EXT_RESET		(0x1<<1)
#define WDT_RESET		(0x1<<2)
#define BOD_RESET		(0x1<<3)
#define SYS_RESET		(0x1<<4)
		
extern void BOD_IRQHandler(void);
extern void BOD_Init( void );

#endif /* end __BOD_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
