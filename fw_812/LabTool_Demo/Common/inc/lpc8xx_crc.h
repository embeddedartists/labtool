/****************************************************************************
 *   $Id:: crc.h 5786 2012-10-31 01:11:32Z usb00423                         $
 *   Project: NXP LPC8xx CRC example
 *
 *   Description:
 *     This file contains CRC code header definition.
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
#ifndef __CRC_H__
#define __CRC_H__

#define CRC_32_POLYNOMIAL		0
#define CRC_16_POLYNOMIAL		1
#define CRC_CCITT_POLYNOMIAL	2

extern void CRC_SetMode( uint32_t crc_poly, uint32_t bit_rvs_wr,
						 uint32_t cmpl_wr, uint32_t bit_rvs_sum, uint32_t cmpl_sum );
extern void CRC_WriteData( uint8_t * data, uint8_t length );
extern uint32_t CRC_GetSum( void );

#endif  /* __CRC_H__ */
/*****************************************************************************
**                            End Of File
******************************************************************************/

