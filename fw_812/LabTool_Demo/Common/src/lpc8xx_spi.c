/**********************************************************************
* $Id$		lpc8xx_spi.c		2012-10-31
*//**
* @file		lpc8xx_spi.c
* @brief	Contains all functions support for SSP firmware library on lpc8xx
* @version	1.0
* @date		31. October. 2012
* @author	NXP MCU SW Application Team
*
* Copyright(C) 2012, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
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
* documentation is hereby granted, under NXP Semiconductors’
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup SPI
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc8xx.h"
#include "lpc8xx_nmi.h"
#include "lpc8xx_spi.h"

/* To be addressed later. For now, all the interrupt flags are shared by 
	both SPI0 and SPI1. Only one can run at a time. */	
	
volatile uint32_t SPIInterruptCount = 0;
volatile uint32_t SPIRXCount = 0;
volatile uint32_t SPITXCount = 0;
volatile uint32_t SPISSELAssertCount = 0;
volatile uint32_t SPISSELDeassertCount = 0;
volatile uint32_t SPIRXOverrunCount = 0;
volatile uint32_t SPITXUnderrunCount = 0;
#if STALL_ENABLE
volatile uint32_t SPIClkStallCount = 0;
#endif

volatile uint32_t rxrdy = 0, txrdy = 0;

/*****************************************************************************
** Function name:		SPI_Handler
**
** Descriptions:		Generic handler for all SPI port
**
** parameters:			LPC_SPI_TypeDef
** Returned value:	None
** 
*****************************************************************************/
void SPI_Handler(LPC_SPI_TypeDef *SPIx)
{
	uint32_t active = SPIx->INTSTAT;
	
	SPIInterruptCount++;
	if ( active & STAT_ERROR_MASK )
	{
		if(active & STAT_RXOVERRUN) {
			SPIRXOverrunCount++;
			SPIx->STAT = STAT_RXOVERRUN;
//		return;			/* If overun or undedrun occurs, the data is invalid. Skip return for debugging. */
		}
		if(active & STAT_TXUNDERRUN) {
			SPITXUnderrunCount++;
			SPIx->STAT = STAT_TXUNDERRUN;
//		return;			/* If overun or undedrun occurs, the data is invalid. Skip return for debugging. */
		}
#if STALL_ENABLE
		if(active & STAT_CLKSTALL) {
			SPIClkStallCount++;
			SPIx->STAT = STAT_CLKSTALL;
			SPIx->INTENCLR = STAT_CLKSTALL;
		}
#endif
		if(active & STAT_SELNASSERT) {
			SPISSELAssertCount++;
			SPIx->STAT = STAT_SELNASSERT;
		}
		if(active & STAT_SELNDEASSERT) {
			SPISSELDeassertCount++;
			SPIx->STAT = STAT_SELNDEASSERT;
		}
	}
  if(active & STAT_RXRDY) {
		SPIRXCount++;
		rxrdy = 1;
		SPIx->INTENCLR = STAT_RXRDY;
  }
  if(active & STAT_TXRDY) {
		SPITXCount++;
		txrdy = 1;
		SPIx->INTENCLR = STAT_TXRDY;
  }
	return;
}

/*****************************************************************************
** Function name:		SPI0_IRQHandler
**
** Descriptions:		SPI interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SPI0_IRQHandler(void)
{
	SPI_Handler(LPC_SPI0);
  return;
}

/*****************************************************************************
** Function name:		SPI1_IRQHandler
**
** Descriptions:		SPI interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SPI1_IRQHandler(void)
{
	SPI_Handler( LPC_SPI1 );
  return;
}

/*****************************************************************************
** Function name:		SPI_Init
**
** Descriptions:		SPI port initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void SPI_Init( LPC_SPI_TypeDef *SPIx, uint32_t div, uint32_t cfg, uint32_t dly )
{
	/* Set up clock and power for SSP1 module, M3 clock is all we need for LSPI */

	if ( SPIx == LPC_SPI0 )
	{
		/* Enable SPI clock */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);
		/* Peripheral reset control to SPI, a "1" bring it out of reset. */
		LPC_SYSCON->PRESETCTRL &= ~(0x1<<0);
		LPC_SYSCON->PRESETCTRL |= (0x1<<0);

	}
	else if ( SPIx == LPC_SPI1 )
	{
		/* Enable SPI clock */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
		/* Peripheral reset control to SPI, a "1" bring it out of reset. */
		LPC_SYSCON->PRESETCTRL &= ~(0x1<<1);
		LPC_SYSCON->PRESETCTRL |= (0x1<<1);
	}
	if ( cfg & CFG_MASTER )
	{
    /* divider and delay registers are for master mode only. */		
    SPIx->DIV = div;
		SPIx->DLY = dly;
	}
  SPIx->CFG = (cfg & ~CFG_ENABLE);
	/* For now, txrdy and rxrdy are shared by both SPI0 and SPI1. Only one 
	running at a time. */
	rxrdy = txrdy = 0;

#if SPI_INTERRUPT
#if STALL_ENABLE
  SPIx->INTENSET = STAT_RXRDY | STAT_TXRDY | STAT_RXOVERRUN | STAT_CLKSTALL
			| STAT_SELNASSERT | STAT_SELNDEASSERT;
#else
//  SPIx->INTENSET = STAT_RXRDY | STAT_TXRDY | STAT_RXOVERRUN | STAT_TXUNDERRUN 
//			| STAT_SELNASSERT | STAT_SELNDEASSERT;
  SPIx->INTENSET = STAT_RXRDY | STAT_TXRDY | STAT_RXOVERRUN;
#endif
	if ( SPIx == LPC_SPI0 )
	{
		NVIC_DisableIRQ(SPI0_IRQn);
		NVIC_ClearPendingIRQ(SPI0_IRQn);
#if NMI_ENABLED
		NMI_Init( SPI0_IRQn );
#else
		NVIC_EnableIRQ(SPI0_IRQn);
#endif
	}
	else if ( SPIx == LPC_SPI1 )
	{
		NVIC_DisableIRQ(SPI1_IRQn);
		NVIC_ClearPendingIRQ(SPI1_IRQn);
#if NMI_ENABLED
		NMI_Init( SPI1_IRQn );
#else
		NVIC_EnableIRQ(SPI1_IRQn);
#endif
	}
#else
  SPIx->INTENCLR = STAT_RXRDY | STAT_TXRDY | STAT_RXOVERRUN | STAT_TXUNDERRUN 
			| STAT_SELNASSERT | STAT_SELNDEASSERT;
#endif
  SPIx->CFG |= CFG_ENABLE;
  return;
}

/*****************************************************************************
** Function name:		SPI_SendRcv
**
** Descriptions:		Send a block of data to the SPI port, the 
**									first parameter is the buffer pointer, the 2nd 
**									parameter is the block length.
**
** parameters:			buffer pointer, and the block length
** Returned value:		None
** 
*****************************************************************************/
void SPI_SendRcv( LPC_SPI_TypeDef *SPIx, SLAVE_t slave, uint8_t *tx, uint8_t *rx, uint32_t Length )
{
  uint32_t i = 0;

  if ( Length == 1 ) {
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | *tx;
		SPIx->INTENSET = STAT_TXRDY;
		while(!rxrdy);
		rxrdy = 0;
		*rx = SPIx->RXDAT;
#if STALL_ENABLE
		SPIx->INTENSET = STAT_RXRDY | STAT_CLKSTALL;
#else
		SPIx->INTENSET = STAT_RXRDY;
#endif
#else
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		/* Set frame length to fixed 8 for now. */
		SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | *tx;
		while ( (SPIx->STAT & STAT_RXRDY) == 0 );
		*rx = SPIx->RXDAT;
#endif
		return;
  }

  while ( i < Length ) {
	/* Move only if TXRDY is ready */
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | *tx++;
		}
		else if ( i == Length-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | *tx++;
		}
		else {
			SPIx->TXDAT = *tx++;
		}
		SPIx->INTENSET = STAT_TXRDY;
		while(!rxrdy);
		rxrdy = 0;
		*rx++ = SPIx->RXDAT;
#if STALL_ENABLE
		SPIx->INTENSET = STAT_RXRDY | STAT_CLKSTALL;
#else
		SPIx->INTENSET = STAT_RXRDY;
#endif
#else
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | *tx++;
		}
		else if ( i == Length-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | *tx++;
		}
		else {
			SPIx->TXDAT = *tx++;
		}
		while ( (SPIx->STAT & STAT_RXRDY) == 0 );
		*rx++ = SPIx->RXDAT;
#endif
		i++;
  }
  return; 
}

/*****************************************************************************
** Function name:		SPI_Send
**
** Descriptions:		Send a block of data to the SPI port, the 
**						first parameter is slave select, the second is the buffer pointer, 
**						the 3rd parameter is the block length.
**
** parameters:			slave select, buffer pointer, and the block length
** Returned value:		None
** 
*****************************************************************************/
void SPI_Send( LPC_SPI_TypeDef *SPIx, SLAVE_t slave, uint8_t *tx, uint32_t Length )
{
  uint32_t i = 0;

  if ( Length == 1 ) {
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) 
			| TXDATCTL_RX_IGNORE | TXDATCTL_EOT | *tx;
		SPIx->INTENSET = STAT_TXRDY;
#else
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) 
			| TXDATCTL_RX_IGNORE | TXDATCTL_EOT | *tx;
#endif
		return;
  }

  while ( i < Length ) {
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) 
				| TXDATCTL_RX_IGNORE | *tx++;
		}
		else if ( i == Length-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) 
				| TXDATCTL_RX_IGNORE | TXDATCTL_EOT | *tx++;
		}
		else {
			SPIx->TXDAT = *tx++;
		}
		SPIx->INTENSET = STAT_TXRDY;
#else
		/* Move only if TXRDY is ready */
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) 
				| TXDATCTL_RX_IGNORE | *tx++;
		}
		else if ( i == Length-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) 
				| TXDATCTL_RX_IGNORE | TXDATCTL_EOT | *tx++;
		}
		else {
			SPIx->TXDAT = *tx++;
		}
#endif
		i++;
  }
  return; 
}

/*****************************************************************************
** Function name:		SPI_Receive
** Descriptions:		the module will receive a block of data from 
**						the SPI, the 1st is the slave select, the 2nd parameter 
**						is the rx buffer pointer, the 3rd is the block length.
** parameters:			slave select, buffer pointer, and block length
** Returned value:		None
** 
*****************************************************************************/
void SPI_Receive( LPC_SPI_TypeDef *SPIx, SLAVE_t slave, uint8_t *rx, uint32_t Length )
{
  uint32_t i = 0;

  if ( Length == 1 ) {
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | 0x55;
		SPIx->INTENSET = STAT_TXRDY;
		while(!rxrdy);
		rxrdy = 0;
		*rx = SPIx->RXDAT;
#if STALL_ENABLE
		SPIx->INTENSET = STAT_RXRDY | STAT_CLKSTALL;
#else
		SPIx->INTENSET = STAT_RXRDY;
#endif
#else
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | 0x55;
		/* Read only if RX data is available. */
		while ( (SPIx->STAT & STAT_RXRDY) == 0 );
		*rx = SPIx->RXDAT;
#endif
		return;
  }

  while ( i < Length )
  {
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | 0x55;
		}
		else if ( i == Length-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | 0x55;
		}
		else {
			SPIx->TXDAT = 0x55;
		}
		SPIx->INTENSET = STAT_TXRDY;
		while(!rxrdy);
		rxrdy = 0;
		*rx++ = SPIx->RXDAT;
#if STALL_ENABLE
		SPIx->INTENSET = STAT_RXRDY | STAT_CLKSTALL;
#else
		SPIx->INTENSET = STAT_RXRDY;
#endif
#else
		/* Move only if TXRDY is ready, write dummy 0x55 in order to read. */
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | 0x55;
    }
		else if ( i == Length-1 ) {
			SPIx->TXDATCTL = TXDATCTL_SSELN(slave) | TXDATCTL_FSIZE(MASTER_FRAME_SIZE) | TXDATCTL_EOT | 0x55;
		}
		else {
			SPIx->TXDAT = 0x55;
		}
		/* Read only if RX data is available. */
		while ( (SPIx->STAT & STAT_RXRDY) == 0 );
		*rx++ = SPIx->RXDAT;
#endif
		i++;
  }
  return; 
}

/*****************************************************************************
** Function name:		SPI_SlaveSend
**
** Descriptions:		Send a block of data to the SPI port, the 
**						first parameter is the buffer pointer, the 2nd 
**						parameter is the block length.
**
** parameters:			buffer pointer, and the block length
** Returned value:		None
** 
*****************************************************************************/
void SPI_SlaveSend( LPC_SPI_TypeDef *SPIx, uint8_t *tx, uint32_t Length )
{
  uint32_t i = 0;

  while ( i < Length ) {
#if SPI_INTERRUPT
		while(!txrdy);
		txrdy = 0;
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_FSIZE(SLAVE_FRAME_SIZE) | TXDATCTL_RX_IGNORE | *tx++;
		}
		else {
			SPIx->TXDAT = *tx++;
		}
		SPIx->INTENSET = STAT_TXRDY;
#else
		/* Move only if TXRDY is ready */
		while ( (SPIx->STAT & STAT_TXRDY) == 0 );
		/* Set frame length to fixed 8 for now. */
		if ( i == 0 ) {
			SPIx->TXDATCTL = TXDATCTL_FSIZE(SLAVE_FRAME_SIZE) | TXDATCTL_RX_IGNORE | *tx++;
		}
		else {
			SPIx->TXDAT = *tx++;
		}
#endif
		i++;
  }
  return; 
}

/*****************************************************************************
** Function name:		SPI_SlaveReceive
** Descriptions:		the module will receive a block of data from 
**						the SPI, the 2nd parameter is the block length.
** parameters:			buffer pointer, and block length
** Returned value:		None
** 
*****************************************************************************/
void SPI_SlaveReceive( LPC_SPI_TypeDef *SPIx, uint8_t *rx, uint32_t Length )
{
  uint32_t i = 0;

  /* Set frame length to fixed 8 for now. */
  SPIx->TXCTRL = TXDATCTL_FSIZE(SLAVE_FRAME_SIZE);
  while ( i < Length )
  {
#if SPI_INTERRUPT
		while(!rxrdy);
		rxrdy = 0;
		*rx++ = SPIx->RXDAT;
#if STALL_ENABLE
		SPIx->INTENSET = STAT_RXRDY | STAT_CLKSTALL;
#else
		SPIx->INTENSET = STAT_RXRDY;
#endif
#else
		/* Read only if RX data is available. */
		while ( (SPIx->STAT & STAT_RXRDY) == 0 );
		*rx++ = SPIx->RXDAT;
#endif
		i++;
  }
  return; 
}

/******************************************************************************
**                            End Of File
******************************************************************************/
