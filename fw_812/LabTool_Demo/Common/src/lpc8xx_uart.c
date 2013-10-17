/**********************************************************************
* $Id$		lpc8xx_uart.c		2012-10-31
*//**
* @file		lpc8xx_uart.c
* @brief	Contains all functions support for UART firmware library on lpc8xx
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
* documentation is hereby granted, under NXP Semiconductors�
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup UART
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "LPC8xx.h"
#include <stdint.h>
#include "lpc8xx_nmi.h"
#include "lpc8xx_uart.h"

/* All these variables, mostly used for debugging only, are shared by all UARTs 
now for easy debugging. So only one UART can run at a time. They should be rewritten
when we release the software to the public. */
volatile uint32_t UARTStatus;
volatile uint8_t  UARTTxEmpty = 1;
volatile uint8_t  UARTTxBuffer[BUFSIZE];
volatile uint8_t  UARTRxBuffer[BUFSIZE];
volatile uint32_t UARTTxCount = 0;
volatile uint32_t UARTRxCount = 0;
volatile uint32_t RxErrorCount = 0;
volatile uint32_t TxErrorCount = 0;
volatile uint32_t UARTRxRdyCount = 0;
volatile uint32_t ParityErrorCount = 0;
volatile uint32_t UARTBlockReceived = 0;
volatile uint32_t UARTBlockTransmitted = 0;
volatile uint32_t AddrDetected = 0;

volatile uint32_t UARTInterruptCount = 0;

#if FLOWCTRL_ENABLE
volatile uint32_t DeltaCTSCount = 0;
#endif

#if TX_DISABLE
volatile uint32_t TxDisableCount = 0;
#endif

#if ERROR_INTERRUPT
volatile uint32_t RxBreakCount = 0;
volatile uint32_t DeltaRxBreakCount = 0;
volatile uint32_t StartDetectCount = 0;
volatile uint32_t FrameErrorCount = 0;
volatile uint32_t OverrunErrorCount = 0;
volatile uint32_t RxNoiseCount = 0;
#endif

/* Right now, the debugging info. and variables are shared by all UARTs.
Only one UART can run at a time. Will be fixed later. */

/*****************************************************************************
** Function name:		UART_Handler
**
** Descriptions:		Generic UART interrupt handler for UART0/1/2
**
** parameters:			LPC_UART_TypeDef *UARTx
** Returned value:	None
** 
*****************************************************************************/
void UART_Handler( LPC_USART_TypeDef *UARTx )
{
	uint32_t Status = 0, regVal=regVal;

  UARTInterruptCount++;
  Status = UARTx->STAT;

#if FLOWCTRL_ENABLE
	if ( Status & CTS_DELTA )
  {
		UARTx->STAT = CTS_DELTA;	/* clear Delta CTS status */ 
		DeltaCTSCount++;
  }
#endif

#if ERROR_INTERRUPT
	if ( Status & UART_ERROR_MASK )
	{
		if ( Status & RXBRK )
		{
			RxBreakCount++;
		}
		if ( Status & DELTA_RXBRK )
		{
			UARTx->STAT = DELTA_RXBRK;	/* clear Delta Rx Break status */ 
			DeltaRxBreakCount++;
		}
		if ( Status & START_DETECT )
		{
			UARTx->STAT = START_DETECT;	/* clear Start Detect status */ 
			StartDetectCount++;
		}
		if ( Status & FRM_ERR )
		{
			UARTx->STAT = FRM_ERR;	/* clear frame error status */ 
			FrameErrorCount++;
		}
		if ( Status & OVRN_ERR )
		{
			UARTx->STAT = OVRN_ERR;	/* clear overrun status */ 
			OverrunErrorCount++;
		}
		if ( Status & RXNOISE )
		{
			UARTx->STAT = RXNOISE;	/* clear Rx Noise status */
			RxNoiseCount++;
		}
		if ( Status & PAR_ERR )
		{
			UARTx->STAT = PAR_ERR;	/* clear parity status */
			ParityErrorCount++;
		}
  }
#endif
	
#if TX_DISABLE
  if (Status & TXINT_DIS)
  {
		TxDisableCount++;
		UARTx->INTENCLR = TXINT_DIS; 
  }
#endif

  if (Status & RXRDY)	/* RX Ready */
  {
		/* Receive Data Available */
#if 1
		if ( ((regVal=UARTx->RXDATA) & 0x100) && (UARTx->CTRL & ADDR_DET) )
		{
			AddrDetected = regVal & 0x1FF;
		}
		UARTRxBuffer[UARTRxCount++] = regVal;
		if (UARTRxCount == BUFSIZE)
		{
			UARTRxCount = 0;		/* buffer overflow */
			UARTBlockReceived = 1;
		}
#if HALF_DUPLEX
		UARTx->CTRL |= ( CC | CCCLR );
#endif
#else
		if ( !((regVal=UARTx->RXDATA_STAT) & 0xF000) )
		{
			if ( (regVal & 0x100) && (UARTx->CTRL & ADDR_DET) )
			{
				AddrDetected = regVal & 0x1FF;
			}
			UARTRxBuffer[UARTRxCount++] = (uint8_t)(regVal & 0xFF);
			if (UARTRxCount == BUFSIZE)
			{
				UARTRxCount = 0;		/* buffer overflow */
				UARTBlockReceived = 1;
			}
		}
		else
		{
			RxErrorCount++;
		}
#endif
		UARTRxRdyCount++;
  }
  if (Status & TXRDY)	/* TX Ready */
  {
		UARTTxEmpty = 1;
		UARTx->INTENCLR = TXRDY;
  }
	return;
}

/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART0_IRQHandler(void)
{
	UART_Handler(LPC_USART0);
  return;
}

/*****************************************************************************
** Function name:		UART1_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART1_IRQHandler(void)
{
	UART_Handler(LPC_USART1);
  return;
}

/*****************************************************************************
** Function name:		UART2_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART2_IRQHandler(void)
{
	UART_Handler(LPC_USART2);
  return;
}

#if FLOWCTRL_ENABLE
/*****************************************************************************
** Function name:		FlowControlInit
**
** Descriptions:		Initialize UART as a H/W external flow control port, 
**						Need to be called after UARTInit();
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void FlowControlInit( LPC_USART_TypeDef *UARTx )
{			
  UARTx->CFG |= EXT_CTS_EN;   /* Turn on external CTS */
  UARTx->CFG &= ~INT_CTS_EN;  /* Turn off internal CTS. */
  UARTx->INTENSET = CTS_DELTA;
  return;
}
#endif

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART port, setup pin select,
**						clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
** 
*****************************************************************************/
void UARTClock_Init( LPC_USART_TypeDef *UARTx )
{
  LPC_SYSCON->UARTCLKDIV = 1;     /* divided by 1 */
	
	if (UARTx == LPC_USART0)
	{
		NVIC_DisableIRQ(UART0_IRQn);
		/* Enable UART clock */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<14);
		/* Peripheral reset control to UART, a "1" bring it out of reset. */
		LPC_SYSCON->PRESETCTRL &= ~(0x1<<3);
		LPC_SYSCON->PRESETCTRL |= (0x1<<3);
	}
	if (UARTx == LPC_USART1)
	{
		NVIC_DisableIRQ(UART1_IRQn);
		/* Enable UART clock */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<15);
		/* Peripheral reset control to UART, a "1" bring it out of reset. */
		LPC_SYSCON->PRESETCTRL &= ~(0x1<<4);
		LPC_SYSCON->PRESETCTRL |= (0x1<<4);
	}
	if (UARTx == LPC_USART2)
	{
		NVIC_DisableIRQ(UART2_IRQn);
		/* Enable UART clock */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);
		/* Peripheral reset control to UART, a "1" bring it out of reset. */
		LPC_SYSCON->PRESETCTRL &= ~(0x1<<5);
		LPC_SYSCON->PRESETCTRL |= (0x1<<5);
	}
	return;	
}

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART port, setup pin select,
**						clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
** 
*****************************************************************************/
void UARTInit(LPC_USART_TypeDef *UARTx, uint32_t baudrate)
{
	uint32_t UARTSysClk;
		
  UARTTxEmpty = 1;
	
	UARTClock_Init( UARTx );
	
	UARTSysClk = SystemCoreClock/LPC_SYSCON->UARTCLKDIV;
	
  UARTx->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_1; /* 8 bits, no Parity, 1 Stop bit */
//  UARTx->CFG = DATA_LENG_7|PARITY_NONE|STOP_BIT_1; /* 7 bits, no Parity, 1 Stop bit */
//  UARTx->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_2; /* 8 bits, no Parity, 2 Stop bit */
//  UARTx->CFG = DATA_LENG_8|PARITY_EVEN|STOP_BIT_1; /* 8 bits, even Parity, 1 Stop bit */
//  UARTx->CFG = DATA_LENG_8|PARITY_ODD|STOP_BIT_1; /* 8 bits, odd Parity, 1 Stop bit */

#if ADDR_DETECT_EN
	/* Without two boards connected, the simpliest test is, on the PC side, set to 8 bits, 
	even parity. on the board side, set to 9 bit, ADDR_DET is set. When data is received
	and 9th bit is one, if ADDR_DET is set in CTRL, grab the data as address detected. */
  /* Overwrite above UART CFG register. */
  UARTx->CFG = DATA_LENG_9|PARITY_NONE|STOP_BIT_1; /* 9 bits, Parity doesn't apply, 1 Stop bit */
  UARTx->CTRL = ADDR_DET;
#endif

	if ( !LPC_SYSCON->UARTCLKDIV )
	{
		/* UART clock divider for FDR is disabled. need to know why? */
		while ( 1 );
	}
	else
	{
		UARTx->BRG = UARTSysClk/16/baudrate-1;	/* baud rate */
		/*
			Integer divider:
			BRG = UARTSysClk/(Baudrate * 16) - 1
			Frational divider:
			FRG = ((UARTSysClk / (Baudrate * 16 * (BRG + 1))) - 1) where FRG = (LPC_SYSCON->UARTFRDADD + 1) / (LPC_SYSCON->UARTFRDSUB + 1)
		*/
		/*	(1) The easist way is set SUB value to 256, -1 encoded, thus SUB register is 0xFF. 
				(2) In ADD register value, depending on the value of UartSysClk, baudrate, BRG register value, and SUB register value, be careful 
				about the order of multiplyer and divider and make sure any multiplyer doesn't exceed 32-bit boundary and any divider doesn't get 
				down below one(integer 0).
				(3) ADD should be always less than SUB. */

		LPC_SYSCON->UARTFRGDIV = 0xFF;
		LPC_SYSCON->UARTFRGMULT = (((UARTSysClk / 16) * (LPC_SYSCON->UARTFRGDIV + 1)) / (baudrate * (UARTx->BRG + 1))) - (LPC_SYSCON->UARTFRGDIV + 1);
	}
	
#if FLOWCTRL_ENABLE
  FlowControlInit(UARTx);
#endif
		
  UARTx->STAT = CTS_DELTA | DELTA_RXBRK;		/* Clear all status bits. */
  /* Enable the UART Interrupt. */
	if (UARTx == LPC_USART0) {
#if NMI_ENABLED
		NVIC_DisableIRQ( UART0_IRQn );
		NMI_Init( UART0_IRQn );
#else
		NVIC_EnableIRQ(UART0_IRQn);
#endif
	}
	else if (UARTx == LPC_USART1) {
#if NMI_ENABLED
		NVIC_DisableIRQ( UART1_IRQn );
		NMI_Init( UART1_IRQn );
#else
		NVIC_EnableIRQ(UART1_IRQn);
#endif
	}
	else if (UARTx == LPC_USART2) {
#if NMI_ENABLED
		NVIC_DisableIRQ( UART2_IRQn );
		NMI_Init( UART2_IRQn );
#else
		NVIC_EnableIRQ(UART2_IRQn);
#endif
	}
	
#if TX_INTERRUPT
  UARTx->INTENSET = RXRDY | TXRDY | DELTA_RXBRK;	/* Enable UART interrupt */
#else
  UARTx->INTENSET = RXRDY | DELTA_RXBRK;
  UARTx->INTENCLR = TXRDY;
#endif

#if ERROR_INTERRUPT
  UARTx->INTENSET = (FRM_ERR|OVRN_ERR|PAR_ERR|RXNOISE);
#endif

#if TX_DISABLE
  UARTx->CTRL |= TXDIS;
  UARTx->INTENSET = TXINT_DIS;
#endif

  UARTx->CFG |= UART_EN; 
  return;
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**						on the data length
**
** parameters:			buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UARTSend(LPC_USART_TypeDef *UARTx, uint8_t *BufferPtr, uint32_t Length)
{
#if ADDR_DETECT_EN
  uint32_t flag = 0;
#endif

  while ( Length != 0 )
  {
#if !TX_INTERRUPT
	  while ( !(UARTx->STAT & TXRDY) );
#if ADDR_DETECT_EN
	  if ( flag == 0 )
	  {
			/* In the first byte, set 9th bit to 1 for address detection. */
			UARTx->TXDATA = *BufferPtr|0x100;
			flag = 1;
	  }
	  else
	  {
			UARTx->TXDATA = *BufferPtr;
	  }
#else
	  UARTx->TXDATA = *BufferPtr;
#endif
#else
	  /* Below flag is set inside the interrupt handler when TXRDY is set. */
    while ( !(UARTTxEmpty & 0x01) );
#if ADDR_DETECT_EN
	  if ( flag == 0 )
	  {
			/* In the first byte, set 9th bit to 1 for address detection. */
			UARTx->TXDATA = *BufferPtr|0x100;
			flag = 1;
	  }
	  else
	  {
			UARTx->TXDATA = *BufferPtr;
	  }
#else
	  UARTx->TXDATA = *BufferPtr;
#endif
    UARTTxEmpty = 0;	/* reset the flag after data is written to the TXDATA */
	  UARTx->INTENSET = TXRDY;
#endif
    BufferPtr++;
    Length--;
  }
  return;
}

/*****************************************************************************
** Function name:		USARTInit
**
** Descriptions:		Initialize USART port, setup pin select,
**						clock, parity, stop bits, FIFO, etc.
**
** parameters:			mode: master(1) or slave(0), UART baudrate
** Returned value:		None
** 
*****************************************************************************/
void USARTInit(LPC_USART_TypeDef *UARTx, uint32_t baudrate, uint32_t mode)
{
  UARTTxEmpty = 1;
  NVIC_DisableIRQ(UART0_IRQn);

	UARTClock_Init( UARTx );
	
  UARTx->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_1; /* 8 bits, no Parity, 1 Stop bit */
//  UARTx->CFG = DATA_LENG_7|PARITY_NONE|STOP_BIT_1; /* 7 bits, no Parity, 1 Stop bit */
//  UARTx->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_2; /* 8 bits, no Parity, 2 Stop bit */
//  UARTx->CFG = DATA_LENG_8|PARITY_EVEN|STOP_BIT_1; /* 8 bits, even Parity, 1 Stop bit */
//  UARTx->CFG = DATA_LENG_8|PARITY_ODD|STOP_BIT_1; /* 8 bits, odd Parity, 1 Stop bit */

  UARTx->BRG = (SystemCoreClock/LPC_SYSCON->UARTCLKDIV)/baudrate-1;	/*baud rate */
  UARTx->STAT = CTS_DELTA | DELTA_RXBRK;		/* Clear all status bits. */		

  UARTx->CFG |= SYNC_EN;
  if ( mode != 0 )
  {
		UARTx->CFG |= SYNC_MS;
  }
  else
  {
		UARTx->CFG &= ~SYNC_MS;
  }    

#if HALF_DUPLEX
	UARTx->CTRL |= ( CC | CCCLR );
#else
#if 0
  UARTx->CTRL |= CC;
#endif
#if 0
  UARTx->CTRL |= CCCLR;
#endif
#endif
#if 0
  UARTx->CFG |= CLK_POL;
#endif

  /* Enable the UART Interrupt */
  NVIC_EnableIRQ(UART0_IRQn);

#if TX_INTERRUPT
  UARTx->INTENSET = RXRDY | TXRDY;	/* Enable UART interrupt */
#else
  UARTx->INTENSET = RXRDY;
  UARTx->INTENCLR = TXRDY;
#endif

#if ERROR_INTERRUPT
  UARTx->INTENSET = (FRM_ERR|OVRN_ERR|PAR_ERR|RXNOISE);
#endif

  UARTx->CFG |= UART_EN;	/* Deassert RESET, UART is in operation. */ 
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
