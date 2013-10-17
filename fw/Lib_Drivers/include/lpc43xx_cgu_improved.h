/*!
 * @file
 * @brief     Fixes to the broken or limited lpc43xx_cgu driver
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
#ifndef __LPC43XX_CGU_IMPROVED_H
#define __LPC43XX_CGU_IMPROVED_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc43xx_cgu.h"

/******************************************************************************
 * Functions
 *****************************************************************************/

CGU_ERROR CGU_Improved_SetPLL0audio(uint32_t msel, uint32_t nsel, uint32_t psel);
CGU_ERROR CGU_Improved_SetPLL1(uint32_t mult, uint32_t div);
void CGU_Improved_Init(void);

#endif /* end __LPC43XX_CGU_IMPROVED_H */
