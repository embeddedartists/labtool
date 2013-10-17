/****************************************************************************
 *   $Id:: LPC8xx_wkt.h 3635 2010-06-02 00:31:46Z usb00423              $
 *   Project: NXP LPC8xx software example
 *
 *   Description:
 *     This file contains definition and prototype for alarm/wake timer 
 *     configuration.
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
#ifndef __WKT_H 
#define __WKT_H

/* Control register bit definition. */
#define WKT_CLKSEL					(0x1<<0)
#define WKT_FLAG						(0x1<<1)
#define WKT_CLR							(0x1<<2)

void WKT_IRQHandler(void);
void init_wkt(uint32_t clkSrc, uint32_t timerInterval);
void halt_wkt(void);

#endif /* end __WKT_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
