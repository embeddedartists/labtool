/**********************************************************************
* $Id$		lpc8xx_uart.h		2012-10-31
*//**
* @file		lpc8xx_uart.h
* @brief	Contains all macro definitions and function prototypes
* 			support for UART firmware library on lpc8xx
* @version	1.0
* @date		02. June. 2012
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

#ifndef __lpc8xx_UART_H
#define __lpc8xx_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Public Macros -------------------------------------------------------------- */
/** @defgroup UART_Public_Macros  UART Public Macros
 * @{
 */

#define LOOPBACK_TEST			0		/* Loopback uses polling only */
#define TX_INTERRUPT    	1	/* 0 if TX uses polling, 1 interrupt driven. */
#define ERROR_INTERRUPT 	0
#define TX_DISABLE				0
#define FLOWCTRL_ENABLE 	0	/* Enable the flow control INTERRUPT to get the status 
														update. It's meant to be for external H/W flow control. 
														For lookback and flow control, if it's set and loopback
														test flag is turned on. It's the internal flow control. 
														external CTS needs to be set to 0, and internal CTS needs
														to be set to 1. */
#define ADDR_DETECT_EN    0   /* Without two boards connected, the simple test is,
                                on the PC side, set to 8 bits, even parity. on the board
																side, it's 9 bit, ADDR_DET is enabled. */
#define SLAVE_MODE		0
#define MASTER_MODE		1

#define HALF_DUPLEX		1

/* Configuration register bit def. except data bit, parity, stop bits setting. */
#define UART_EN       (0x01<<0)
#define DATA_LENG_7   (0x00<<2)
#define DATA_LENG_8	  (0x01<<2)
#define DATA_LENG_9	  (0x02<<2)
#define PARITY_NONE   (0x00<<4)
#define PARITY_NC     (0x01<<4)
#define PARITY_EVEN   (0x02<<4)
#define PARITY_ODD    (0x03<<4)
#define STOP_BIT_1    (0x00<<6)
#define STOP_BIT_2	  (0x01<<6)
#define MODE_32K      (0x01<<7)
#define EXT_CTS_EN    (0x01<<9)
#define INT_CTS_EN    (0x01<<10)
#define SYNC_EN       (0x01<<11)
#define CLK_POL       (0x01<<12)
#define SYNC_MS       (0x01<<14)
#define LOOPBACK      (0x01<<15)

/* UART Control register */
#define TXBRK_EN      (0x01<<1)
#define ADDR_DET      (0x01<<2)
#define TXDIS         (0x01<<6)
#define CC            (0x01<<8)
#define CCCLR         (0x01<<9) 

/* UART status register bit definition. */
#define RXRDY         (0x01<<0)
#define RXIDLE        (0x01<<1)
#define TXRDY         (0x01<<2)
#define TXIDLE        (0x01<<3)
#define CTS           (0x01<<4)
#define CTS_DELTA     (0x01<<5)
#define TXINT_DIS     (0x01<<6)

#define OVRN_ERR      (0x01<<8)
#define RXBRK         (0x01<<10)
#define DELTA_RXBRK   (0x01<<11)
#define START_DETECT  (0x01<<12)
#define FRM_ERR       (0x01<<13)
#define PAR_ERR       (0x01<<14)
#define RXNOISE       (0x01<<15)
/* Debugging related also included in the ERROR_MASK */
#define UART_ERROR_MASK	(OVRN_ERR|RXBRK|DELTA_RXBRK|START_DETECT|FRM_ERR|PAR_ERR|RXNOISE)

#define BUFSIZE       0x20

void UART_Handler( LPC_USART_TypeDef *UARTx );
void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);
void FlowControlInit( LPC_USART_TypeDef *UARTx );
void UARTInit(LPC_USART_TypeDef *UARTx, uint32_t Baudrate);

void UARTSend(LPC_USART_TypeDef *UARTx, uint8_t *BufferPtr, uint32_t Length);

void USARTInit(LPC_USART_TypeDef *UARTx, uint32_t Baudrate, uint32_t mode);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __lpc8xx_UART_H */

/**
 * @}
 */

