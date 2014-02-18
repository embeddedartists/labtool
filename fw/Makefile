GCC_BIN =
PROJECT = firmware
USB_OBJS = ./Lib_USB/LPCUSBLib/Drivers/USB/Core/DeviceStandardReq.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/Events.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/USBMemory.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/USBTask.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/Device_LPC.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/Endpoint_LPC.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/EndpointStream_LPC.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/Pipe_LPC.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/PipeStream_LPC.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/USBController_LPC.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/HAL/LPC18XX/HAL_LPC18xx.o \
           ./Lib_USB/LPCUSBLib/Drivers/USB/Core/LPC/DCD/LPC18XX/Endpoint_LPC18xx.o \
           ./Lib_USB/bsp.o
PRG_OBJS = ./program/source/calibrate.o \
           ./program/source/capture.o \
           ./program/source/capture_sgpio.o \
           ./program/source/capture_vadc.o \
           ./program/source/circbuff.o \
           ./program/source/experiments.o \
           ./program/source/generator.o \
           ./program/source/generator_dac.o \
           ./program/source/generator_sgpio.o \
           ./program/source/log.o \
           ./program/source/main.o \
           ./program/source/monitor_i2c.o \
           ./program/source/sgpio_cfg.o \
           ./program/source/statemachine.o \
           ./program/source/usb_descriptors.o \
           ./program/source/usb_handler.o
OBJECTS = $(patsubst %c,%o,$(wildcard ./Lib_Drivers/source/*.c)) \
          $(patsubst %c,%o,$(wildcard ./Lib_MCU/source/*.c)) \
          $(patsubst %c,%o,$(wildcard ./program/sct/*.c)) \
          $(PRG_OBJS) \
          $(USB_OBJS) \
          ./program/source/startup_ARMCM4.o

SYS_OBJECTS = 
INCLUDE_PATHS = -I./Lib_CMSIS/include -I./Lib_Drivers/include -I./Lib_MCU/include \
                -I./Lib_USB/ -I./Lib_USB/LPCUSBLib/Drivers/USB -I./program/include \
                -I./program/sct
LIBRARY_PATHS = 
LIBRARIES = 
LINKER_SCRIPT = ./gcc_arm.ld

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy

ifdef SystemRoot
   RM     = del /Q
   RMOBJS = $(subst /,\,$(OBJECTS))
else
   ifeq ($(shell uname), Linux)
      RM     = rm -f
      RMOBJS = $(OBJECTS)
   endif
endif

CPU = -mcpu=cortex-m4 -mthumb
CC_FLAGS = $(CPU) -c -Os -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections 
CC_SYMBOLS = -DCORE_M4 -DUSE_SPIFI_LIB -D__LPC43XX__ -DUSB_DEVICE_ONLY

LD_FLAGS = -mcpu=cortex-m4 -mthumb -Wl,--gc-sections --specs=nano.specs -u _printf_float -u _scanf_float
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

all: $(PROJECT).bin

.PHONY: clean
clean:
	$(RM) $(PROJECT).bin $(PROJECT).elf $(RMOBJS)

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
