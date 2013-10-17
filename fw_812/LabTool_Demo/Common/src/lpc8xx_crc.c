/****************************************************************************
 *   $Id:: crc.c 5786 2012-10-31 01:11:32Z usb00423                         $
 *   Project: NXP LPC8xx CRC example
 *
 *   Description:
 *     This file contains CRC code example which include CRC APIs.
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
#include <stdint.h>
#include "lpc8xx_crc.h"

/*****************************************************************************
** Function name:		CRC_SetMode
**
** Descriptions:		Set polynomial, bit order reverse, 1's complement, etc.
**
** parameters:			crc_poly, bit_rvs_wr, cmpl_wr, bit_rvs_sum, cmpl_sum
** Returned value:		None
** 
*****************************************************************************/
void CRC_SetMode( uint32_t crc_poly, uint32_t bit_rvs_wr,
				  uint32_t cmpl_wr, uint32_t bit_rvs_sum, uint32_t cmpl_sum )
{
  switch ( crc_poly )
  {
	case CRC_32_POLYNOMIAL:
	  LPC_CRC->MODE &= ~0x3;
	  LPC_CRC->MODE |= 0x2;
	break;
 	case CRC_16_POLYNOMIAL:
	  LPC_CRC->MODE &= ~0x3;
	  LPC_CRC->MODE |= 0x1;
	break;
	case CRC_CCITT_POLYNOMIAL:
	  LPC_CRC->MODE &= ~0x3;
	break;
	default:
	  LPC_CRC->MODE &= ~0x3;
	  LPC_CRC->MODE |= 0x2;
	break;
  }

  if ( bit_rvs_wr == 0 )
  {
		LPC_CRC->MODE &= ~0x4;
  }
  else
  {
		LPC_CRC->MODE |= 0x4;
  }

  if ( cmpl_wr == 0 )
  {
		LPC_CRC->MODE &= ~0x8;
  }
  else
  {
		LPC_CRC->MODE |= 0x8;
  }

  if ( bit_rvs_sum == 0 )
  {
		LPC_CRC->MODE &= ~0x10;
  }
  else
  {
		LPC_CRC->MODE |= 0x10;
  }

  if ( cmpl_sum == 0 )
  {
		LPC_CRC->MODE &= ~0x20;
  }
  else
  {
		LPC_CRC->MODE |= 0x20;
  }
  return;
}

/*****************************************************************************
** Function name:		CRC_WriteData
**
** Descriptions:		Write data
**
** parameters:			data, length
** Returned value:		None
** 
*****************************************************************************/
void CRC_WriteData( uint8_t * data, uint8_t length )
{
  uint16_t * data_word = (uint16_t *)data;
  uint32_t * data_dword = (uint32_t *)data;

  switch ( length )
  {
	case 4:
	  LPC_CRC->WR_DATA_DWORD = *data_dword;
	break;
 	case 2:
	  LPC_CRC->WR_DATA_WORD = *data_word;
	break;
	case 1:
	  LPC_CRC->WR_DATA_BYTE = *data;
	break;
	default:
	break;
  }
  return;
}

/*****************************************************************************
** Function name:		CRC_GetSum
**
** Descriptions:		Get sum
**
** parameters:			None
** Returned value:		Sum
** 
*****************************************************************************/
uint32_t CRC_GetSum( void )
{
  uint32_t regVal;
  regVal = LPC_CRC->SUM;
  return regVal;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

