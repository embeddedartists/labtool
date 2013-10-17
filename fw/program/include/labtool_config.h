/*!
 * @file
 * @brief     Configuration of enabled functionality
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
#ifndef __LABTOOL_CONFIG_H
#define __LABTOOL_CONFIG_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Disables the configuration option */
#define OPT_DISABLED   (0)

/*! @brief Enables the configuration option */
#define OPT_ENABLED    (1)

/*! @brief Run at above 200MHz.
 * When set the M4_CORE will be overclocked to 240MHz. When disabled the M4_CORE will be 200MHz.
 */
#define HYPERSPEED                   OPT_DISABLED

/*! @brief Enables macros for performance measuring.
 * Enables the macros in \ref meas.h to control some GPIO pins.
 */
#define ENABLE_MEASSURING            OPT_DISABLED

/*! @brief Test basic functionality of the SPI EEPROM.
 */
#define TEST_SPI_EEPROM              OPT_DISABLED

/*! @brief Test to read/store calibration data in EEPROM
 */
#define TEST_CALIB_DATA_STORE_LOAD   OPT_DISABLED

/*! @brief Uses the State Configurable Timer (SCT) to determine the frequency of one
 * of the digital inputs.
 */
#define TEST_SCT_FREQUENCY_COUNTER   OPT_DISABLED

/*! @brief Records all activity on the I2C bus.
 * @todo Enable I2C Monitoring in the client application
 */
#define TEST_I2C_MONITOR             OPT_DISABLED

/*! @brief Debug aid.
 * Prints histogram information about the enabled analog channels before
 * sending the data to the client.
 */
#define PRINT_ANALOG_HISTOGRAM       OPT_DISABLED

/*! @brief Debug aid.
 * Prints min/max/avg statistics for each channel before sending the data
 * to the client.
 */
#define PRINT_STATISTICS             OPT_DISABLED

/*! @brief Debug aid.
 * When sampling both channels it is expected that the number of samples from
 * each channel is the same. Before data containing samples from both analog
 * channels is sent to the client it is search for "skipped samples".
 */
#define FIND_SKIPPED_SAMPLES         OPT_DISABLED

/*! @brief Debug aid.
 * As there is no USB uart available for printing, the project uses the Trace 
 * functionallity by remapping the UARTPutChar() function into ITM_SendChar()
 * in debug_frmwrk.c. The printouts are only available when debugging so this
 * option makes it possible to completely remove all printouts which saves
 * ROM and some RAM.
 */
#define ENABLE_LOGGING               OPT_DISABLED

/*! @brief Verify configuration before attempting to apply it.
 * There are some limitations to what the hardware can and can't do. By enabling
 * this option those checks are made and if the hardware is likely to be
 * overpowered an error code is returned to the client instead of applying
 * the configuration. By disabling this option you are likely to get errors
 * in the client due to disconnected hardware. This occurs because the MCU is
 * so busy that there is no time to maintain the USB connection.
 *
 * Note: This options should ALWAYS be enabled in released software!
 */
#define DO_WEIGHTED_CONFIG_CHECK     OPT_ENABLED

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/


#endif /* end __LABTOOL_CONFIG_H */

