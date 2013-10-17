/****************************************************************************
 *   $Id:: lpc8xx_nmi.c 7227 2011-04-27 20:20:38Z usb01267                         $
 *   Project: NXP LPC8xx NMI interrupt example
 *
 *   Description:
 *     This file contains NMI interrupt handler code example.
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
#include "lpc8xx_mrt.h"
#include "lpc8xx_uart.h"
#include "lpc8xx_spi.h"
#include "lpc8xx_i2c.h"
#include "lpc8xx_bod.h"
#include "lpc8xx_wdt.h"
#include "lpc8xx_gpio.h"
#include "lpc8xx_wkt.h"
#include "lpc8xx_comp.h"
#include "lpc8xx_nmi.h"

#if NMI_ENABLED
volatile uint32_t NMI_Counter[MAX_NMI_NUM];

/*****************************************************************************
** Function name:		NMI_Handler
**
** Descriptions:		NMI interrupt handler
** parameters:		None			 
** 						
** Returned value:	None
** 
*****************************************************************************/
void NMI_Handler( void )
{
  uint32_t regVal;

  regVal = LPC_SYSCON->NMISRC;
  regVal &=	~0x80000000;
  if ( regVal < MAX_NMI_NUM )
  {
		if ( regVal == MRT_IRQn )
		{
			MRT_IRQHandler();
		}
		if ( regVal == UART0_IRQn )
		{
			UART0_IRQHandler();
			//UART_Handler(LPC_USART0);
		}
		if ( regVal == UART1_IRQn )
		{
			UART1_IRQHandler();
			//UART_Handler(LPC_USART1);
		}
		if ( regVal == UART2_IRQn )
		{
			UART2_IRQHandler();
			//UART_Handler(LPC_USART2);
		}
		if ( regVal == SPI0_IRQn )
		{
			SPI0_IRQHandler();
			//SPI_Handler(LPC_SPI0);
		}
		if ( regVal == SPI1_IRQn )
		{
			SPI1_IRQHandler();
			//SPI_Handler(LPC_SPI1);
		}
		if ( regVal == I2C_IRQn )
		{
			I2C_IRQHandler();
		}
		if ( regVal == SCT_IRQn )
		{
			SCT_IRQHandler();
		}
		if ( regVal == BOD_IRQn )
		{
			BOD_IRQHandler();
		}
		if ( regVal == WDT_IRQn )
		{
			WDT_IRQHandler();
		}
		if ( (regVal >= PININT0_IRQn) && (regVal <= PININT7_IRQn) )
		{
			PININT_Handler ( regVal - PININT0_IRQn );
		}
		if ( regVal == WKT_IRQn )
		{
			WKT_IRQHandler();
		}
		if ( regVal == CMP_IRQn )
		{
			CMP_IRQHandler();
		}
		NMI_Counter[regVal]++; 
  }
  return;
}

/*****************************************************************************
** Function name:		NMI_Init
**
** Descriptions:		NMI initialization
** parameters:			NMI number			 
** 						
** Returned value:		None
** 
*****************************************************************************/
void NMI_Init( uint32_t NMI_num )
{
  uint32_t i;

  for ( i = 0; i < MAX_NMI_NUM; i++ )
  {
    NMI_Counter[i] = 0x0;
  }
  LPC_SYSCON->NMISRC = 0x80000000|NMI_num;
  return;
}

#endif
