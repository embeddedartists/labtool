/*!
 * @file
 * @brief     Descriptors for the USB stack
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
#ifndef __DESCRIPTORS_H
#define __DESCRIPTORS_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "USB.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Endpoint number of the LabTool device-to-host data IN endpoint. */
#define LABTOOL_IN_EPNUM          5

/*! Endpoint number of the LabTool host-to-device data OUT endpoint. */
#define LABTOOL_OUT_EPNUM         2

/*! Size in bytes of the LabTool data endpoints. */
#define LABTOOL_IO_EPSIZE         512

/*! Interface number for the LabTool interface */
#define LABTOOL_IF_NUMBER         0

/*! @brief USB descriptors for the LabTool device.
 *
 * Type define for the device configuration descriptor structure. This must be defined in the
 * application code, as the configuration descriptor contains several sub-descriptors which
 * vary between devices, and which describe the device's usage to the host.
 */
typedef struct
{
  USB_Descriptor_Configuration_Header_t Config;

  // LabTool Communication Interface
  USB_Descriptor_Interface_t      LabTool_Interface;
  USB_Descriptor_Endpoint_t       LabTool_DataInEndpoint;
  USB_Descriptor_Endpoint_t       LabTool_DataOutEndpoint;
  unsigned char                   LabTool_Termination;
} USB_Descriptor_Configuration_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif /* end __DESCRIPTORS_H */

