/**********************************************************************
* $Id$		lpc8xx_spi.h		2012-10-31
*//**
* @file		lpc8xx_spi.h
* @brief	Contains all macro definitions and function prototypes
* 			support for SPI firmware library on lpc18ax
* @version	1.0
* @date		31. October. 2012
* @author	NXP MCU SW Application Team
*
* Copyright(C) 2011, NXP Semiconductor
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
/** @defgroup RGU RGU (Reset Generation Unit)
 * @ingroup LPC800CMSIS_FwLib_Drivers
 * @{
 */

#ifndef lpc8xx_SPI_H_
#define lpc8xx_SPI_H_

/* Includes ------------------------------------------------------------------- */
#include "lpc8xx.h"


#ifdef __cplusplus
extern "C"
{
#endif
/* Public Types --------------------------------------------------------------- */
/** @defgroup RGU_Public_Types RGU Public Types
 * @{
 */

#define SPI_INTERRUPT	1		/* For TXRDY and RXRDY test. */
#define SPI_TX_RX			0		/* For master mode, it's either test a EEPROM or interboard
													master/slave communication test. set SPI_TX_RX to 0(default) 
													to test EEPROM. */
#define SPI_LOOPBACK_TEST	0		/* Polling only in loopback test. */
#define STALL_ENABLE	0
	
/* SPI read and write buffer size */
#define SPI_BUFSIZE     0x10

#define MASTER_FRAME_SIZE		(8-1)
#define SLAVE_FRAME_SIZE		(8-1)		
	
#define SFLASH_INDEX    2
#define MAX_TIMEOUT     0xFFF

#define CFG_ENABLE			(1 << 0)
#define CFG_MASTER			(1 << 2)
#define CFG_SLAVE				(0 << 2)
#define CFG_LSBF				(1 << 3)
#define CFG_CPHA				(1 << 4)
#define CFG_CPOL				(1 << 5)
#define CFG_MOSIDRV			(1 << 6)
#define CFG_LOOPBACK		(1 << 7)
#define CFG_SPOL(s)			(1 << (8 + s))

#define DLY_PREDELAY(d)		((d) << 0)
#define DLY_POSTDELAY(d)	((d) << 4)
#define DLY_FRAMEDELAY(d)	((d) << 8)
#define DLY_INTERDELAY(d)	((d) << 12)

#define STAT_RXRDY 				(1 << 0)
#define STAT_TXRDY 				(1 << 1)
#define STAT_RXOVERRUN 		(1 << 2)
#define STAT_TXUNDERRUN 	(1 << 3)
#define STAT_SELNASSERT 	(1 << 4)
#define STAT_SELNDEASSERT (1 << 5)
#define STAT_CLKSTALL 		(1 << 6)
#define STAT_EOF     			(1 << 7)
#define STAT_ERROR_MASK		(STAT_RXOVERRUN|STAT_TXUNDERRUN|STAT_SELNASSERT|STAT_SELNDEASSERT|STAT_CLKSTALL)

typedef enum {
	SLAVE0 = ((~(1 << 0)) & 0xf),
	SLAVE1 = ((~(1 << 1)) & 0xf),
	SLAVE2 = ((~(1 << 2)) & 0xf),
	SLAVE3 = ((~(1 << 3)) & 0xf)
} SLAVE_t;

#define TXDATCTL_SSELN(s) 	(s << 16)
#define TXDATCTL_EOT 				(1 << 20)
#define TXDATCTL_EOF 				(1 << 21)
#define TXDATCTL_RX_IGNORE	(1 << 22)
#define TXDATCTL_FSIZE(s)		((s) << 24)

#define RXDAT_SOT						(1 << 20)

/* Below is application specific. */
/* ATMEL SEEPROM command set */
#define WREN            0x06		/* MSB A8 is set to 0, simplifying test */
#define WRDI            0x04
#define RDSR            0x05
#define WRSR            0x01
#define READ            0x03
#define WRITE           0x02

/* ATMEL 25DF041 extra commands */
#define BLOCK_ERASE			0x20
#define CHIP_ERASE			0x60
#define PROTECT_SEC			0x36
#define UNPROTECT_SEC		0x39

/* RDSR status bit definition */
#define RDSR_RDY        (0x01<<0)
#define RDSR_WEN        (0x01<<1)
#define RDSR_EPE        (0x01<<5)

void SPI_Handler(LPC_SPI_TypeDef *SPIx);
void SPI0_IRQHandler(void);
void SPI1_IRQHandler(void);
extern void SPI_Init( LPC_SPI_TypeDef *SPIx, uint32_t div, uint32_t cfg, uint32_t dly );
extern void SPI_SendRcv( LPC_SPI_TypeDef *SPIx, SLAVE_t slave, uint8_t *tx, uint8_t *rx, uint32_t Length );
extern void SPI_Send( LPC_SPI_TypeDef *SPIx, SLAVE_t slave, uint8_t *tx, uint32_t Length );
extern void SPI_Receive( LPC_SPI_TypeDef *SPIx, SLAVE_t slave, uint8_t *rx, uint32_t Length );
extern void SPI_SlaveSend( LPC_SPI_TypeDef *SPIx, uint8_t *tx, uint32_t Length );
extern void SPI_SlaveReceive( LPC_SPI_TypeDef *SPIx, uint8_t *rx, uint32_t Length );

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* lpc8xx_SPI_H_ */

/**
 * @}
 */
