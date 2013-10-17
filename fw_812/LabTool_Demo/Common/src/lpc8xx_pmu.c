/****************************************************************************
 *   $Id:: pmu.c 3635 2012-10-31 00:31:46Z usb00423                         $
 *   Project: NXP LPC8xx PMU example
 *
 *   Description:
 *     This file contains PMU code example which include PMU 
 *     initialization, PMU interrupt handler, and APIs for PMU
 *     reading.
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
#include <stdint.h>
#include "lpc8xx_gpio.h"
#include "lpc8xx_pmu.h"
#include "lpc8xx_wkt.h"

/*****************************************************************************
** Function name:		PMU_Init
**
** Descriptions:		Initialize PMU and setup wakeup source.
**						For Sleep and deepsleep, any of the I/O pins can be 
**						used as the wakeup source.
**						For Deep Powerdown, only pin P1.4 can be used as 
**						wakeup source from deep powerdown. 
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void PMU_Init( void )
{
  /* Enable all clocks, even those turned off at power up. */
  LPC_SYSCON->PDRUNCFG &= ~WDT_OSC_PD;

#if __WKT
  /* alarm/wake timer as wakeup source */
  LPC_SYSCON->STARTERP1 = 0x1<<15;
#else
#if 0
  /* PININT0 source */
  GPIOSetPinInterrupt( CHANNEL0, PORT0, 2, 0, 0 );
  /* PININT0 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<0;
#endif

#if 0
  /* PININT1 source */
  GPIOSetPinInterrupt( CHANNEL1, PORT0, 3, 0, 0 );
  /* PININT1 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<1;
#endif

#if 0
  /* PININT2 source */
  GPIOSetPinInterrupt( CHANNEL2, PORT0, 16, 0, 0 );
  /* PININT2 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<2;
#endif

#if 0
  /* PININT3 source */
  GPIOSetPinInterrupt( CHANNEL3, PORT0, 17, 0, 0 );
  /* PININT3 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<3;
#endif

#if 0
  /* PININT4 source */
  GPIOSetPinInterrupt( CHANNEL4, PORT0, 8, 0, 0 );
  /* PININT4 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<4;
#endif

#if 0
  /* PININT5 source */
  GPIOSetPinInterrupt( CHANNEL5, PORT0, 9, 0, 0 );
  /* PININT5 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<5;
#endif

#if 0
  /* PININT6 source */
  GPIOSetPinInterrupt( CHANNEL6, PORT0, 14, 0, 0 );
  /* PININT6 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<6;
#endif

#if 1
  /* PININT7 source */
  GPIOSetPinInterrupt( CHANNEL7, PORT0, 15, 0, 1 );
  /* PININT7 as wakeup source */
  LPC_SYSCON->STARTERP0 = 0x1<<7;
#endif
#endif

  return;
}

/*****************************************************************************
** Function name:		PMU_Sleep
**
** Descriptions:		Put some of the peripheral in sleep mode.
**
** parameters:			SleepMode: 2 is power down, 1 is deep sleep, 0 is sleep, 
**						Sleep peripheral module(s)
** Returned value:		None
** 
*****************************************************************************/
void PMU_Sleep( uint32_t SleepMode, uint32_t SleepCtrl )
{
  LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;
  LPC_SYSCON->PDSLEEPCFG = SleepCtrl;
  /* If normal sleep, not deep sleep, don't do anything to SCR reg. */
  switch ( SleepMode )
  {
	case MCU_POWER_DOWN:
	  SCB->SCR |= NVIC_LP_SLEEPDEEP;
#if __WKT
	  LPC_PMU->DPDCTRL |= (0x1 << 2);
	  init_wkt(1, 10000 * 10);
#endif	
#if 1
	  LPC_PMU->PCON = 0x2;
#else
	  LPC_SYSCON->PDSLEEPCFG &= 0xFDFF;
#endif	
	  break;
	case MCU_DEEP_SLEEP:
	  SCB->SCR |= NVIC_LP_SLEEPDEEP;
#if __WKT
	  LPC_PMU->DPDCTRL |= (0x1 << 2);
	  init_wkt(1, 10000 * 10);
#endif	
#if 1
	  LPC_PMU->PCON = 0x1;
#else
	  LPC_SYSCON->PDSLEEPCFG &= 0xEFF9;
#endif	
	  break;
	case MCU_SLEEP:
	default:
	  break;
  }
  __WFI();
  return;
}

/*****************************************************************************
** Function name:		PMU_DeepPowerDown
**
** Descriptions:		Some of the content should not be touched 
**						during the deep power down to wakeup process.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void PMU_DeepPowerDown( void )
{
  uint32_t regVal;

  if ( (LPC_PMU->PCON & (0x1<<11)) != 0x0 )
  {
    /* Check deep power down bits. If deep power down mode is entered, 
    clear the PCON bits. */
    regVal = LPC_PMU->PCON;
    regVal |= (0x1<<11);
    LPC_PMU->PCON = regVal;

    if ( (LPC_PMU->GPREG0 != 0x12345678)||(LPC_PMU->GPREG1 != 0x87654321)
         ||(LPC_PMU->GPREG2 != 0x56781234)||(LPC_PMU->GPREG3 != 0x43218765) )
    {
      while (1);
    }
  }
  else
  {
	/* If in neither sleep nor deep power mode, enter deep power
	down mode now. */
    LPC_PMU->GPREG0 = 0x12345678;
    LPC_PMU->GPREG1 = 0x87654321;
    LPC_PMU->GPREG2 = 0x56781234;
    LPC_PMU->GPREG3 = 0x43218765;
    SCB->SCR |= NVIC_LP_SLEEPDEEP;
#if __WKT
	  LPC_PMU->DPDCTRL |= (0x3 << 2);
	  init_wkt(1, 10000 * 10);
#endif	
    LPC_PMU->PCON = 0x3;
    __WFI();
  }
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
