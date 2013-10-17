/*****************************************************************************
 *   comp.h:  Header file for NXP LPC8xx Family Microprocessors
 *
 *   Copyright(C) 2012, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2012.10.31  ver 1.00    Prelimnary version, first Release
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
******************************************************************************/
#ifndef __COMP_H__
#define __COMP_H__


#define COMP_VP	0
#define COMP_VM	1

#define COMPSA			(0x1 << 6)
#define EDGECLR			(0x1 << 20)
#define COMPSTAT		(0x1 << 21)
#define COMPEDGE		(0x1 << 23)

extern void CMP_IRQHandler (void);
extern void COMP_IOConfig( void );
extern void COMP_Init( void );
extern void COMP_SelectInput( uint32_t comp_channel, uint32_t input );
extern void COMP_SetOutput( uint32_t sync );
extern void COMP_SetInterrupt( uint32_t single, uint32_t event );
extern void COMP_SetHysteresis( uint32_t level );
extern uint32_t COMP_GetOutputStatus( void );

#endif  /* __COMP_H__ */
/*****************************************************************************
**                            End Of File
******************************************************************************/

