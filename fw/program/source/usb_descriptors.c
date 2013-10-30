/*!
 * @file
 * @brief   Descriptors for the USB stack
 * @ingroup IP_USB
 * @ingroup FUNC_COMM
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


/** \file
 *
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */

#include "usb_descriptors.h"

/* On some devices, there is a factory set internal serial number which can be automatically sent to the host as
 * the device's serial number when the Device Descriptor's .SerialNumStrIndex entry is set to USE_INTERNAL_SERIAL.
 * This allows the host to track a device across insertions on different ports, allowing them to retain allocated
 * resources like COM port numbers and drivers. On demos using this feature, give a warning on unsupported devices
 * so that the user can supply their own serial number descriptor instead or remove the USE_INTERNAL_SERIAL value
 * from the Device Descriptor (forcing the host to generate a serial number for each device from the VID, PID and
 * port location).
 */

/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
  .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

  .USBSpecification       = VERSION_BCD(02.00),
  .Class                  = USB_CSCP_NoDeviceClass,
  .SubClass               = USB_CSCP_NoDeviceSubclass,
  .Protocol               = USB_CSCP_NoDeviceProtocol,

  .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

  .VendorID               = 0x1fc9, /* NXP */
  .ProductID              = 0x0018, /* LabTool */
  .ReleaseNumber          = VERSION_BCD(01.00),

  .ManufacturerStrIndex   = 0x01,
  .ProductStrIndex        = 0x02,
  .SerialNumStrIndex      = USE_INTERNAL_SERIAL,

  .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
  .Config =
  {
    .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

    .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t) - 1, /* B.VERNOUX Last byte LabTool_Termination shall be not sent remove 1, v0.97 used sizeof(...)-1 */
    .TotalInterfaces        = 1,

    .ConfigurationNumber    = 1,
    .ConfigurationStrIndex  = NO_DESCRIPTOR,

    .ConfigAttributes       = USB_CONFIG_ATTR_BUSPOWERED,

    .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
  },

  .LabTool_Interface =
  {
    .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

    .InterfaceNumber        = LABTOOL_IF_NUMBER,
    .AlternateSetting       = 0,

    .TotalEndpoints         = 2,

    .Class                  = USB_CSCP_VendorSpecificClass,
    .SubClass               = USB_CSCP_VendorSpecificSubclass,
    .Protocol               = USB_CSCP_VendorSpecificProtocol,

    .InterfaceStrIndex      = NO_DESCRIPTOR
  },

  .LabTool_DataInEndpoint =
  {
    .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

    .EndpointAddress        = (ENDPOINT_DIR_IN | LABTOOL_IN_EPNUM),
    .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
    .EndpointSize           = LABTOOL_IO_EPSIZE,
    .PollingIntervalMS      = 0x01
  },

  .LabTool_DataOutEndpoint =
  {
    .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

    .EndpointAddress        = (ENDPOINT_DIR_OUT | LABTOOL_OUT_EPNUM),
    .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
    .EndpointSize           = LABTOOL_IO_EPSIZE,
    .PollingIntervalMS      = 0x01
  },
  .LabTool_Termination = 0x00
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
uint8_t LanguageString[] =
{
  USB_STRING_LEN(1),
  DTYPE_String,
  WBVAL(LANGUAGE_ID_ENG),
};
USB_Descriptor_String_t *LanguageStringPtr = (USB_Descriptor_String_t*)LanguageString;

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
uint8_t ManufacturerString[] =
{
  USB_STRING_LEN(16),
  DTYPE_String,
  WBVAL('E'),
  WBVAL('m'),
  WBVAL('b'),
  WBVAL('e'),
  WBVAL('d'),
  WBVAL('d'),
  WBVAL('e'),
  WBVAL('d'),
  WBVAL(' '),
  WBVAL('A'),
  WBVAL('r'),
  WBVAL('t'),
  WBVAL('i'),
  WBVAL('s'),
  WBVAL('t'),
  WBVAL('s'),
};
USB_Descriptor_String_t *ManufacturerStringPtr = (USB_Descriptor_String_t*)ManufacturerString;

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
uint8_t ProductString[] =
{
  USB_STRING_LEN(7),
  DTYPE_String,
  WBVAL('L'),
  WBVAL('a'),
  WBVAL('b'),
  WBVAL('T'),
  WBVAL('o'),
  WBVAL('o'),
  WBVAL('l'),
};
USB_Descriptor_String_t *ProductStringPtr = (USB_Descriptor_String_t*)ProductString;

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
  const uint8_t  DescriptorType   = (wValue >> 8);
  const uint8_t  DescriptorNumber = (wValue & 0xFF);

  const void* Address = NULL;
  uint16_t    Size    = NO_DESCRIPTOR;

  switch (DescriptorType)
  {
    case DTYPE_Device:
      Address = &DeviceDescriptor;
      Size    = sizeof(USB_Descriptor_Device_t);
      break;
    case DTYPE_Configuration:
      Address = &ConfigurationDescriptor;
      Size    = sizeof(USB_Descriptor_Configuration_t) - 1; /* B.VERNOUX Last byte LabTool_Termination shall be not sent */
      break;
    case DTYPE_String:
      switch (DescriptorNumber)
      {
        case 0x00:
          Address = LanguageStringPtr;
          Size    = pgm_read_byte(&LanguageStringPtr->Header.Size);
          break;
        case 0x01:
          Address = ManufacturerStringPtr;
          Size    = pgm_read_byte(&ManufacturerStringPtr->Header.Size);
          break;
        case 0x02:
          Address = ProductStringPtr;
          Size    = pgm_read_byte(&ProductStringPtr->Header.Size);
          break;
      }
      break;
  }

  *DescriptorAddress = Address;
  return Size;
}

