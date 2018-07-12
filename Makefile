# Main settings includes
#include	../settings.mk
# build directory
BUILD_BASE	= .build

# firmware directory
FW_BASE		= bin

# name for the target project
TARGET		= esp8266_LLRP_MagicRF

# Base directory for the compiler
SDK_BASE	?= /home/aithinker/project/ESP8266_NONOS_SDK
SDK_TOOLS	?= $(SDK_BASE)/tools/

# Extra libs, include and ld file
EXTRA_BASE	?= $(SDK_BASE)/include

BOOT ?= new
APP ?= 1
SPI_SPEED ?= 40
SPI_MODE ?= QIO
SPI_SIZE_MAP ?= 6

#VERBOSE = 1

# Basic project settings
MODULES	= user
LIBS	= c gcc hal phy pp net80211 lwip wpa crypto main json smartconfig airkiss
# Root includes
include	../common_nonos.mk
