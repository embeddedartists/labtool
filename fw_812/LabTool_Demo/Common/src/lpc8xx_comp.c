/****************************************************************************
 *   $Id:: lpc8xx_comp.c 8973 2012-10-31 00:13:56Z usb00423                         $
 *   Project: NXP LPC8xx CMP example
 *
 *   Description:
 *     This file contains Comparator code example which include Comparator 
 *     initialization, COMP interrupt handler, and APIs.
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
#include "LPC8xx.h"			/* LPC8xx Peripheral Registers */
#include "lpc8xx_nmi.h"
#include "lpc8xx_comp.h"

/* statistics of all the interrupts */
volatile uint32_t CompStatCnt = 0;
volatile uint32_t CompEdgeCnt = 0;
/*****************************************************************************
** Function name:		CMP_IRQHandler
**
** Descriptions:		Comparator interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void CMP_IRQHandler(void) 
{
  uint32_t regVal;

  regVal = LPC_CMP->CTRL;
  if (regVal & COMPSTAT)
  {
		CompStatCnt++;
  }
  if (regVal & COMPEDGE)
  {
		CompEdgeCnt++;
		LPC_CMP->CTRL = regVal | EDGECLR;
		LPC_CMP->CTRL = regVal & ~EDGECLR;
  }
  return;
}

/*****************************************************************************
** Function name:		COMP_Init
**
** Descriptions:		Comparator initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void COMP_Init( void )
{
  uint32_t regVal;
   
  /* Comparator should be powered up first. Use of comparator requires BOD */
  LPC_SYSCON->PDRUNCFG &= ~( (0x1 << 15) | (0x1 << 3) );
  /* Enable AHB clock to the Comparator. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (0x1 << 19);
  /* Peripheral reset control to Comparator, a "1" bring it out of reset. */
  LPC_SYSCON->PRESETCTRL &= ~(0x1 << 12);
  LPC_SYSCON->PRESETCTRL |= (0x1 << 12);

	/*connect the Comparator sigals to port pins(P0.0, P0.1, P0.7)*/
	LPC_IOCON->PIO0_0 &= ~(0x3 << 3);
	LPC_IOCON->PIO0_1 &= ~(0x3 << 3);
  LPC_SWM->PINENABLE0 &= ~(0x3 << 0); /* P0.0 is COMP_0a input; P0.1 is COMP_0b input */
  regVal = LPC_SWM->PINASSIGN8 & ~(0xFF << 8);
  LPC_SWM->PINASSIGN8 = regVal | (0x7 << 8);	/* P0.7 is comparator output */

  /* Ladder enable, voltage ladder value is VDDCMP_CMP */
#ifdef __COMP_VREF
	LPC_IOCON->PIO0_6 &= ~(0x3 << 3);
  LPC_SWM->PINENABLE0 &= ~(0x1 << 8); /* P0.6 is COMP_VREF input */
  LPC_CMP->LAD = 0x1 | (0x1C<<1) | (0x1<<6);
#else
  LPC_CMP->LAD = 0x1 | (0x1C<<1);
#endif
  return;
}

/*****************************************************************************
** Function name:		COMP_SelectInput
**
** Descriptions:		Select comparator input channel
**
** parameters:			comp_channel, input
** Returned value:		None
** 
*****************************************************************************/
void COMP_SelectInput( uint32_t comp_channel, uint32_t input )
{
  switch ( comp_channel )
  {
	case COMP_VP:
	  LPC_CMP->CTRL &= ~0x700;
	  LPC_CMP->CTRL |= ((0x7 & input) << 8);
	break;
	case COMP_VM:
	  LPC_CMP->CTRL &= ~0x3800;
	  LPC_CMP->CTRL |= ((0x7 & input) << 11);
	break;
	default:
	break;
  }
  return;
}

/*****************************************************************************
** Function name:		COMP_SetOutput
**
** Descriptions:		Set output async or sync
**
** parameters:			sync
** Returned value:		None
** 
*****************************************************************************/
void COMP_SetOutput( uint32_t sync )
{
  if ( sync == 0 )
  {
		LPC_CMP->CTRL &= ~COMPSA;
  }
  else
  {
		LPC_CMP->CTRL |= COMPSA;
  }
  return;
}

/*****************************************************************************
** Function name:		COMP_SetInterrupt
**
** Descriptions:		Set interrupt single/double, polarity.
**						single or double edge: 0 is single, 1 is double. 
**						rising or falling edge: 0 is falling, 1 is rising.
**
** parameters:			single, event
** Returned value:		None
** 
*****************************************************************************/
void COMP_SetInterrupt( uint32_t single, uint32_t event )
{
  if ( single == 0 )
  {
		LPC_CMP->CTRL &= ~0x10;
  }
  else
  {
		LPC_CMP->CTRL |= 0x10;
  }
  if ( event == 1 )
  {
		LPC_CMP->CTRL |= 0x8;
  }
  else
  {
		LPC_CMP->CTRL &= ~0x8;
  }

  /* Enable the COMP Interrupt */
#if NMI_ENABLED
	NVIC_DisableIRQ( CMP_IRQn );
	NMI_Init( CMP_IRQn );
#else
  NVIC_EnableIRQ( CMP_IRQn );
#endif	
  return;
}

/*****************************************************************************
** Function name:		COMP_SetHysteresis
**
** Descriptions:		Set hysteresis
**
** parameters:			level
** Returned value:		None
** 
*****************************************************************************/
void COMP_SetHysteresis( uint32_t level )
{
  LPC_CMP->CTRL &= ~(0x3 << 25);
  LPC_CMP->CTRL |= (level << 25);
  return;
}

/*****************************************************************************
** Function name:		COMP_GetOutputStatus
**
** Descriptions:		Get output status
**
** parameters:			None
** Returned value:		Output status
** 
*****************************************************************************/
uint32_t COMP_GetOutputStatus( void )
{
  uint32_t regVal = 0;
  
  regVal = ( LPC_CMP->CTRL & COMPSTAT ) >> 21;

  return regVal;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

