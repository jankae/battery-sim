##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [2.25.0] date: [Sat Sep 16 14:17:13 CEST 2017] 
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = batsim


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O2


#######################################
# paths
#######################################
# source path
SOURCES_DIR =  \
Drivers/STM32F1xx_HAL_Driver/Src \
Drivers/CMSIS \
Application \
Middlewares/Third_Party/FatFs/src \
Middlewares/Third_Party/FatFs/src/option \
Drivers \
Application/MAKEFILE \
Middlewares/Third_Party/FreeRTOS/Source \
Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 \
Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang \
Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src \
Middlewares/ST/STM32_USB_Device_Library/Core/Src \
Application/User \
Middlewares \
Src \
Abstraction \
Applications \
Applications/Battery \
Applications/Charger \
Applications/Info \
Applications/Settings \
Applications/Supply \
Applications/Scope \
GUI

# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES := $(foreach sdir,$(SOURCES_DIR),$(wildcard $(sdir)/*.c))
CXX_SOURCES := $(foreach sdir,$(SOURCES_DIR),$(wildcard $(sdir)/*.cpp))


# ASM sources
ASM_SOURCES =  \
startup_stm32f103xe.s


######################################
# firmware library
######################################
PERIFLIB_SOURCES = 


#######################################
# binaries
#######################################
BINPATH = /usr/bin
PREFIX = arm-none-eabi-
CC = $(BINPATH)/$(PREFIX)gcc
CXX  = $(BINPATH)/$(PREFIX)g++
AS = $(BINPATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(BINPATH)/$(PREFIX)objcopy
AR = $(BINPATH)/$(PREFIX)ar
SZ = $(BINPATH)/$(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m3

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F103xE


# AS includes
AS_INCLUDES =  \
-IInc

# C includes
C_INCLUDES =  \
-IInc \
-IDrivers/STM32F1xx_HAL_Driver/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IMiddlewares/Third_Party/FatFs/src \
-IMiddlewares/Third_Party/FreeRTOS/Source/include \
-IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
-IDrivers/CMSIS/Include \
-IAbstraction \
-IApplications \
-IApplications/Battery \
-IApplications/Charger \
-IApplications/Info \
-IApplications/Settings \
-IApplications/Supply \
-IApplications/Scope \
-IGUI  \
-IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CXXFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -std=c++11 -Wall -fno-exceptions -fno-rtti -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
CXXFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD 
CXXFLAGS += -MMD


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F103VETx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -u _printf_float -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
DEPS := $(OBJECTS:.o=.d)

-include $(DEPS)

# *** EOF ***
