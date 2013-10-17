/*!
 * @file
 * @brief   Configuration of the SwitchMatrix module.
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
    
#include "LPC8xx.h"    /* LPC8xx Peripheral Registers */


void SwitchMatrix_Init() 
{ 
	uint32_t regVal;

	/* Pin Assign 8 bit Configuration */
    /* CTIN_0 */
    regVal = LPC_SWM->PINASSIGN5 & ~(0xFF<<24);
    LPC_SWM->PINASSIGN5 = regVal | (0x01 << 24);	// CTIN_0 -> PIO0_1
    /* CTOUT_0 */
    regVal = LPC_SWM->PINASSIGN6 & ~(0xFF<<24);
    LPC_SWM->PINASSIGN6 = regVal | (0x07 << 24);	// CTOUT_0 -> PIO0_7
    /* CTOUT_1, CTOUT_2, CTOUT_3 */
//    regVal = LPC_SWM->PINASSIGN7 & ~(0xFF<<0);
//    LPC_SWM->PINASSIGN7 = regVal | (0x0c << 0);	// CTOUT_1 -> PIO0_12
//    regVal = LPC_SWM->PINASSIGN7 & ~(0xFF<<8);
//    LPC_SWM->PINASSIGN7 = regVal | (0x0d << 8);	// CTOUT_2 -> PIO0_13

    //    regVal = LPC_SWM->PINASSIGN7 & ~(0xFF<<16);
//    LPC_SWM->PINASSIGN7 = regVal | (0xff << 16);	// CTOUT_3 -> N/A
//    LPC_SWM->PINASSIGN7 = 0xffff0d0cUL;	// CTOUT_1 -> PIO0_12, CTOUT_2 -> PIO0_13, CTOUT_3 -> N/A */
}

 /**********************************************************************
 **                            End Of File
 **********************************************************************/
