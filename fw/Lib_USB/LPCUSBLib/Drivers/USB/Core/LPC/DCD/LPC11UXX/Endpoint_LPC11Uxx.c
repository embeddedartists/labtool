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
#include "../../../USBMode.h"

#if (defined(__LPC11UXX__) || defined(__LPC13UXX__)) && defined(USB_CAN_BE_DEVICE)
#include "../../../Endpoint.h"

#if defined(USB_DEVICE_ROM_DRIVER)

PRAGMA_ALIGN_256
uint8_t usb_RomDriver_buffer[ROMDRIVER_MEM_SIZE] ATTR_ALIGNED(256);
PRAGMA_ALIGN_4
uint8_t usb_RomDriver_MSC_buffer[ROMDRIVER_MSC_MEM_SIZE] ATTR_ALIGNED(4);
PRAGMA_ALIGN_4
uint8_t usb_RomDriver_CDC_buffer[ROMDRIVER_CDC_MEM_SIZE] ATTR_ALIGNED(4);
/** Endpoint IN buffer, used for DMA operation */
PRAGMA_ALIGN_4
uint8_t UsbdCdc_EPIN_buffer[CDC_MAX_BULK_EP_SIZE] ATTR_ALIGNED(4);
/** Endpoint OUT buffer, used for DMA operation */
PRAGMA_ALIGN_4
uint8_t UsbdCdc_EPOUT_buffer[CDC_MAX_BULK_EP_SIZE] ATTR_ALIGNED(4);
PRAGMA_ALIGN_4
uint8_t usb_RomDriver_HID_buffer[ROMDRIVER_HID_MEM_SIZE] ATTR_ALIGNED(4);

void USB_IRQHandler (void)
{
	UsbdRom_IrqHandler();
}

#else

#define IsOutEndpoint(PhysicalEP)		(! ((PhysicalEP) & 1) )
PRAGMA_ALIGN_256
/*static*/ USB_CMD_STAT EndPointCmdStsList[USED_PHYSICAL_ENDPOINTS][2] __DATA(USBRAM_SECTION) ATTR_ALIGNED(256) ; /* 10 endpoints with 2 buffers each */
PRAGMA_ALIGN_64
static uint8_t SetupPackage[8] __DATA(USBRAM_SECTION) ATTR_ALIGNED(64);
uint32_t EndpointMaxPacketSize[USED_PHYSICAL_ENDPOINTS];
uint32_t Remain_length[ENDPOINT_DETAILS_MAXEP];
bool shortpacket;
uint16_t stream_total_packets;
/********************************************************************//**
 * @brief
 * @param
 * @return
 *********************************************************************/
void HAL_Reset (void)
{
	LPC_USB->EPINUSE = 0;
	LPC_USB->EPSKIP = 0xFFFFFFFF;
	LPC_USB->EPBUFCFG = 0;
	
	LPC_USB->DEVCMDSTAT |= USB_EN;
	/* Clear all EP interrupts, device status, and SOF interrupts. */
	LPC_USB->INTSTAT = 0xC00003FF;
	/* Enable all ten(10) EPs interrupts including EP0, note: EP won't be
	ready until it's configured/enabled when device sending SetEPStatus command
	to the command engine. */
	LPC_USB->INTEN  = DEV_STAT_INT;

	/* Initialize EP Command/Status List. */
	memset(EndPointCmdStsList, 0, sizeof(EndPointCmdStsList) );
	LPC_USB->EPLISTSTART = (uint32_t) EndPointCmdStsList;
	LPC_USB->DATABUFSTART = ((uint32_t) usb_data_buffer) & 0xFFC00000;
	
	HAL_SetDeviceAddress(0);
	
	shortpacket = false;
	
	return;
}

bool Endpoint_ConfigureEndpointControl(const uint16_t Size)
{
	/* Endpoint Control OUT Buffer 0 */
	EndPointCmdStsList[0][0].BufferAddrOffset = 0;
	EndPointCmdStsList[0][0].NBytes = 0x200;
	EndPointCmdStsList[0][0].Active = 0;
	
	/* Setup Buffer */
	EndPointCmdStsList[0][1].BufferAddrOffset = ( ((uint32_t) SetupPackage) >> 6) & 0xFFFF;

	/* Endpoint Control IN Buffer 0 */
	EndPointCmdStsList[1][0].BufferAddrOffset = 0;
	EndPointCmdStsList[1][0].NBytes = 0;
	EndPointCmdStsList[1][0].Active = 0;

	// clear control endpoint interrupt status
	Endpoint_InterruptClear(0);
	Endpoint_InterruptClear(1);

	// enable control endpoints
	Endpoint_InterruptEnable(0);
	Endpoint_InterruptEnable(1);

	EndpointMaxPacketSize[0]=EndpointMaxPacketSize[1]=Size;

	DcdDataTransfer(ENDPOINT_CONTROLEP, usb_data_buffer, USB_DATA_BUFFER_TEM_LENGTH);
	return true;
}

/********************************************************************//**
 * @brief
 * @param
 * @return
 *********************************************************************/
bool Endpoint_ConfigureEndpoint(const uint8_t Number, const uint8_t Type,
								const uint8_t Direction, const uint16_t Size, const uint8_t Banks)
{
	uint32_t PhyEP = 2*Number + (Direction == ENDPOINT_DIR_OUT ? 0 : 1);
	
	memset(EndPointCmdStsList[PhyEP], 0, sizeof(USB_CMD_STAT)*2 );
	
	Endpoint_InterruptClear(PhyEP);
	Endpoint_InterruptEnable(PhyEP);

	EndpointMaxPacketSize[PhyEP] = Size;
	endpointhandle[Number] = (Number==ENDPOINT_CONTROLEP) ? ENDPOINT_CONTROLEP : PhyEP;

	if (IsOutEndpoint(PhyEP))
		DcdDataTransfer(PhyEP, usb_data_buffer_OUT, USB_DATA_BUFFER_TEM_LENGTH);
	else
		EndPointCmdStsList[PhyEP][0].NBytes = 0;

	return true;
}
/********************************************************************//**
 * @brief
 * @param
 * @return
 *********************************************************************/
void Endpoint_Streaming(uint8_t * buffer,uint16_t packetsize,
						uint16_t totalpackets,uint16_t dummypackets)
{
	uint8_t PhyEP = endpointhandle[endpointselected];
	uint16_t i;

	if(PhyEP&1)
	{
		for(i=0;i<totalpackets;i++){
			while(!Endpoint_IsReadWriteAllowed());
			Endpoint_Write_Stream_LE((void*)(buffer + i*packetsize), packetsize,NULL);
			Endpoint_ClearIN();
		}
		for(i=0;i<dummypackets;i++){
			while(!Endpoint_IsReadWriteAllowed());
			Endpoint_Write_Stream_LE((void*)buffer, packetsize,NULL);
			Endpoint_ClearIN();
		}
	}
	else
	{
		stream_total_packets = totalpackets + dummypackets;
		for(i=0;i<totalpackets;i++){
			DcdDataTransfer(PhyEP,(uint8_t*)(buffer + i*packetsize),packetsize);
			Endpoint_ClearOUT();
			while(!Endpoint_IsReadWriteAllowed());
		}
		for(i=0;i<dummypackets;i++){
			DcdDataTransfer(PhyEP,buffer,packetsize);
			Endpoint_ClearOUT();
			while(!Endpoint_IsReadWriteAllowed());
		}		
		stream_total_packets = 0;
	}
}
/********************************************************************//**
 * @brief
 * @param
 * @return
 *********************************************************************/
void DcdDataTransfer( uint8_t EPNum, uint8_t *pData, uint32_t length )
{
	if (!IsOutEndpoint(EPNum))		// IN endpoint
	{
		if(length >= EndpointMaxPacketSize[EPNum])
		{
			if((length == EndpointMaxPacketSize[EPNum])&&(EPNum==1))
			{
				shortpacket = true;
			}
			Remain_length[EPNum/2] = length - EndpointMaxPacketSize[EPNum];
			length = EndpointMaxPacketSize[EPNum];
		}
		else
			Remain_length[EPNum/2] = 0;
	}

	EndPointCmdStsList[EPNum][0].NBytes = length;
	EndPointCmdStsList[EPNum][0].BufferAddrOffset = ( ((uint32_t) pData) >> 6 ) & 0xFFFF;
	EndPointCmdStsList[EPNum][0].Active = 1;
}

void Endpoint_GetSetupPackage(uint8_t* pData)
{
	memcpy(pData, SetupPackage, 8);
	/* Clear endpoint control stall flag if set */
	EndPointCmdStsList[0][0].Stall = 0;
	EndPointCmdStsList[1][0].Stall = 0;
}

/********************************************************************//**
 * @brief
 * @param
 * @return
 *********************************************************************/
static inline void USB_ProcessInterrupt (uint32_t IntStat)
{
	/* SOF Interrupt */
	if (IntStat & FRAME_INT)
	{

	}

	/* Device Status Interrupt (Reset, Connect change, Suspend/Resume) */
	if (IntStat & DEV_STAT_INT)
	{
		uint32_t DevCmdStat = LPC_USB->DEVCMDSTAT;       		/* Device Status */

		if (DevCmdStat & USB_DRESET_C)               	/* Reset */
		{
			LPC_USB->DEVCMDSTAT |= USB_DRESET_C;
			HAL_Reset();
			USB_DeviceState = DEVICE_STATE_Default;
			Endpoint_ConfigureEndpointControl(USB_Device_ControlEndpointSize);
		}

		if (DevCmdStat & USB_DCON_C)                 	/* Connect change */
		{
			LPC_USB->DEVCMDSTAT |= USB_DCON_C;
		}

		if (DevCmdStat & USB_DSUS_C)                   /* Suspend/Resume */
		{
			LPC_USB->DEVCMDSTAT |= USB_DSUS_C;
			if (DevCmdStat & USB_DSUS)                   /* Suspend */
			{
			}
			else                               	/* Resume */
			{
			}
		}
	}

	/* Endpoint's Interrupt */
	if (IntStat & 0x3FF) {  /* if any of the EP0 through EP9 is set, or bit 0 through 9 on disr */
		uint32_t PhyEP;
		for (PhyEP = 0; PhyEP < USED_PHYSICAL_ENDPOINTS; PhyEP++) /* Check All Endpoints */
		{
			if ( IntStat & (1 << PhyEP) )  // Is this the endpoint that caused the interrupt?
			{
				if ( IsOutEndpoint(PhyEP) ) /* OUT Endpoint */
				{                 
					if ( !Endpoint_IsSETUPReceived() )
					{
						if(PhyEP == 0)
							usb_data_buffer_size = (512 - EndPointCmdStsList[PhyEP][0].NBytes);
						else
							usb_data_buffer_OUT_size = (512 - EndPointCmdStsList[PhyEP][0].NBytes);
					}
				}
				else                             /* IN Endpoint */
				{
					if (Remain_length[PhyEP/2] > 0)
					{
						uint32_t i;
						if(PhyEP == 1) /* Control IN */
						{
							for (i = 0; i < Remain_length[PhyEP/2]; i++)
							{
								usb_data_buffer [i] = usb_data_buffer [i + EndpointMaxPacketSize[PhyEP]];
							}
							DcdDataTransfer(PhyEP,usb_data_buffer, Remain_length[PhyEP/2]);
						}
						else
						{
							for (i = 0; i < Remain_length[PhyEP/2]; i++)
							{
								usb_data_buffer_IN [i] = usb_data_buffer_IN [i + EndpointMaxPacketSize[PhyEP]];
							}
							DcdDataTransfer(PhyEP,usb_data_buffer_IN, Remain_length[PhyEP/2]);
						}
					}
					else
					{
						if(PhyEP == 1) /* Control IN */
						{
							if(shortpacket)
							{
								shortpacket = false;
								DcdDataTransfer(PhyEP, usb_data_buffer, 0);
							}
						}
					}
				}
			}
		}
	}
}


/********************************************************************//**
 * @brief
 * @param
 * @return
 *********************************************************************/
#ifdef USB_BANDWIDTH_DIAGNOSTICS
volatile uint32_t USBIrqTime = 0, USBIrqCount = 0;
#endif
void USB_IRQHandler (void)
{
	uint32_t IntStat = LPC_USB->INTSTAT & LPC_USB->INTEN;         /* Get Interrupt Status and clear immediately. */
#ifdef USB_BANDWIDTH_DIAGNOSTICS
	uint32_t USBIrqStartTime = GetTimerCounter();
#endif

	if (IntStat == 0)
		return;

	// Process the interrupt. IntStat holds the interrupt that got us here.
	USB_ProcessInterrupt(IntStat);

	// clear the interrupt in the controller
	LPC_USB->INTSTAT = IntStat;

#ifdef USB_BANDWIDTH_DIAGNOSTICS
	///////////////////////////////////////////////////////////////////////
	// Diagnostics used to tune USB bandwidth performance
	//
	USBIrqTime += (GetTimerCounter() - USBIrqStartTime); // Time taken in the USB IRQ handler
	USBIrqCount++;  // Number of times this interrupt has occurred
	///////////////////////////////////////////////////////////////////////
#endif

	return;
}
#endif // defined(USB_DEVICE_ROM_DRIVER)

#endif /*__LPC11UXX__ || __LPC13UXX__*/
