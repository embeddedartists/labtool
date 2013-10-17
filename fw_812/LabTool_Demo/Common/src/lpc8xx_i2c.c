/**********************************************************************
* $Id$		lpc8xx_i2c.c		2011-06-02
*//**
* @file		lpc8xx_i2c.c
* @brief	Contains all functions support for I2C firmware library
* 			on lpc8xx
* @version	1.0
* @date		02. June. 2011
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
* documentation is hereby granted, under NXP Semiconductors’
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

/* Includes ------------------------------------------------------------------- */
#include "lpc8xx.h"
#include "lpc8xx_nmi.h"
#include "lpc8xx_i2c.h"

extern volatile uint8_t I2CSlaveTXBuffer[I2C_BUFSIZE];
extern volatile uint8_t I2CSlaveRXBuffer[I2C_BUFSIZE];
extern volatile uint32_t I2CMonBuffer[I2C_MONBUFSIZE];

volatile uint32_t I2CStatus;

volatile uint32_t I2CInterruptCount = 0;
volatile uint32_t I2CMstRXCount = 0;
volatile uint32_t I2CMstTXCount = 0;
volatile uint32_t mstrxrdy = 0, msttxrdy = 0, mstidle = 0;
volatile uint32_t I2CSlvRXCount = 0;
volatile uint32_t I2CSlvTXCount = 0;
volatile uint32_t I2CSlvAddrCount = 0;
volatile uint32_t slvrxrdy = 0, slvtxrdy = 0;
volatile uint32_t slvaddrrcvd = 0;
volatile uint32_t I2CMonRXCount = 0;
volatile uint32_t SlaveAddr = 0;
volatile uint32_t RetryCount = 0;

/* Timeout related */
volatile uint32_t I2CSCLTimeoutCount = 0;
volatile uint32_t I2CEventTimeoutCount = 0;

/* Master related */
volatile uint32_t I2CMstIdleCount = 0;
volatile uint32_t I2CMstNACKAddrCount = 0;
volatile uint32_t I2CMstNACKTXCount = 0;
volatile uint32_t I2CARBLossCount = 0;
volatile uint32_t I2CMstSSErrCount = 0;

/* Slave related */
volatile uint32_t I2CSlvSelectedCount = 0;
volatile uint32_t I2CSlvDeselectedCount = 0;
volatile uint32_t I2CSlvNotStrCount = 0;

/* Monitor related */
volatile uint32_t I2CMonIdleCount = 0;
volatile uint32_t I2CMonOverrunCount = 0;

#if TIMEOUT_ENABLED
/*****************************************************************************
** Function name:		I2C_I2CTimeoutStatus
**
** Descriptions:		I2C interrupt handler to handle timeout status
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_I2CTimeoutStatus( LPC_I2C_TypeDef *I2Cx, uint32_t active )
{
  if(active & STAT_SCLTIMEOUT) {
		I2CSCLTimeoutCount++;
		I2Cx->STAT = STAT_SCLTIMEOUT;
  }
  if(active & STAT_EVTIMEOUT) {
		I2CEventTimeoutCount++;
		I2Cx->STAT = STAT_EVTIMEOUT;
  }
}
#endif

/*****************************************************************************
** Function name:		I2C_MasterStatus
**
** Descriptions:		I2C interrupt handler to handle master status
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_MasterStatus( LPC_I2C_TypeDef *I2Cx, uint32_t active )
{
  if (active & STAT_MSTARBLOSS) {
		I2CARBLossCount++;
		I2CStatus |= STAT_MSTARBLOSS;
		I2Cx->STAT = STAT_MSTARBLOSS;
		/* For debugging only. */
//	I2Cx->I2CCFG |= CFG_MONCLKSTR|CFG_MONENA);
  }
  if (active & STAT_MSTSSERR) {
		I2CMstSSErrCount++;
		I2CStatus |= STAT_MSTSSERR;
		I2Cx->STAT = STAT_MSTSSERR;
		/* For debugging only. Really bad happened if come here. */
//		I2Cx->I2CCFG = (CFG_MONCLKSTR|CFG_MONENA);
  }
  return;
}

/*****************************************************************************
** Function name:		I2C_SlaveStatus
**
** Descriptions:		I2C interrupt handler to handle slave status
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_SlaveStatus( LPC_I2C_TypeDef *I2Cx, uint32_t active )
{
  if (active & STAT_SLVNOTSTR) {
		I2CSlvNotStrCount++;
		I2Cx->INTENCLR = STAT_SLVNOTSTR;
  }
  if (active & STAT_SLVDESEL) {
		I2CSlvDeselectedCount++;
		I2Cx->STAT = STAT_SLVDESEL;
  }
  return;
}

#if I2C_MONITOR_MODE
/*****************************************************************************
** Function name:		I2C_MonitorStatus
**
** Descriptions:		I2C interrupt handler to handle monitor status
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_MonitorStatus( LPC_I2C_TypeDef *I2Cx, uint32_t active )
{
  if (active & STAT_MONIDLE) {
		I2CMonIdleCount++;
		I2Cx->STAT = STAT_MONIDLE;
  }
  if (active & STAT_MONOVERRUN) {
		I2CMonOverrunCount++;
		I2Cx->STAT = STAT_MONOVERRUN;
  }
  return;
}
#endif

/*****************************************************************************
** Function name:		I2C_IRQHandler
**
** Descriptions:		I2C interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_IRQHandler(void)
{
  uint32_t active = LPC_I2C->INTSTAT;
  uint32_t i2cmststate = LPC_I2C->STAT & MASTER_STATE_MASK;	/* Only check Master and Slave State */
  uint32_t i2cslvstate = LPC_I2C->STAT & SLAVE_STATE_MASK;
  uint32_t i;

  if ( active ) {
		I2CInterruptCount++;
  }

#if I2C_MONITOR_MODE
	I2C_MonitorStatus(LPC_I2C, active);
#endif	
  /* If monitor mode is turned on, it needs to be the highest priority,
  otherwise, the slave TX and RX may stretch/release the clock and grab 
  the data already. When that happens, Monitor may have missed a clock already. */
  if(active & STAT_MONRDY) {
		I2CMonBuffer[I2CMonRXCount++] = LPC_I2C->MONRXDAT;
		if ( I2CMonRXCount == I2C_MONBUFSIZE ) {
			I2CMonRXCount = 0;
		}
  }

  /* The error interrupts should have higher priority too before data can be 
  processed. */  
#if TIMEOUT_ENABLED
  /* I2C timeout related interrupts. */
  I2C_I2CTimeoutStatus(LPC_I2C, active);
#endif
  
  /* master related interrupts. */
  I2C_MasterStatus(LPC_I2C, active);
	
  if ( active & STAT_MSTPEND )
  {
		LPC_I2C->INTENCLR = STAT_MSTPEND;
		/* Below five states are for Master mode: IDLE, TX, RX, NACK_ADDR, NAC_TX. 
		IDLE is not under consideration for now. */
		switch ( i2cmststate )
		{
			case STAT_MSTIDLE:
			default:
				I2CMstIdleCount++;
				mstidle = 1;
			break;
	   
			case STAT_MSTRX:
				I2CMstRXCount++;
				mstrxrdy = 1;
			break;
	  
			case STAT_MSTTX:
				I2CMstTXCount++;
				msttxrdy = 1;
			break;
	  
			case STAT_MSTNACKADDR:
				I2CMstNACKAddrCount++;
			break;
	  
			case STAT_MSTNACKTX:
				I2CMstNACKTXCount++;
			break;
		}	  
  }
     
  /* slave related interrupts. */
  I2C_SlaveStatus(LPC_I2C, active);

  if ( active & STAT_SLVPEND )
  {  	
		LPC_I2C->INTENCLR = STAT_SLVPEND;
		/* Below three states are for Slave mode: Address Received, TX, and RX. */
		switch ( i2cslvstate )
		{
			case STAT_SLVRX:
				slvrxrdy = 1;
				I2CSlaveRXBuffer[I2CSlvRXCount++] = LPC_I2C->SLVDAT;
				if ( I2CSlvRXCount != I2C_BUFSIZE )
				{
					LPC_I2C->SLVCTL = CTL_SLVCONTINUE;
				}
				else
				{
					LPC_I2C->SLVCTL = CTL_SLVNACK | CTL_SLVCONTINUE;
					I2CSlvRXCount = 0;
					for ( i = 0; i < I2C_BUFSIZE; i++ )
					{
						/* copy data from SlaveReceive buffer and send the received data back 
						to the master if "SlaveTX" is seen. */
						I2CSlaveTXBuffer[i] = I2CSlaveRXBuffer[i];
					}
				}
				LPC_I2C->INTENSET = STAT_SLVPEND; 
			break;
	  
			case STAT_SLVTX:
				slvtxrdy = 1;
				LPC_I2C->SLVDAT = I2CSlaveTXBuffer[I2CSlvTXCount++];
				LPC_I2C->SLVCTL = CTL_SLVCONTINUE;
				if ( I2CSlvTXCount == I2C_BUFSIZE )
				{
					I2CSlvTXCount = 0;
					for ( i = 0; i < I2C_BUFSIZE; i++ )	/* clear buffer */
					{
						/* If "SlaveTX" is done, clear the slave tx buffer. If the next packet is slave rx, 
						the slave tx will be filled again. If the next packet is slave tx again, "0"s will
						be sent out on the bus. */
						I2CSlaveTXBuffer[i] = 0;
					}
				} 
				LPC_I2C->INTENSET = STAT_SLVPEND;
			break;
	  
			case STAT_SLVADDR:
				/* slave address is received. */
				/* slaveaddrrcvd, slavetxrdy, and slaverxrdy are not used anymore
				as slave tx and rx are handled inside the ISR now. 
				I2C_SlaveSendData(), I2C_SlaveReceiveData(), I2C_SlaveReceiveSend()
				are for polling mode only. */
				slvaddrrcvd = 1;
				I2CSlvAddrCount++;
				LPC_I2C->SLVCTL = CTL_SLVCONTINUE;
				LPC_I2C->INTENSET = STAT_SLVPEND; 
			break;
	  
			default:
			break;
		}
  }
  return;
}

/*****************************************************************************
** Function name:		I2C_MstInit
**
** Descriptions:		I2C port initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_MstInit( LPC_I2C_TypeDef *I2Cx, uint32_t div, uint32_t cfg, uint32_t dutycycle )
{
	/* For master mode plus, if desired I2C clock is 1MHz (SCL high time + SCL low time). 
	If CCLK is 36MHz, MasterSclLow and MasterSclHigh are 0s, 
	SCL high time = (ClkDiv+1) * (MstSclHigh + 2 )
	SCL low time = (ClkDiv+1) * (MstSclLow + 2 )
	Pre-divider should be 8. 
	If fast mode, e.g. communicating with a temp sensor, Max I2C clock is set to 400KHz.
	Pre-divider should be 11. */
  I2Cx->DIV = div;
  I2Cx->CFG &= ~(CFG_MSTENA);
  msttxrdy = mstrxrdy = 0;

  I2Cx->MSTTIME = TIM_MSTSCLLOW(dutycycle) | TIM_MSTSCLHIGH(dutycycle);
#if I2C_INTERRUPT
  I2Cx->INTENSET |= (STAT_MSTARBLOSS | STAT_MSTSSERR);
  NVIC_DisableIRQ(I2C_IRQn);
  NVIC_ClearPendingIRQ(I2C_IRQn);
#if NMI_ENABLED
	NMI_Init( I2C_IRQn );
#else
  NVIC_EnableIRQ(I2C_IRQn);
#endif
#endif
  I2Cx->CFG |= cfg;
  return;
}

/*****************************************************************************
** Function name:		I2C_SlvInit
**
** Descriptions:		I2C Slave mode initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_SlvInit( LPC_I2C_TypeDef *I2Cx, uint32_t addr, uint32_t cfg, uint32_t clkdiv )
{
  I2Cx->CFG &= ~(CFG_SLVENA);
  slvtxrdy = slvrxrdy = 0;
  slvaddrrcvd = 0;

  /* For master mode plus, if desired I2C clock is 1MHz (SCL high time + SCL low time). 
	If CCLK is 36MHz, MasterSclLow and MasterSclHigh are 0s, 
	SCL high time = (ClkDiv+1) * (MstSclHigh + 2 )
	SCL low time = (ClkDiv+1) * (MstSclLow + 2 )
	Pre-divider should be 8. 
	If fast mode, e.g. communicating with a temp sensor, Max I2C clock is set to 400KHz.
	Pre-divider should be 11. */
  I2Cx->DIV = clkdiv;

  /* Enable all addresses */
  I2Cx->SLVADR0 = addr;
  I2Cx->SLVADR1 = addr + 0x20;
  I2Cx->SLVADR2 = addr + 0x40; 
  I2Cx->SLVADR3 = addr + 0x60;

#if ADDR_QUAL_ENABLE
  /* RANGE (bit0 = 1) or MASK(bit0 = 0) mode */
#if 1
  /* RANGE (bit0 = 1) mode, (SLVADR0 <= addr <= SLVQUAL0) */
  I2Cx->SLVQUAL0 = ((LPC_I2C->SLVADR0 + 0x60) | 0x01);
#else
  /* MASK (bit0 = 0) mode, (addr & ~SLVQUAL0) = SLVADR0 */
  I2Cx->SLVQUAL0 = 0x06;
//  I2Cx->SLVQUAL0 = 0xfe;
#endif
#endif
  
#if I2C_INTERRUPT
  I2Cx->INTENSET |= (STAT_SLVDESEL | STAT_SLVNOTSTR);
  NVIC_DisableIRQ(I2C_IRQn);
  NVIC_ClearPendingIRQ(I2C_IRQn);
  NVIC_EnableIRQ(I2C_IRQn);
#endif
  I2Cx->CFG |= cfg;
  return;
}

#if I2C_MONITOR_MODE
/*****************************************************************************
** Function name:		I2C_MonInit
**
** Descriptions:		I2C Monitor mode initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_MonInit( LPC_I2C_TypeDef *I2Cx, uint32_t cfg )
{
  I2Cx->CFG &= ~CFG_MONENA;
#if I2C_INTERRUPT
  I2Cx->INTENSET = STAT_MONRDY | STAT_MONOVERRUN | STAT_MONIDLE;
  NVIC_DisableIRQ(I2C_IRQn);
  NVIC_ClearPendingIRQ(I2C_IRQn);
  NVIC_EnableIRQ(I2C_IRQn);
#endif
  I2Cx->CFG |= cfg;
}
#endif

#if TIMEOUT_ENABLED
/*****************************************************************************
** Function name:		I2C_TimeoutInit
**
** Descriptions:		I2C Timeout initialization routine
**				
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C_TimeoutInit( LPC_I2C_TypeDef *I2Cx, uint32_t timeout_value )
{
  uint32_t to_value;
  
  I2Cx->CFG &= ~CFG_TIMEOUTENA;
  to_value = I2Cx->TIMEOUT & 0x000F;
  to_value |= (timeout_value << 4);
  I2Cx->TIMEOUT = to_value;  
#if I2C_INTERRUPT
  I2Cx->INTENSET |= (STAT_EVTIMEOUT|STAT_SCLTIMEOUT); 
  
  NVIC_DisableIRQ(I2C_IRQn);
  NVIC_ClearPendingIRQ(I2C_IRQn);
  NVIC_EnableIRQ(I2C_IRQn);
#endif
  I2Cx->CFG |= CFG_TIMEOUTENA;
}
#endif

/*****************************************************************************
** Function name:		I2C_CheckError
**
** Descriptions:		In the multi-master mode environment, error check
**						is required as arbitration error may cause program
**						to hang. In the real-world app., if error occurs, 
**						a graceful waiting period is needed before a retry occur. 
**						If retry count reaches zero, either stay in the master mode 
**						or switch to slave mode. This test has infinite retry 
**						without retry count and delay.
**						If interrupt is enabled, status is cleared inside ISR.  				
** parameters:			None
** Returned value:		Non-zero means error has occured.
** 
*****************************************************************************/
uint32_t I2C_CheckError( LPC_I2C_TypeDef *I2Cx )
{
#if I2C_INTERRUPT
  /* I2CStatus is updated inside the ISR. */
  if ( I2CStatus & ( STAT_MSTARBLOSS | STAT_MSTSSERR ) )
  {
		if ( I2CStatus & STAT_MSTARBLOSS ) {
			I2CStatus &= ~STAT_MSTARBLOSS;
		}
		if ( I2CStatus & STAT_MSTSSERR ) {
			I2CStatus &= ~STAT_MSTSSERR;
		}
		RetryCount++;
		return ( 1 );
  }
  else {
		return ( 0 );
  }
#else
  if ( I2Cx->STAT & (STAT_MSTARBLOSS | STAT_MSTSSERR) )
  {
		/* In polling mode, treat all the error related master status the same
		for now and clear all if set. Will address them individually later. */
		I2Cx->STAT = STAT_MSTARBLOSS | STAT_MSTSSERR;
		RetryCount++;
		return ( 1 );
  }
  else {
		return ( 0 );
  }	
#endif 
}

/*****************************************************************************
** Function name:		I2C_CheckIdle
**
** Descriptions:		Check idle, if the master is NOT idle, it shouldn't 
**									start the transfer.		
** parameters:			None
** Returned value:		None, if NOT IDLE, spin here forever.
** 
*****************************************************************************/
void I2C_CheckIdle( LPC_I2C_TypeDef *I2Cx )
{
#if I2C_INTERRUPT
  I2Cx->INTENSET = STAT_MSTPEND;
  while(!mstidle);
  mstidle = 0;
#else
  /* Once the master is enabled, pending bit should be set, as the state should be in IDLE mode. */
  /* Pending, but not idle, spin here forever. */
  while ( (I2Cx->STAT & (STAT_MSTPEND|MASTER_STATE_MASK)) != (STAT_MSTPEND|STAT_MSTIDLE) );
#endif
  return;
}

/*****************************************************************************
** Function name:		I2C_MstSend
**
** Descriptions:		Send a block of data to the I2C port, the 
**									first parameter is the device addre, the 2nd is buffer 
**                  pointer, the 3rd is the block length.
**
** parameters:			device addr, buffer pointer, and the block length
** Returned value:		None
** 
*****************************************************************************/
void I2C_MstSend( LPC_I2C_TypeDef *I2Cx, uint32_t addr, uint8_t *tx, uint32_t Length )
{
  uint32_t i;

  I2Cx->MSTDAT = addr;
  I2Cx->MSTCTL = CTL_MSTSTART;
#if I2C_INTERRUPT
  msttxrdy = 0;
  I2Cx->INTENSET = STAT_MSTPEND;
#endif
#if I2C_INTERRUPT
  for ( i = 0; i < Length; i++ ) 
  {
		while(!msttxrdy) {
			if ( I2C_CheckError(I2Cx) ) {
				I2C_CheckIdle(I2Cx);
				/* Recursive call here. Be very careful with stack over flow if come here, especially
				when the other master trys to grab the bus all the time. */
//				while ( 1 );
				I2C_MstSend( I2Cx, addr, tx, Length );
				return;
			}
		}
		msttxrdy = 0;
		I2Cx->MSTDAT = tx[i];
		I2Cx->MSTCTL = CTL_MSTCONTINUE;
		I2Cx->INTENSET = STAT_MSTPEND;
  }
  
  /* Wait for the last one to go out. */
  while(!msttxrdy) {
		if ( I2C_CheckError(I2Cx) ) {
	    I2C_CheckIdle(I2Cx);
			/* Recursive call here. Be very careful with stack over flow if come here. especially
			when the other master trys to grab the bus all the time. */
//			while ( 1 );
			I2C_MstSend( I2Cx, addr, tx, Length );
			return;
		}
  }
  msttxrdy = 0;
#else
  for ( i = 0; i < Length; i++ ) 
  {
		/* Move only if TXRDY is ready */
		while (!(I2Cx->STAT & STAT_MSTPEND));
		if((I2Cx->STAT & MASTER_STATE_MASK) != STAT_MSTTX)
			while(1);
		I2Cx->MSTDAT = *tx++;
		I2Cx->MSTCTL = CTL_MSTCONTINUE;
  }
  
  /* Wait for the last one to go out. */
  while (!(I2Cx->STAT & STAT_MSTPEND));
  if((I2Cx->STAT & MASTER_STATE_MASK) != STAT_MSTTX)
		while(1);
#endif
  /* Send STOP condition. */  
  I2Cx->MSTCTL = CTL_MSTSTOP | CTL_MSTCONTINUE;
  I2C_CheckIdle(I2Cx);
  return; 
}

/*****************************************************************************
** Function name:		I2C_MstReceive
** Descriptions:		the module will receive a block of data from 
**						the I2C.
** parameters:			Device address, buffer pointer, and block length
** Returned value:		None
** 
*****************************************************************************/
void I2C_MstReceive( LPC_I2C_TypeDef *I2Cx, uint32_t addr, uint8_t *rx, uint32_t Length )
{
  uint32_t i;

  I2Cx->MSTDAT = addr;  
  I2Cx->MSTCTL = CTL_MSTSTART;
#if I2C_INTERRUPT
  mstrxrdy = 0;
  I2Cx->INTENSET = STAT_MSTPEND;
#endif 
	for ( i = 0; i < Length; i++ )
	{
#if I2C_INTERRUPT
		while(!mstrxrdy)
		{
			if ( I2C_CheckError(I2Cx) ) {
				I2C_CheckIdle(I2Cx);
				/* Recursive call here. Be very careful with stack over flow if come here. especially
				when the other master trys to grab the bus all the time. */
//	    while ( 1 );
				I2C_MstReceive( I2Cx, addr, rx, Length );
				return;
			}
		}
		mstrxrdy = 0;
		*rx++ = I2Cx->MSTDAT;
		if ( i != Length -1 )
		{
			I2Cx->MSTCTL = CTL_MSTCONTINUE;
			I2Cx->INTENSET = STAT_MSTPEND;
		}
#else
		/* Slave address has been sent, master receive is ready. */
		while (!(I2Cx->STAT & STAT_MSTPEND));
		if((I2Cx->STAT & MASTER_STATE_MASK) != STAT_MSTRX)
			while( 1 );
		*rx++ = I2Cx->MSTDAT;
		if ( i != Length -1 )
		{
			I2Cx->MSTCTL = CTL_MSTCONTINUE;
		}
#endif
  }
  I2Cx->MSTCTL = CTL_MSTSTOP | CTL_MSTCONTINUE;
  I2C_CheckIdle(I2Cx);
  return; 
}

/*****************************************************************************
** Function name:		I2C_MstSendRcv
**
** Descriptions:		Send a block of data to the I2C port combining master
**                      send and master recevie with repeated start in the middle.
**
** parameters:			Dev addr. TX ptr, TX length, RX ptr, and RX length.
** Returned value:		None
** 
*****************************************************************************/
void I2C_MstSendRcv( LPC_I2C_TypeDef *I2Cx, uint32_t addr, uint8_t *tx, uint32_t txlen, uint8_t *rx, uint32_t rxlen )
{
  uint32_t i;

  I2Cx->MSTDAT = addr;
  I2Cx->MSTCTL = CTL_MSTSTART;
#if I2C_INTERRUPT
  msttxrdy = mstrxrdy = 0;
  I2Cx->INTENSET = STAT_MSTPEND;
#endif 
  for ( i = 0; i < txlen; i++ ) 
  {
#if I2C_INTERRUPT
		while(!msttxrdy) 
		{
			if ( I2C_CheckError(I2Cx) ) {
				I2C_CheckIdle(I2Cx);
				/* Recursive call here. Be very careful with stack over flow if come here, especially
				when the other master trys to grab the bus all the time. */
//				while ( 1 );
				I2C_MstSend( I2Cx, addr, tx, txlen );
				return;
	  	}
		}
	msttxrdy = 0;
	I2Cx->MSTDAT = *tx++;
	I2Cx->MSTCTL = CTL_MSTCONTINUE;
	I2Cx->INTENSET = STAT_MSTPEND;
#else
	/* Move only if TXRDY is ready */
	while (!(I2Cx->STAT & STAT_MSTPEND));
	if((I2Cx->STAT & MASTER_STATE_MASK) != STAT_MSTTX)
	  while(1);
	I2Cx->MSTDAT = *tx++;
	I2Cx->MSTCTL = CTL_MSTCONTINUE;
#endif
  }

  /* Wait for the last TX to finish before setting repeated start. */
#if I2C_INTERRUPT
  while(!msttxrdy) 
  {
		if ( I2C_CheckError(I2Cx) ) {
			I2C_CheckIdle(I2Cx);
			/* Recursive call here. Be very careful with stack over flow if come here. especially
			when the other master trys to grab the bus all the time. */
//			while ( 1 );
			I2C_MstSend( I2Cx, addr, tx, txlen );
			return;
		}
  }
  msttxrdy = 0;
#else
  /* Move on only if TXRDY is ready */
  while (!(I2Cx->STAT & STAT_MSTPEND));
  if((I2Cx->STAT & MASTER_STATE_MASK) != STAT_MSTTX)
    while(1);
#endif

  /* Repeated Start */
  I2Cx->MSTDAT = addr|RD_BIT;   
  I2Cx->MSTCTL = CTL_MSTSTART|CTL_MSTCONTINUE;
#if I2C_INTERRUPT
  I2Cx->INTENSET = STAT_MSTPEND;
#endif

  for ( i = 0; i < rxlen; i++ )
  {
#if I2C_INTERRUPT
		while(!mstrxrdy) {
			if ( I2C_CheckError(I2Cx) ) {
				I2C_CheckIdle(I2Cx);
				/* Recursive call here. Be very careful with stack over flow if come here, especially
				when the other master trys to grab the bus all the time. */
//				while ( 1 );
				I2C_MstReceive( I2Cx, addr|RD_BIT, rx, rxlen );
				return;
			}
		}
		mstrxrdy = 0;
		*rx++ = I2Cx->MSTDAT;
		if ( i != rxlen-1 ) {
			I2Cx->MSTCTL = CTL_MSTCONTINUE;
			I2Cx->INTENSET = STAT_MSTPEND;
		}
#else
		/* Slave address has been sent, master receive is ready. */
		while (!(I2Cx->STAT & STAT_MSTPEND));
		if((I2Cx->STAT & MASTER_STATE_MASK) != STAT_MSTRX)
			while(1);
		*rx++ = I2Cx->MSTDAT;
		if ( i != rxlen-1 ) {
			I2Cx->MSTCTL = CTL_MSTCONTINUE;
		}
#endif
  }

  I2Cx->MSTCTL = CTL_MSTSTOP | CTL_MSTCONTINUE;
  I2C_CheckIdle(I2Cx);
  return; 
}

/*****************************************************************************
** Function name:		I2C_SlaveSend
**
** Descriptions:		Send a block of data to the I2C port, the 
**									first parameter is the buffer pointer, the 2nd 
**									parameter is the block length.
**
** parameters:			buffer pointer, and the block length
** Returned value:	None
** 
*****************************************************************************/
void I2C_SlaveSendData( LPC_I2C_TypeDef *I2Cx, uint8_t *tx, uint32_t Length )
{
  uint32_t i;

  I2Cx->SLVCTL = CTL_SLVCONTINUE;
#if I2C_INTERRUPT
  I2Cx->INTENSET = STAT_SLVPEND;
#endif

  for ( i = 0; i < Length; i++ ) 
  {
#if I2C_INTERRUPT
		while(!slvtxrdy);
		slvtxrdy = 0;
		I2Cx->SLVDAT = *tx++;
		I2Cx->SLVCTL = CTL_SLVCONTINUE;
		I2Cx->INTENSET = STAT_SLVPEND; 
#else
		/* Move only if TX is ready */
		while ( (I2Cx->STAT & (STAT_SLVPEND|SLAVE_STATE_MASK)) != (STAT_SLVPEND|STAT_SLVTX) );
		I2Cx->SLVDAT = *tx++;
		I2Cx->SLVCTL = CTL_SLVCONTINUE;
#endif
  }
  return; 
}

/*****************************************************************************
** Function name:		I2C_SlaveReceive
** Descriptions:		the module will receive a block of data from 
**						the I2C, the 2nd parameter is the block 
**						length.
** parameters:			buffer pointer, and block length
** Returned value:		None
** 
*****************************************************************************/
void I2C_SlaveReceiveData( LPC_I2C_TypeDef *I2Cx, uint8_t *rx, uint32_t Length )
{
  uint32_t i;

  I2Cx->SLVCTL = CTL_SLVCONTINUE;
#if I2C_INTERRUPT
  I2Cx->INTENSET = STAT_SLVPEND;
#endif

  for ( i = 0; i < Length; i++ )
  {
#if I2C_INTERRUPT
		while(!slvrxrdy);
		slvrxrdy = 0;
		*rx++ = I2Cx->SLVDAT;
		if ( i != Length-1 ) {
			I2Cx->SLVCTL = CTL_SLVCONTINUE;
		}
		else {
			// I2Cx->SLVCTL = CTL_SLVNACK | CTL_SLVCONTINUE;
			I2Cx->SLVCTL = CTL_SLVCONTINUE;
		}
		I2Cx->INTENSET = STAT_SLVPEND; 
#else	
		while ( (I2Cx->STAT & (STAT_SLVPEND|SLAVE_STATE_MASK)) != (STAT_SLVPEND|STAT_SLVRX) );
		*rx++ = I2Cx->SLVDAT;
		if ( i != Length-1 ) {
			I2Cx->SLVCTL = CTL_SLVCONTINUE;
		}
		else {
			// I2Cx->SLVCTL = CTL_SLVNACK | CTL_SLVCONTINUE; 
			I2Cx->SLVCTL = CTL_SLVCONTINUE; 
		}
#endif
  }
  return; 
}

/*****************************************************************************
** Function name:		I2C_SlaveReceiveSend
** Descriptions:		Based on bit 0 of slave address, this module will
**						send or receive block of data to/from the master.
**						The length of TX and RX are the same. Two buffers,
**						one for TX and one for RX, are used.
** parameters:			tx buffer ptr, rx buffer ptr, and block length
** Returned value:		None
** 
*****************************************************************************/
void I2C_SlaveReceiveSend( LPC_I2C_TypeDef *I2Cx, uint8_t *tx, uint8_t *rx, uint32_t Length )
{
  uint32_t addr7bit;
  
    LPC_I2C->SLVCTL = CTL_SLVCONTINUE;
#if I2C_INTERRUPT
  I2Cx->INTENSET = STAT_SLVPEND;
  while(!slvaddrrcvd);
  slvaddrrcvd = 0;
#else
  while ( (I2Cx->STAT & (STAT_SLVPEND|SLAVE_STATE_MASK)) != (STAT_SLVPEND|STAT_SLVADDR) );
#endif
  SlaveAddr = I2Cx->SLVDAT;
  addr7bit = SlaveAddr & 0xFE;
#if ADDR_QUAL_ENABLE
  if ( I2Cx->SLVQUAL0 & 0xFF )
  {
		/* Either MASK or RANGE */
		if ( I2Cx->SLVQUAL0 & 0x01 ) {
			/* RANGE mode (SLVQUAL0 >= addr >= SLVADR0) */
			if ( (addr7bit < I2Cx->SLVADR0) || (addr7bit > (I2Cx->SLVQUAL0 & 0xFE)) ) {
				/* For debugging. That shouldn't happen. */
				while ( 1 );
			}
		}		
		else {
#if 0
			/* MASK mode */
			if ( (addr7bit & (I2Cx->SLVQUAL0&0xFE)) != I2Cx->SLVADR0 ) {
				/* For debugging. That shouldn't happen. */
				while ( 1 );
			}
#endif
		}		
  }
#else
  if ( ( addr7bit != I2Cx->SLVADR0 ) && ( addr7bit != I2Cx->SLVADR1 )
		&& ( addr7bit != I2Cx->SLVADR2 ) && ( addr7bit != I2Cx->SLVADR3 ) ) {
		/* For debugging. It should never happen, SLDADDR bit changes but addr doesn't match. */
		while ( 1 );
  }
#endif
   
  if ( (SlaveAddr & RD_BIT) == 0x00 ) {
		/* slave reads from master. */
		I2C_SlaveReceiveData( I2Cx, rx, Length ); 
  }
  else {
		/* slave write to master. */
		I2C_SlaveSendData( I2Cx, tx, Length ); 
  }
  return; 
}

//#endif /* _I2C */


/* --------------------------------- End Of File ------------------------------ */
