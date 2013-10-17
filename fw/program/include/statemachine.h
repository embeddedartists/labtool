/*!
 * @file
 * @brief     Statemachine. Handles resource allocation
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
#ifndef __STATEMACHINE_H
#define __STATEMACHINE_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "error_codes.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! States.
 * @see statemachine_RequestState
 */
typedef enum
{
  STATE_INIT,        /*!< Start point */
  STATE_IDLE,        /*!< Noting ongoing */
  STATE_CAPTURING,   /*!< Capturing of analog and/or digital signals ongoing */
  STATE_GENERATING,  /*!< Generation of analog and/or digital signals ongoing */
  STATE_CALIBRATING, /*!< Calibration of analog in/out signals ongoing */
} states_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

void statemachine_Init(void);
cmd_status_t statemachine_RequestState(states_t newState);
states_t statemachine_GetState(void);

#endif /* end __STATEMACHINE_H */

