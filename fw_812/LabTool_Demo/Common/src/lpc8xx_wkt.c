/****************************************************************************
 *   $Id:: LPC8xx_wkt.c 5543 2010-11-09 02:19:19Z usb00423                  $
 *   Project: NXP LPC8xx alarm/wake timer example
 *
 *   Description:
 *     This file contains alarm/wake timer code example which include timer 
 *     initialization, timer interrupt handler, and related APIs for 
 *     timer setup.
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
#include "LPC8xx.h"
#include "lpc8xx_nmi.h"
#include "lpc8xx_wkt.h"

/******************************************************************************
** Function name:		WKT_IRQHandler
**
** Descriptions:		alarm/wake timer interrupt handler
**
** parameters:			None
** Returned value:	None
** 
******************************************************************************/
void WKT_IRQHandler(void)
{
  if ( LPC_WKT->CTRL & WKT_FLAG )
  {
		LPC_WKT->CTRL |= WKT_FLAG;			/* clear interrupt flag */
  }
  return;
}

/******************************************************************************
** Function name:		init_wkt
**
** Descriptions:		Initialize timer, select clock source, set timer interval,
**									install timer interrupt handler
**
** parameters:			clock source and timer interval
** Returned value:	None
** 
******************************************************************************/
void init_wkt(uint32_t clkSrc, uint32_t timerInterval) 
{
  LPC_SYSCON->SYSAHBCLKCTRL |= (0x1 << 9);
	LPC_SYSCON->PRESETCTRL &= ~(0x1 << 9);
	LPC_SYSCON->PRESETCTRL |= (0x1 << 9);

	if ( clkSrc & WKT_CLKSEL )
	{
		LPC_WKT->CTRL |= WKT_CLKSEL;
	}
	else
	{
		LPC_WKT->CTRL &= ~WKT_CLKSEL;
	}
  LPC_WKT->COUNT = timerInterval;
	
  /* Enable the WKT Interrupt */
#if NMI_ENABLED
	NVIC_DisableIRQ( WKT_IRQn );
	NMI_Init( WKT_IRQn );
#else
  NVIC_EnableIRQ(WKT_IRQn);
#endif
  return;
}

/******************************************************************************
** Function name:		halt_wkt
**
** Descriptions:		Halt timer
**
** parameters:			None
** Returned value:	None
** 
******************************************************************************/
void halt_wkt(void) 
{
	LPC_WKT->CTRL |= WKT_CLR;
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
