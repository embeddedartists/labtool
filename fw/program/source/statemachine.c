/*!
 * @file
 * @brief   Statemachine. Handles resource allocation
 * @ingroup FUNC_STATE
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


/******************************************************************************
 * Includes
 *****************************************************************************/

#include "statemachine.h"
#include "capture.h"
#include "generator.h"
#include "calibrate.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/


/******************************************************************************
 * Global variables
 *****************************************************************************/


/******************************************************************************
 * Local variables
 *****************************************************************************/

static states_t currentState;

/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * External method
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes the state machine
 *
 *****************************************************************************/
void statemachine_Init(void)
{
  currentState = STATE_INIT;
}

/**************************************************************************//**
 *
 * @brief  Changes to the new state
 *
 * Attempts to change to the new state by ending all activities in the old
 * state and then initializing activities belonging to the new state.
 *
 *  \dot
 *  digraph example {
 *      node [shape=oval, fontname=Helvetica, fontsize=10];
 *      init [ label="INIT", shape="doublecircle" ];
 *      idle [ label="Idle" ];
 *      cap [ label="Signal Capturing" URL="\ref capture.c"];
 *      cal [ label="Calibration" URL="\ref calibrate.c"];
 *      gen [ label="Signal Generation" URL="\ref generator.c"];
 *      mon [ label="I2C Monitoring" URL="\ref monitor_i2c.c", style="dotted" ];
 *      init -> idle [ arrowhead="open", style="solid" ];
 *      idle -> cap [ arrowhead="open", arrowtail="open", style="solid", dir="both" ];
 *      idle -> gen [ arrowhead="open", style="solid", dir="both" ];
 *      idle -> mon [ arrowhead="open", style="solid", dir="both" ];
 *      idle -> cal [ arrowhead="open", style="solid", dir="both" ];
 *      cap -> gen [ arrowhead="open", style="solid", dir="both" ];
 *      cap -> mon [ arrowhead="open", style="dotted", dir="both" ];
 *      cap -> cal [ arrowhead="open", style="dotted", dir="both" ];
 *      gen -> mon [ arrowhead="open", style="dotted", dir="both" ];
 *      gen -> cal [ arrowhead="open", style="dotted", dir="both" ];
 *      mon -> cal [ arrowhead="open", style="dotted", dir="both" ];
 *  }
 *  \enddot
 *
 *
 * @param [in] newState  The requested state
 *
 * @retval CMD_STATUS_OK   If the state changed or was correct already
 * @retval CMD_STATUS_ERR_NO_SUCH_STATE  If the \a newState is invalid
 * @retval CMD_STATUS_ERR_*  If the stop function(s) of the old state failed
 *
 *****************************************************************************/
cmd_status_t statemachine_RequestState(states_t newState)
{
  cmd_status_t result = CMD_STATUS_OK;

  /* No change */
  if (newState == currentState)
  {
    return CMD_STATUS_OK;
  }

  /* Special handling of STATE_CAPTURING while calibrating */
  if ((currentState == STATE_CALIBRATING) && (newState == STATE_CAPTURING))
  {
    /* Let the capturing believe that the state has changed */
    return CMD_STATUS_OK;
  }

  /* Stop current activities */
  switch (currentState)
  {
    case STATE_INIT:
    case STATE_IDLE:
      /* nothing started so there is nothing to stop... */
      break;

    case STATE_CAPTURING:
      result = capture_Disarm();
      break;

    case STATE_GENERATING:
      result = generator_Stop();
      break;

    case STATE_CALIBRATING:
      /* only started thing is the capturing for anlog in calibration */
      capture_Disarm();
      break;

    default:
      result = CMD_STATUS_ERR_NO_SUCH_STATE;
      break;
  }

  /* Prepare new activities */
  if (result == CMD_STATUS_OK)
  {
    switch (newState)
    {
      case STATE_IDLE:
        /* nothing to prepare... */
        break;

      case STATE_CAPTURING:
        capture_Init();
        break;

      case STATE_GENERATING:
        generator_Init();
        break;

      case STATE_CALIBRATING:
        result = calibrate_Init();
        break;

      default:
        result = CMD_STATUS_ERR_NO_SUCH_STATE;
        break;
    }
  }

  if (result == CMD_STATUS_OK)
  {
    currentState = newState;
  }

  return result;
}

/**************************************************************************//**
 *
 * @brief  Returns the current state
 *
 * @return Current state
 *
 *****************************************************************************/
states_t statemachine_GetState(void)
{
  return currentState;
}

