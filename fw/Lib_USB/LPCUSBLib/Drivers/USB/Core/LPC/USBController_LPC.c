/*
* Copyright(C) NXP Semiconductors, 2011
* All rights reserved.
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* LPC products.  This software is supplied "AS IS" without any warranties of
* any kind, and NXP Semiconductors and its licensor disclaim any and 
* all warranties, express or implied, including all implied warranties of 
* merchantability, fitness for a particular purpose and non-infringement of 
* intellectual property rights.  NXP Semiconductors assumes no responsibility
* or liability for the use of the software, conveys no license or rights under any
* patent, copyright, mask work right, or any other intellectual property rights in 
* or to any products. NXP Semiconductors reserves the right to make changes
* in the software without notification. NXP Semiconductors also makes no 
* representation or warranty that such application will be suitable for the
* specified use without further testing or modification.
* 
* Permission to use, copy, modify, and distribute this software and its 
* documentation is hereby granted, under NXP Semiconductors' and its 
* licensor's relevant copyrights in the software, without fee, provided that it 
* is used in conjunction with NXP Semiconductors microcontrollers.  This 
* copyright, permission, and disclaimer notice must appear in all copies of 
* this code.
*/


#define  __INCLUDE_FROM_USB_DRIVER
#define  __INCLUDE_FROM_USB_CONTROLLER_C
#include "../USBController.h"

#if (!defined(USB_HOST_ONLY) && !defined(USB_DEVICE_ONLY))
volatile uint8_t USB_CurrentMode = USB_MODE_None;
#endif

uint8_t USBPortNum;

void USB_Init(void)
{
#if defined(USB_MULTI_PORTS)
	uint8_t i;
	USB_Memory_Init(USBRAM_BUFFER_SIZE);
	for(i = 0;i<MAX_USB_CORE;i++)
	{
		HAL_USBInit(i);
		USB_ResetInterface(i);
	}
#else
	USBPortNum = (uint8_t)USB_PORT_SELECTED;
	HAL_USBInit(USBPortNum);
	#if defined(USB_CAN_BE_HOST) 
	USB_Memory_Init(USBRAM_BUFFER_SIZE);
	#endif
	USB_ResetInterface(USBPortNum);
#endif
	USB_IsInitialized = true;
}

void USB_Disable(void)
{
	USB_IsInitialized = false;
	if (USB_CurrentMode == USB_MODE_Device)
	{
		#if defined(USB_CAN_BE_DEVICE)
		HAL_USBConnect(USBPortNum,0);
		HAL_USBDeInit(USBPortNum);
		#endif
	}
	if (USB_CurrentMode == USB_MODE_Host)
	{
		#if defined(USB_CAN_BE_HOST)

		#if defined(USB_MULTI_PORTS)
			uint8_t i;
			for(i=0;i<MAX_USB_CORE;i++)
			{
				HcdDeInitDriver(i);
				HAL_USBDeInit(i);
			}
		#else
			HcdDeInitDriver(USBPortNum);
			HAL_USBDeInit(USBPortNum);
		#endif

		#endif
	}
}

void USB_ResetInterface(uint8_t corenum)
{
	if (USB_CurrentMode == USB_MODE_Device)
	{
		#if defined(USB_CAN_BE_DEVICE)
		USB_Init_Device(corenum);
		#endif
	}
	else if (USB_CurrentMode == USB_MODE_Host)
	{
		#if defined(USB_CAN_BE_HOST)
		USB_Init_Host(corenum);
		#endif
	}
}

#if defined(USB_CAN_BE_DEVICE)
static void USB_Init_Device(uint8_t corenum)
{
	USB_DeviceState          = DEVICE_STATE_Unattached;
	USB_Device_ConfigurationNumber  = 0;

	#if !defined(NO_DEVICE_REMOTE_WAKEUP)
	USB_Device_RemoteWakeupEnabled  = false;
	#endif

	#if !defined(NO_DEVICE_SELF_POWER)
	USB_Device_CurrentlySelfPowered = false;
	#endif

	#if defined(USB_DEVICE_ROM_DRIVER)
		UsbdRom_Init();
	#else
		Endpoint_ConfigureEndpoint(ENDPOINT_CONTROLEP, EP_TYPE_CONTROL,
							   	   ENDPOINT_DIR_OUT, USB_Device_ControlEndpointSize,
							   	   ENDPOINT_BANK_SINGLE);
	#endif
	HAL_EnableUSBInterrupt(USBPortNum);
	HAL_USBConnect(USBPortNum, 1);
}
#endif

#if defined(USB_CAN_BE_HOST)
static void USB_Init_Host(uint8_t corenum)
{
	//uint8_t i;

	//for(i=0;i<PIPE_TOTAL_PIPES;i++) PipeInfo[i].PipeHandle=0;

	pipeselected[corenum] = PIPE_CONTROLPIPE;

	if(HcdInitDriver(corenum)==HCD_STATUS_OK)
	{
		USB_IsInitialized = true;
		HAL_EnableUSBInterrupt(corenum);
	}
	else
	{
		USB_IsInitialized = false;
		HcdDeInitDriver(corenum);
	}

	USB_HostState[corenum]   = HOST_STATE_Unattached;
	USB_Host_ControlPipeSize[corenum] = PIPE_CONTROLPIPE_DEFAULT_SIZE;
}
#endif

