
GCC_BIN = 
PROJECT = firmware
OBJECTS = ./Lib_Drivers\source\lpc43xx_cgu_improved.o ./Lib_Drivers\source\spi_control.o ./Lib_Drivers\source\spi_dac.o ./Lib_Drivers\source\spi_eeprom.o ./Lib_MCU\source\debug_frmwrk.o ./Lib_MCU\source\lpc43xx_adc.o ./Lib_MCU\source\lpc43xx_atimer.o ./Lib_MCU\source\lpc43xx_cgu.o ./Lib_MCU\source\lpc43xx_dac.o ./Lib_MCU\source\lpc43xx_gpdma.o ./Lib_MCU\source\lpc43xx_gpio.o ./Lib_MCU\source\lpc43xx_i2c.o ./Lib_MCU\source\lpc43xx_nvic.o ./Lib_MCU\source\lpc43xx_rgu.o ./Lib_MCU\source\lpc43xx_sct.o ./Lib_MCU\source\lpc43xx_scu.o ./Lib_MCU\source\lpc43xx_ssp.o ./Lib_MCU\source\lpc43xx_timer.o ./Lib_MCU\source\lpc43xx_uart.o ./Lib_MCU\source\lpc43xx_wwdt.o ./Lib_MCU\source\system_LPC43xx.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\DeviceStandardReq.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\Events.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\USBMemory.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\USBTask.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\Device_LPC.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\Endpoint_LPC.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\EndpointStream_LPC.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\Pipe_LPC.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\PipeStream_LPC.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\USBController_LPC.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\HAL\LPC18XX\HAL_LPC18xx.o ./Lib_USB\LPCUSBLib\Drivers\USB\Core\LPC\DCD\LPC18XX\Endpoint_LPC18xx.o ./Lib_USB\bsp.o ./program\sct\sct_fsm.o ./program\source\startup_ARMCM4.o ./program\source\calibrate.o ./program\source\capture.o ./program\source\capture_sgpio.o ./program\source\capture_vadc.o ./program\source\circbuff.o ./program\source\experiments.o ./program\source\generator.o ./program\source\generator_dac.o ./program\source\generator_sgpio.o ./program\source\log.o ./program\source\main.o ./program\source\monitor_i2c.o ./program\source\sgpio_cfg.o ./program\source\statemachine.o ./program\source\usb_descriptors.o ./program\source\usb_handler.o
SYS_OBJECTS = 
INCLUDE_PATHS = -I./Lib_CMSIS/include -I./Lib_Drivers/include -I./Lib_MCU/include -I./Lib_USB/ -I./Lib_USB/LPCUSBLib/Drivers/USB -I./program/include -I./program/sct
LIBRARY_PATHS = 
LIBRARIES = 
LINKER_SCRIPT = ./gcc_arm.ld

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy

CPU = -mcpu=cortex-m4 -mthumb
CC_FLAGS = $(CPU) -c -Os -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections 
CC_SYMBOLS = -DCORE_M4 -DUSE_SPIFI_LIB -D__LPC43XX__ -DUSB_DEVICE_ONLY

LD_FLAGS = -mcpu=cortex-m4 -mthumb -Wl,--gc-sections --specs=nano.specs -u _printf_float -u _scanf_float
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

all: $(PROJECT).bin

clean:
	rm -f $(PROJECT).bin $(PROJECT).elf $(OBJECTS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(LIBRARIES) $(LD_SYS_LIBS)

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $< $@