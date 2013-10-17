/*!
 * @file
 * @brief     Log functionality to aid debugging
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
#ifndef __LOG_H
#define __LOG_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdio.h>
#include "lpc_types.h"
#include "debug_frmwrk.h"
#include "labtool_config.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#if (ENABLE_LOGGING == OPT_ENABLED)

/*!
 * @brief Prints a debug message message
 *
 * The debug message includes information about which function the log
 * message originates in.
 *
 * Usage:
 *     log_d("Hello World\r\n");        // called from main()
 *     log_d("Found value %d\r\n", 19); // called from configure()
 *
 * Outputs:
 *
 *     main(): Hello World
 *     configure(): Found value 19
 */
#define log_d(...) do {\
    _DBG(__FUNCTION__); \
    _DBG(": "); \
    lpc_printf(__VA_ARGS__); \
    _DBG_(""); \
  } while(0)

/*!
 * @brief Prints an informational message
 *
 * Usage:
 *     log_i("Hello World\r\n");
 *     log_i("Found interesting value %d\r\n", 42);
 *
 * Outputs:
 *
 *     Hello World
 *     Found interesting value 42
 */
#define log_i(...) do {\
    lpc_printf(__VA_ARGS__); \
  } while(0)

/*!
 * @brief Prints an error message
 *
 * Usage:
 *     log_e("Hello World\r\n");
 *     log_e("Found strange value %d\r\n", 42);
 *
 * Outputs:
 *
 *     SYS_ERROR: Hello World
 *     SYS_ERROR: Found strange value 42
 */
#define log_e(...) do {\
    _DBG("SYS_ERROR: "); \
    lpc_printf(__VA_ARGS__); \
  } while(0)

/*!
 * @brief Helps when printing registers and their values.
 *
 * Usage:
 *     LOG_REG(LPC_VADC->CONFIG);
 *     LOG_REG(LPC_VADC->LAST_SAMPLE[0]);
 *
 * Outputs:
 *
 *     Reg 0x400f001c LPC_VADC->CONFIG = 0x00002421
 *     Reg 0x400f0028 LPC_VADC->LAST_SAMPLE[0] = 0x00000000
 */
#define LOG_REG(__x) log_i("Reg 0x%08x " #__x " = 0x%08x\r\n", (uint32_t)(&(__x)), __x)

#else
  #define log_d(...)
  #define log_i(...)
  #define log_e(...)
  #define LOG_REG(__x)
#endif

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void display_buffer_hex(const unsigned char * const buffer, unsigned size);

#endif /* end __LOG_H */

