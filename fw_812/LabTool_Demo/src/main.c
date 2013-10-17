/*!
 * @file
 * @brief     Program entry point
 *
 * @copyright Copyright 2013 Embedded Artists AB
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifdef __USE_CMSIS
#include "LPC8xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "lpc8xx_clkconfig.h"
#include "lpc8xx_gpio.h"
#include "lpc8xx_mrt.h"
#include "lpc8xx_uart.h"
#include "lpc8xx_comp.h"
#include "lpc8xx_i2c.h"
#include "lpc8xx_spi.h"
#include "lpc8xx_pmu.h"
#include "string.h"
#include "swm.h"
#include "sct_fsm.h"

#define PWM_OFF 999

volatile uint8_t I2CMasterTXBuffer[I2C_BUFSIZE];
volatile uint8_t I2CMasterRXBuffer[I2C_BUFSIZE];
volatile uint32_t I2CReadLength, I2CWriteLength;
volatile uint8_t I2CSlaveTXBuffer[I2C_BUFSIZE];
volatile uint8_t I2CSlaveRXBuffer[I2C_BUFSIZE];
volatile uint32_t I2CMonBuffer[I2C_MONBUFSIZE];

#define CS_USED			SLAVE0
volatile uint8_t src_addr[SPI_BUFSIZE];
volatile uint8_t dest_addr[SPI_BUFSIZE];


// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

extern volatile uint32_t mrt_counter;

uint8_t welcomeStr[] = "*** LabTool - temp: xx.x deg. C ***\n";

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define LM75_ADDR      0x90
#define LM75_CONFIG    0x01
#define LM75_TEMP      0x00


volatile uint32_t numPwmCycles;
volatile int pwmAborted;
volatile int pwmPhase;

void SCT_IRQHandler (void)
{
	uint32_t status = LPC_SCT->EVFLAG;

	if (status & (1u << SCT_IRQ_EVENT_IRQ_cycle)) {
		/* Interrupt once per PWM cycle */
		++numPwmCycles;
	}

	if (status & (1u << SCT_IRQ_EVENT_IRQ_abort)) {
		/* Abort interrupt */
		pwmAborted = 1;
	}

	/* Acknowledge interrupts */
	LPC_SCT->EVFLAG = status;
}

/*****************************************************************************
** Function name:		SEEPROMTest
**
** Descriptions:		Serial EEPROM(Atmel 25xxx) test
**
** parameters:			port #
** Returned value:		None
**
*****************************************************************************/
void SPI_SEEPROMTest( LPC_SPI_TypeDef *SPIx, SLAVE_t slave )
{
  uint32_t i, now;

  /* Set write enable latch */
  src_addr[0] = WREN;			/* set write enable latch */
  SPI_Send( SPIx, slave, (uint8_t *)src_addr, 1 );
  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */

  src_addr[0] = RDSR;	/* check status to see if write enabled is latched */
  src_addr[1] = 0x55;	/* Dummy byte for read. */
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)dest_addr, 2 );
#if 1
  if ( (dest_addr[1] & (RDSR_WEN|RDSR_RDY)) != RDSR_WEN )
  /* bit 0 to 0 is ready, bit 1 to 1 is write enable */
  {
		while ( 1 );
  }
#endif

  if ((dest_addr[1] & (0x0C)) != 0)
  {
	  for ( i = 0; i < 0x80; i++ );	/* delay minimum 250ns */
	  src_addr[0] = WRSR;
	  src_addr[1] = 0x00;				/* Make the whole device unprotected. */
	  SPI_Send( SPIx, slave, (uint8_t *)src_addr, 2 );

	  //wait at least 5 ms
	  now = mrt_counter;
	  while ((mrt_counter - now) < 6)
		;
  }

  //read string
  for ( i = 0; i < 0x80; i++ );	/* delay, minimum 250ns */
  src_addr[0] = READ;		/* Read command is 0x03 */
  src_addr[1] = 0x00;		/* Read address offset is 0x00 */
  SPI_SendRcv( SPIx, slave, (uint8_t *)src_addr, (uint8_t *)&dest_addr, 16 );

  //check if string is incorrect (which it is for all non-zero values)
  if (memcmp(&dest_addr[2], (void *)"Stored string", 14) != 0)
  {
    for ( i = 0; i < 0x80; i++ );	/* delay, minimum 250ns */
    src_addr[0] = WREN;			/* set write enable latch */
    SPI_Send( SPIx, slave, (uint8_t *)src_addr, 1 );
    for ( i = 0; i < 0x80; i++ );	/* delay, minimum 250ns */

    //write string
    src_addr[0]  = WRITE;   /* Write command */
    src_addr[1]  = 0x00;    /* write address offset is 0x00 */
    src_addr[2]  = 'S';
    src_addr[3]  = 't';
    src_addr[4]  = 'o';
    src_addr[5]  = 'r';
    src_addr[6]  = 'e';
    src_addr[7]  = 'd';
    src_addr[8]  = ' ';
    src_addr[9]  = 's';
    src_addr[10] = 't';
    src_addr[11] = 'r';
    src_addr[12] = 'i';
    src_addr[13] = 'n';
    src_addr[14] = 'g';
    src_addr[15] = 0;
    SPI_Send( SPIx, slave, (uint8_t *)src_addr, 16 );

    //wait at least 5 ms
    now = mrt_counter;
    while ((mrt_counter - now) < 6)
      ;
  }

  return;
}

int main(void) {
    uint32_t regVal;
	uint32_t lastCycles;

    SystemCoreClockUpdate();

    /***************************************************************************
     * Init UART#0 (115200bps, 8N1) and send welcome message
     */
    UARTInit(LPC_USART0, 115200);

    // P0.4 is UART0 TX
    LPC_IOCON->PIO0_4 &= ~(0x3 << 3);
	regVal = LPC_SWM->PINASSIGN0 & ~( (0xFF << 0) | (0x00 << 8) );
	LPC_SWM->PINASSIGN0 = regVal | ( (0 << 0));         /* P0.0 is UART0 TX, ASSIGN(7:0) */

	UARTSend(LPC_USART0, (uint8_t *)welcomeStr, sizeof(welcomeStr)-1);

    /***************************************************************************
     * Config CLKOUT on PIO0_6
     */
  	regVal = LPC_SWM->PINASSIGN8 & ~( 0xFF << 16 );
  	LPC_SWM->PINASSIGN8 = regVal | ( 6 << 16 );	/* P0.6 is CLKOUT, ASSIGN(23:16). */
    CLKOUT_Setup( CLKOUTCLK_SRC_MAIN_CLK );
    LPC_SYSCON->CLKOUTDIV = 12;			/* Divided by 12 = 2 MHz from the 24 MHz core clock */

    /***************************************************************************
     * Initialize SPI pin connect
     */
    regVal = LPC_SWM->PINASSIGN3 & ~( 0xFFUL<<24 );
    LPC_SWM->PINASSIGN3 = regVal | ( 0x11UL<<24 );      /* P0.17 is SCK, ASSIGN3(31:24) */
    regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<0 );
    LPC_SWM->PINASSIGN4 = regVal | ( 0xf<<0 );          /* P0.15 is MOSI. ASSIGN4(7:0) */
    regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<8 );
    LPC_SWM->PINASSIGN4 = regVal | ( 0x10<<8 );         /* P0.16 is MISO. ASSIGN4(15:8) */
    regVal = LPC_SWM->PINASSIGN4 & ~( 0xFF<<16 );
    LPC_SWM->PINASSIGN4 = regVal | ( 14<<16 );          /* P0.14 is SSEL. ASSIGN4(23:16) */
    SPI_Init(LPC_SPI0, 24, CFG_MASTER, DLY_PREDELAY(0x0)|DLY_POSTDELAY(0x0)|DLY_FRAMEDELAY(0x0)|DLY_INTERDELAY(0x0));

    /***************************************************************************
     * Initialize I2C pin connect
     */
    regVal = LPC_SWM->PINASSIGN7 & ~(0xFFUL<<24);
    LPC_SWM->PINASSIGN7 = regVal | (10 << 24);          /* P0.10 is I2C SDA, ASSIGN0(31:24) */
    regVal = LPC_SWM->PINASSIGN8 & ~(0xFF<<0);
    LPC_SWM->PINASSIGN8 = regVal | (11 << 0);           /* P0.11 is I2C SCL. ASSIGN0(7:0) */
    regVal = LPC_IOCON->PIO0_10 & ~(0x3<<8);
    LPC_IOCON->PIO0_10 = regVal | (0x0<<8);             /* Enable I2C */
    regVal = LPC_IOCON->PIO0_11 & ~(0x3<<8);
    LPC_IOCON->PIO0_11 = regVal | (0x0<<8);             /* Enable I2C */

    /* Enable I2C clock */
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);

    /* Toggle peripheral reset control to I2C, a "1" bring it out of reset. */
    LPC_SYSCON->PRESETCTRL &= ~(0x1<<6);
    LPC_SYSCON->PRESETCTRL |= (0x1<<6);

#if 1
    /* Ready to receive the slave address, interrupt(SLAVE ADDR STATE) if match occurs. */
    LPC_I2C->SLVCTL = CTL_SLVCONTINUE;
  #if I2C_INTERRUPT
    LPC_I2C->INTENSET = STAT_SLVPEND;
  #endif

    /* For master mode plus, if desired I2C clock is 1MHz (SCL high time + SCL low time).
  	If CCLK is 36MHz, MasterSclLow and MasterSclHigh are 0s,
  	SCL high time = (ClkDiv+1) * (MstSclHigh + 2 )
  	SCL low time = (ClkDiv+1) * (MstSclLow + 2 )
  	Pre-divider should be 36000000/(1000000*4)-1 = 9-1 = 8.
  	If fast mode, e.g. communicating with a temp sensor, Max I2C clock is set to 400KHz.
  	Pre-divider should be 36000000/(400000*4)-1 = 22.5-1 = 22.
  	If standard mode, e.g. communicating with a temp sensor, Max I2C clock is set to 100KHz.
  	Pre-divider should be 36000000/(100000*4)-1 = 90-1 = 89. */
    I2C_MstInit(LPC_I2C, I2C_FMODE_PRE_DIV, CFG_MSTENA, 0x00);
    I2C_CheckIdle(LPC_I2C);
#endif

	/***************************************************************************
     * Configure the switch matrix (connecting SCT outputs to the green LED)
     */
	SwitchMatrix_Init();
    LPC_IOCON->PIO0_7 = 0x90;
//    LPC_IOCON->PIO0_12 = 0x90;
//    LPC_IOCON->PIO0_13 = 0x90;

	// enable the SCT clock
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 8);

	// clear peripheral reset the SCT:
	LPC_SYSCON->PRESETCTRL |= ( 1<< 8);

	// Initialize it:
	sct_fsm_init();

	/* Conflict resolution: Inactive state takes precedence.
	 * CTOUT_0, CTOUT_1: Inactive state 0
	 * CTOUT_2, CTOUT_3: Inactive state 1
     */
	LPC_SCT->RES = 0x0000005A;

	NVIC_EnableIRQ(SCT_IRQn);

	// unhalt it: - clearing bit 2 of the CTRL register
	LPC_SCT->CTRL_L &= ~( 1<< 2  );

	lastCycles = numPwmCycles;

	/***************************************************************************
     * Config GPIO
     */
    /* Enable AHB clock to the GPIO domain. */
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

    /***************************************************************************
     * Setup clock for 1ms ticks
     */
  	init_mrt(24000);

    // Enter an infinite loop
    while(1)
    {
    	if (numPwmCycles != lastCycles)
    	{
    		lastCycles = numPwmCycles;
    		/* Every few PWM cycles change the duty cycles */
//			if ((lastCycles % 10) == 0) {
    		if (1) {
				/* TODO: Access to match registers will change in future tool release */

				/* Prevent the reload registers from being used before we have updated
				 * all PWM channels.
				 */
				LPC_SCT->CONFIG |= (1u << 7);	/* NORELOAD_L (U) */

				//check if time to update pwm
                if(1)
                {
                	uint16_t region;
                	region = mrt_counter % 2200;

                	if (region < 200)
                	{
                		//all colors off
                		reload_pwm_val1(PWM_OFF);  //red   999=off, 0=on
                		reload_pwm_val2(PWM_OFF);  //blue  999=off, 0=on
                		reload_pwm_val3(PWM_OFF);  //green 999=off, 0=on
                	}
                	else if (region < 1200)
                	{
                		reload_pwm_val1(PWM_OFF - (region - 200));
                	}
                	else if (region < 2200)
                	{
                		reload_pwm_val1(region - 1200);
                	}
                }
				if (0)
//				if ((mrt_counter % 10) == 0)
				{
					uint16_t region;
					uint8_t color;
					uint16_t valueRB, valueG;

					//check which region
					region = (mrt_counter*10) % 1400;
					color  = ((mrt_counter*10) % 9800) / 1400;
					if (region < 200)
					{
						//all colors off
						reload_pwm_val1(PWM_OFF);  //red   999=off, 0=on
						reload_pwm_val2(PWM_OFF);  //blue  999=off, 0=on
						reload_pwm_val3(PWM_OFF);  //green 999=off, 0=on
					}
					else if ((region >= 200) && (region < 800))
					{
						valueRB = PWM_OFF - ((region - 200) * 1);
						valueG  = PWM_OFF - ((region - 200) / 2);
						switch (color)
						{
						case 0: reload_pwm_val1(valueRB); reload_pwm_val2(PWM_OFF); reload_pwm_val3(PWM_OFF); break;
						case 1: reload_pwm_val1(PWM_OFF); reload_pwm_val2(valueRB); reload_pwm_val3(PWM_OFF); break;
						case 2: reload_pwm_val1(PWM_OFF); reload_pwm_val2(PWM_OFF); reload_pwm_val3(valueG);  break;
						case 3: reload_pwm_val1(valueRB); reload_pwm_val2(valueRB); reload_pwm_val3(PWM_OFF); break;
						case 4: reload_pwm_val1(valueRB); reload_pwm_val2(PWM_OFF); reload_pwm_val3(valueG);  break;
						case 5: reload_pwm_val1(PWM_OFF); reload_pwm_val2(valueRB); reload_pwm_val3(valueG);  break;
						case 6: reload_pwm_val1(valueRB); reload_pwm_val2(valueRB); reload_pwm_val3(valueG);  break;
						default: break;
						}
					}
					else
					{
						valueRB = 399 + ((region - 800) * 1);
						valueG  = 699 + ((region - 800) / 2);
						switch (color)
						{
						case 0: reload_pwm_val1(valueRB); reload_pwm_val2(PWM_OFF); reload_pwm_val3(PWM_OFF); break;
						case 1: reload_pwm_val1(PWM_OFF); reload_pwm_val2(valueRB); reload_pwm_val3(PWM_OFF); break;
						case 2: reload_pwm_val1(PWM_OFF); reload_pwm_val2(PWM_OFF); reload_pwm_val3(valueG);  break;
						case 3: reload_pwm_val1(valueRB); reload_pwm_val2(valueRB); reload_pwm_val3(PWM_OFF); break;
						case 4: reload_pwm_val1(valueRB); reload_pwm_val2(PWM_OFF); reload_pwm_val3(valueG);  break;
						case 5: reload_pwm_val1(PWM_OFF); reload_pwm_val2(valueRB); reload_pwm_val3(valueG);  break;
						case 6: reload_pwm_val1(valueRB); reload_pwm_val2(valueRB); reload_pwm_val3(valueG);  break;
						default: break;
						}
					}
				}
				/* Update done */
				LPC_SCT->CONFIG &= ~(1u << 7);	/* NORELOAD_L (U) */
			}
        }

    	//Do once a second
    	if ((mrt_counter % 1000) == 0)
//    	if ((mrt_counter % 100) == 0)
    	{
        	uint32_t temp;

        	//Communicate with SPI E2PROM
        	SPI_SEEPROMTest(LPC_SPI0, CS_USED);
#if 1
            /* Test master send and receive with repeated start */
            I2CMasterTXBuffer[0] = LM75_CONFIG;
            I2CMasterTXBuffer[1] = 0x00;		/* configuration value, no change from default */
            I2C_MstSend( LPC_I2C, LM75_ADDR, (uint8_t *)I2CMasterTXBuffer, 2 );

            /* Get temp reading */
            I2CMasterTXBuffer[0] = LM75_TEMP;
            I2C_MstSendRcv( LPC_I2C, LM75_ADDR, (uint8_t *)I2CMasterTXBuffer, 1, (uint8_t *)I2CMasterRXBuffer, 2 );
            /* The temp reading value should reside in I2CMasterRXBuffer... */
#endif

            temp = ((I2CMasterRXBuffer[0] << 8) | (I2CMasterRXBuffer[1]));
            temp = ((temp * 100) >> 8);
            welcomeStr[20] = '0' + ((temp / 1000) % 10);
            welcomeStr[21] = '0' + ((temp /  100) % 10);
            welcomeStr[23] = '0' + ((temp /   10) % 10);
            UARTSend(LPC_USART0, (uint8_t *)welcomeStr, sizeof(welcomeStr)-1);
    	}
if (0)
    	{
    		uint32_t regVal;
    		PMU_Init();
    		regVal = LPC_SYSCON->PDSLEEPCFG;
    		//regVal &= ~(WDT_OSC_PD | BOD_PD);
    		PMU_Sleep( MCU_SLEEP, regVal );
    	}

    }
    return 0 ;
}
