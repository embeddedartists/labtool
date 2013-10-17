/****************************************************************************
 *   $Id:: lpc8xx_gpio.c 5137 2012-10-31 00:15:18Z nxp28433                        $
 *   Project: NXP LPC8xx GPIO example
 *
 *   Description:
 *     This file contains GPIO code example which include GPIO 
 *     initialization, GPIO interrupt handler, and related APIs for 
 *     GPIO access.
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
#include "lpc8xx_gpio.h"

volatile uint32_t flex_int_counter[INT_CHANNEL_NUM] = {0,0,0,0,0,0,0,0};
volatile uint32_t flex_int_level_counter[INT_CHANNEL_NUM] = {0,0,0,0,0,0,0,0};
volatile uint32_t flex_int_rising_edge_counter[INT_CHANNEL_NUM] = {0,0,0,0,0,0,0,0};
volatile uint32_t flex_int_falling_edge_counter[INT_CHANNEL_NUM] = {0,0,0,0,0,0,0,0};

volatile uint32_t gint0_counter = 0;
volatile uint32_t gint1_counter = 0;
volatile uint32_t gint0_level_counter = 0;
volatile uint32_t gint0_edge_counter = 0;
volatile uint32_t gint1_level_counter = 0;
volatile uint32_t gint1_edge_counter = 0;

/*****************************************************************************
** Function name:		PININT_Handler
**
** Descriptions:		General call made by the handlers.
**
** parameters:			IRQ number(0~7)
** 						
** Returned value:	None
** 
*****************************************************************************/
void PININT_Handler ( uint32_t irq_num )
{
	uint32_t condition, regVal;
	
	flex_int_counter[irq_num]++;
	if ( LPC_PIN_INT->PMCTRL & SEL_PMATCH )
	{
		condition = ( LPC_PIN_INT->PMCFG >> (8 + irq_num *3) ) & 0x7;
		switch ( condition )
		{
			case PATTERN_LEVEL_HI: 
			case PATTERN_LEVEL_LO:
				flex_int_level_counter[irq_num]++;
				break;
			case PATTERN_RISING:
			case PATTERN_FALLING:
			case PATTERN_R_OR_F:
				if ( LPC_PIN_INT->RISE & (0x1<<irq_num) )
				{
					LPC_PIN_INT->RISE = 0x1<<irq_num;
					flex_int_rising_edge_counter[irq_num]++;
				}
				else
				{
					LPC_PIN_INT->FALL = 0x1<<irq_num;
					flex_int_falling_edge_counter[irq_num]++;
				}
				regVal = LPC_PIN_INT->PMSRC;
				LPC_PIN_INT->PMSRC = regVal;
				break;
			case PATTERN_EVENT:
				/* Non-sticky, won't know if it's rising or falling, increment both. */
				flex_int_rising_edge_counter[irq_num]++;
				flex_int_falling_edge_counter[irq_num]++;
				break;
			default:
				break;
		}
		return;	
	}
	/* regular pin interrupt */
  if ( LPC_PIN_INT->IST & (0x1<<irq_num) )
  {
		if ( LPC_PIN_INT->ISEL & (0x1<<irq_num) )
		{
			flex_int_level_counter[irq_num]++;
		}
		else
		{
			if ( ( LPC_PIN_INT->RISE & (0x1<<irq_num) ) && ( LPC_PIN_INT->IENR & (0x1<<irq_num) ) )
			{
				flex_int_rising_edge_counter[irq_num]++;
				LPC_PIN_INT->RISE = 0x1<<irq_num;
			}
			if ( ( LPC_PIN_INT->FALL & (0x1<<irq_num) ) && ( LPC_PIN_INT->IENF & (0x1<<irq_num) ) )
			{
				flex_int_falling_edge_counter[irq_num]++;
				LPC_PIN_INT->FALL = 0x1<<irq_num;
			}
			LPC_PIN_INT->IST = 0x1<<irq_num;
		}
  }		
}

/*****************************************************************************
** Function name:		PININT0_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT0_IRQHandler(void)
{
	PININT_Handler ( 0 );
  return;
}

/*****************************************************************************
** Function name:		PININT1_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT1_IRQHandler(void)
{
	PININT_Handler ( 1 );
  return;
}

/*****************************************************************************
** Function name:		PININT2_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT2_IRQHandler(void)
{
	PININT_Handler ( 2 );
  return;
}

/*****************************************************************************
** Function name:		PININT3_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PIN3_IRQHandler(void)
{
	PININT_Handler ( 3 );
  return;
}

/*****************************************************************************
** Function name:		PININT4_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT4_IRQHandler(void)
{
	PININT_Handler ( 4 );
  return;
}

/*****************************************************************************
** Function name:		PININT5_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT5_IRQHandler(void)
{
	PININT_Handler ( 5 );
  return;
}

/*****************************************************************************
** Function name:		PININT6_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT6_IRQHandler(void)
{
	PININT_Handler ( 6 );
  return;
}

/*****************************************************************************
** Function name:		PININT7_IRQHandler
**
** Descriptions:		Use one GPIO pin as interrupt source
**
** parameters:			None
** 						
** Returned value:		None
** 
*****************************************************************************/
void PININT7_IRQHandler(void)
{
	PININT_Handler ( 7 );
  return;
}

/*****************************************************************************
** Function name:		GPIOInit
**
** Descriptions:		Initialize GPIO and GPIO INT block
**
** parameters:			None
** Returned value:	None
** 
*****************************************************************************/
void GPIOInit( void )
{
  /* Enable AHB clock to the GPIO domain. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);
	
	/* Peripheral reset control to GPIO and GPIO INT, a "1" bring it out of reset. */
	LPC_SYSCON->PRESETCTRL &= ~(0x1<<10);
	LPC_SYSCON->PRESETCTRL |= (0x1<<10);
  return;
}

/*****************************************************************************
** Function name:		GPIOSetPinInterrupt
**
** Descriptions:		Set interrupt sense, event, etc.
**						sense: edge or level, 0 is edge, 1 is level 
**						event/polarity: 0 is active low/falling, 1 is high/rising.
**
** parameters:			channel #, port #, bit position, sense, event(polarity)
** 						
** Returned value:		None
** 
*****************************************************************************/
void GPIOSetPinInterrupt( uint32_t channelNum, uint32_t portNum, uint32_t bitPosi,
		uint32_t sense, uint32_t event )
{
	/* right now, there is only port 0 on LPC8xx, more ports may be added, save
	temporarily for future use. */
#if 1
	LPC_SYSCON->PINTSEL[channelNum] = bitPosi; 
#if NMI_ENABLED
	NVIC_DisableIRQ( (IRQn_Type)(PININT0_IRQn+channelNum) );
	NMI_Init( (IRQn_Type)(PININT0_IRQn+channelNum) );
#else
	NVIC_EnableIRQ((IRQn_Type)(PININT0_IRQn+channelNum));
#endif
#else
  switch ( channelNum )
  {
	case CHANNEL0:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[0] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[0] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT0_IRQn);
	break;
	case CHANNEL1:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[1] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[1] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT1_IRQn);
	break;
	case CHANNEL2:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[2] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[2] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT2_IRQn);
	break;
	case CHANNEL3:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[3] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[3] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT3_IRQn);  	 
	break;
	case CHANNEL4:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[4] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[4] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT4_IRQn);
	break;
	case CHANNEL5:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[5] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[5] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT5_IRQn);
	break;
	case CHANNEL6:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[6] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[6] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT6_IRQn);
	break;
	case CHANNEL7:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[7] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[7] = bitPosi; 
	  }
	  NVIC_EnableIRQ(PININT7_IRQn);
	break;
	default:
	  break;
  }
#endif
	
  if ( sense == 0 )
  {
		LPC_PIN_INT->ISEL &= ~(0x1<<channelNum);	/* Edge trigger */
		if ( event == 0 )
		{
			LPC_PIN_INT->IENF |= (0x1<<channelNum);	/* faling edge */
		}
		else
		{
			LPC_PIN_INT->IENR |= (0x1<<channelNum);	/* Rising edge */
		}
  }
  else
  {
		LPC_PIN_INT->ISEL |= (0x1<<channelNum);	/* Level trigger. */
		LPC_PIN_INT->IENR |= (0x1<<channelNum);	/* Level enable */
		if ( event == 0 )
		{
			LPC_PIN_INT->IENF &= ~(0x1<<channelNum);	/* active-low */
		}
		else
		{
			LPC_PIN_INT->IENF |= (0x1<<channelNum);	/* active-high */
		}	
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOPinIntEnable
**
** Descriptions:		Enable Interrupt
**
** parameters:			channel num, event(0 is falling edge, 1 is rising edge)
** Returned value:		None
** 
*****************************************************************************/
void GPIOPinIntEnable( uint32_t channelNum, uint32_t event )
{
  if ( !( LPC_PIN_INT->ISEL & (0x1<<channelNum) ) )
  {
		if ( event == 0 )
		{
			LPC_PIN_INT->SIENF |= (0x1<<channelNum);	/* faling edge */
		}
		else
		{
			LPC_PIN_INT->SIENR |= (0x1<<channelNum);	/* Rising edge */
		}
  }
  else
  {
		LPC_PIN_INT->SIENR |= (0x1<<channelNum);	/* Level */
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOPinIntDisable
**
** Descriptions:		Disable Interrupt
**
** parameters:			channel num, event(0 is falling edge, 1 is rising edge)
** 						
** Returned value:		None
** 
*****************************************************************************/
void GPIOPinIntDisable( uint32_t channelNum, uint32_t event )
{
  if ( !( LPC_PIN_INT->ISEL & (0x1<<channelNum) ) )
  {
		if ( event == 0 )
		{
			LPC_PIN_INT->CIENF |= (0x1<<channelNum);	/* faling edge */
		}
		else
		{
			LPC_PIN_INT->CIENR |= (0x1<<channelNum);	/* Rising edge */
		}
  }
  else
  {
		LPC_PIN_INT->CIENR |= (0x1<<channelNum);	/* Level */
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOPinIntStatus
**
** Descriptions:		Get Interrupt status
**
** parameters:			channel num
** 						
** Returned value:		None
** 
*****************************************************************************/
uint32_t GPIOPinIntStatus( uint32_t channelNum )
{
  if ( LPC_PIN_INT->IST & (0x1<<channelNum) )
  {
		return( 1 );
  }
  else
  {
		return( 0 );
  }
}

/*****************************************************************************
** Function name:		GPIOPinIntClear
**
** Descriptions:		Clear Interrupt
**
** parameters:			channel num
** 						
** Returned value:		None
** 
*****************************************************************************/
void GPIOPinIntClear( uint32_t channelNum )
{
  if ( !( LPC_PIN_INT->ISEL & (0x1<<channelNum) ) )
  {
		LPC_PIN_INT->IST = (1<<channelNum);
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOGetPinValue
**
** Descriptions:		Read Current state of port pin, PIN register value
**
** parameters:			port num, bit position
** Returned value:		None
**
*****************************************************************************/
uint32_t GPIOGetPinValue( uint32_t portNum, uint32_t bitPosi )
{
  uint32_t regVal = 0;	

  if( bitPosi < 0x20 )
  {	
		if ( LPC_GPIO_PORT->PIN0 & (0x1<<bitPosi) )
		{
			regVal = 1;
		}
  }
	else if( bitPosi == 0xFF )
  {
		regVal = LPC_GPIO_PORT->PIN0;
  }
  return ( regVal );		
}

/*****************************************************************************
** Function name:		GPIOSetBitValue
**
** Descriptions:		Set/clear a bit in a specific position
**
** parameters:			port num, bit position, bit value
** 						
** Returned value:		None
**
*****************************************************************************/
void GPIOSetBitValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal )
{
  if ( bitVal )
  {
		LPC_GPIO_PORT->SET0 = 1<<bitPosi;
  }
  else
  {
		LPC_GPIO_PORT->CLR0 = 1<<bitPosi;
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOSetDir
**
** Descriptions:		Set the direction in GPIO port
**
** parameters:			portNum, bit position, direction (1 out, 0 input)
** 						
** Returned value:		None
**
*****************************************************************************/
void GPIOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir )
{
  if( dir )
  {
		LPC_GPIO_PORT->DIR0 |= (1<<bitPosi);
  }
  else
  {
		LPC_GPIO_PORT->DIR0 &= ~(1<<bitPosi);
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOPatternMatchInit
**
** Descriptions:		Initialize pattern match to constant 0, disable any unused slices.
**
** parameters:			None
** 						
** Returned value:	None
**
*****************************************************************************/
void GPIOPatternMatchInit( void )
{
  LPC_PIN_INT->PMCFG 			= (PATTERN_CONST_0<<BIT_SLICE0)|
														(PATTERN_CONST_0<<BIT_SLICE1)|
														(PATTERN_CONST_0<<BIT_SLICE2)|
														(PATTERN_CONST_0<<BIT_SLICE3)|
														(PATTERN_CONST_0<<BIT_SLICE4)|
														(PATTERN_CONST_0<<BIT_SLICE5)|
														(PATTERN_CONST_0<<BIT_SLICE6)|
														(PATTERN_CONST_0<<BIT_SLICE7);
	return;
}

/*****************************************************************************
** Function name:		GPIOSetPatternMatchInput
**
** Descriptions:		Set pattern match input, map to PINTSEL register.
**
** parameters:			channel #, port #, and port pin
** 						
** Returned value:	None
**
*****************************************************************************/
void GPIOSetPatternMatchInput( uint32_t channelNum, uint32_t portNum, uint32_t bitPosi )
{
		/* right now, there is only port 0 on LPC8xx, more ports may be added, save
	temporarily for future use. */
#if 1
	LPC_SYSCON->PINTSEL[channelNum] = bitPosi; 
#else
  switch ( channelNum )
  {
	case CHANNEL0:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[0] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[0] = bitPosi; 
	  }
	break;
	case CHANNEL1:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[1] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[1] = bitPosi; 
	  }
	break;
	case CHANNEL2:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[2] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[2] = bitPosi; 
	  }
	break;
	case CHANNEL3:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[3] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[3] = bitPosi; 
	  }
	break;
	case CHANNEL4:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[4] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[4] = bitPosi; 
	  }
	break;
	case CHANNEL5:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[5] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[5] = bitPosi; 
	  }
	break;
	case CHANNEL6:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[6] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[6] = bitPosi; 
	  }
	break;
	case CHANNEL7:
	  if ( portNum )
	  {
			LPC_SYSCON->PINTSEL[7] = bitPosi + 24; 
	  }
	  else
	  {
			LPC_SYSCON->PINTSEL[7] = bitPosi; 
	  }
	break;
	default:
	  break;
  }
#endif
  return;
}

/*****************************************************************************
** Function name:		GPIOSetPatternMatchSlice
**
** Descriptions:		Set the pattern match slice based on the input
**
** parameters:			channel #, slice #, condition, and product EP 
** 						
** Returned value:	None
**
*****************************************************************************/
void GPIOSetPatternMatchSlice( uint32_t channelNum, uint32_t sliceNum, uint32_t condition, uint32_t isProdEP  )
{
	LPC_PIN_INT->PMSRC &= ~(0x7 << (8 + sliceNum * 3));
	LPC_PIN_INT->PMSRC |= (channelNum << (8 + sliceNum * 3));
	LPC_PIN_INT->PMCFG &= ~(0x7 << (8 + sliceNum * 3));
  LPC_PIN_INT->PMCFG |= (condition << (8 + sliceNum * 3));
	if ( isProdEP  )
	{
		if ( sliceNum != 7 )
		{
			LPC_PIN_INT->PMCFG |= (0x1 << sliceNum);
		}
		NVIC_EnableIRQ((IRQn_Type)(PININT0_IRQn+sliceNum));
	}
	return;
}

/*****************************************************************************
** Function name:		GPIOPatternMatchEnable
**
** Descriptions:		Enable the pattern match engine, called only after inputs 
**									and slices are configured. 
**
** parameters:			RxEv enable flag (TRUE or FALSE )
** 						
** Returned value:	None
**
*****************************************************************************/
void GPIOPatternMatchEnable( uint32_t PMatchSel, uint32_t RxEvEna )
{
	LPC_PIN_INT->PMCTRL &= ~(SEL_PMATCH | ENA_PXEV);

	if ( PMatchSel )
	{
		LPC_PIN_INT->PMCTRL |= SEL_PMATCH;
	}	
	if ( RxEvEna )
	{
		LPC_PIN_INT->PMCTRL |= ENA_PXEV;
	}
	return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
