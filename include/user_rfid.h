#ifndef __USER_RFID_H__
#define __USER_RFID_H__

#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "driver/uart_register.h"
#include "mem.h"
#include "os_type.h"

#define uart_recvTaskPrio        0
#define uart_recvTaskQueueLen    10
os_event_t    uart_recvTaskQueue[uart_recvTaskQueueLen];

#define FRAME_HEAD      (0xBB)
#define FRAME_END       (0x7E)

enum frame_type {
	FRAME_CMD = 0x00,
	FRAME_RES = 0x01,
	FRAME_INFO = 0x02,
	FRAME_ERROR = 0xFF
};

enum cmd_code {
	CMD_HELLO 			= 0x01,
	CMD_HEART_BEAT		= 0x02,
	CMD_GET_MODULE_INFO	= 0x03,
	CMD_IDLE_MODE       = 0x04,
	CMD_SINGLE_ID 		= 0x22,
	CMD_MULTI_ID 		= 0x27,
	CMD_STOP_MULTI 		= 0x28,
	CMD_READ_DATA 		= 0x39,
	CMD_WRITE_DATA 		= 0x49,
	CMD_LOCK_UNLOCK 	= 0x82,
	CMD_KILL 			= 0x65,
	CMD_RESERVED_FOR_OTHER = 0x06,
	CMD_SET_REGION      = 0x07,
	CMD_GET_REGION      = 0x08,
	CMD_SWITCH_ANT      = 0xA8,
	CMD_INSERT_FHSS_CHANNEL = 0xA9,
	CMD_GET_RF_CHANNEL  = 0xAA,
	CMD_SET_RF_CHANNEL  = 0xAB,
	CMD_SET_CHN2_CHANNEL= 0xAF,
	CMD_SET_FHSS        = 0xAD,
	CMD_SET_POWER 	    = 0xB6,
	CMD_GET_POWER 	    = 0xB7,
	CMD_GET_SELECT_PARA	= 0x0B,
	CMD_SET_SELECT_PARA = 0x0C,
	CMD_GET_QUERY_PARA 	= 0x0D,
	CMD_SET_QUERY_PARA 	= 0x0E,
	CMD_SET_CW 			= 0xB0,
	CMD_SET_BLF 		= 0xBF,
	CMD_FAIL 			= 0xFF,
	CMD_SUCCESS 		= 0x00,
	CMD_SET_SFR 		= 0xFE,
	CMD_READ_SFR 		= 0xFD,
	CMD_INIT_SFR 		= 0xEC,
	CMD_CAL_MX   		= 0xEA,
	CMD_CAL_LPF  		= 0xED,
	CMD_READ_MEM		= 0xFB,
	CMD_SET_INV_MODE    = 0x12,
	CMD_SET_UART_BAUDRATE = 0x11,
	//NXP G2X Tag commands
	CMD_NXP_CHANGE_CONFIG = 0xE0,
	CMD_NXP_READPROTECT	  = 0xE1, //Reset ReadProtect can use the same command code but with different parameter
	CMD_NXP_RESET_READPROTECT = 0xE2,
	CMD_NXP_CHANGE_EAS      = 0xE3,
	CMD_NXP_EAS_ALARM       = 0xE4,

	//Monza QT command
	CMD_IPJ_MONZA_QT_READ  = 0xE5,
	CMD_IPJ_MONZA_QT_WRITE = 0xE6,

	//Gen2 Optional command
	CMD_BLOCKPERMALOCK_READ  = 0xD3,
	CMD_BLOCKPERMALOCK_WRITE = 0xD4,

	//LTU27 Temperature Sensor Tag command
	CMD_READ_TEMPERATUR = 0xE7,

	CMD_SCAN_JAMMER     = 0xF2,
	CMD_SCAN_RSSI       = 0xF3,
	CMD_AUTO_ADJUST_CH  = 0xF4,

	CMD_IO_CONTROL      = 0x1A,

	CMD_SET_MODEM_PARA  = 0xF0,
	CMD_READ_MODEM_PARA = 0xF1,
	CMD_SET_ENV_MODE    = 0xF5,
	CMD_TEST_RESET      = 0x55,

	CMD_POWERDOWN_MODE  = 0x17,
	CMD_SET_SLEEP_TIME  = 0x1D,
	CMD_RESTART         = 0x19,
	CMD_LOAD_NV_CONFIG  = 0x0A,
	CMD_SAVE_NV_CONFIG  = 0x09,

	RESPONSE_DEBUG      = 0x34
};

typedef enum fail_code {
//	FAIL_READ_MULTI_TAG = 0x0B,
	FAIL_INVALID_PARA = 0x0E,
	FAIL_INVENTORY_TAG_TIMEOUT = 0x15,
	FAIL_INVALID_CMD = 0x17,

	FAIL_FHSS_FAIL = 0x20,

	FAIL_ACCESS_PWD_ERROR = 0x16,

	FAIL_READ_MEMORY_NO_TAG = 0x09,
	FAIL_READ_ERROR_CODE_BASE = 0xA0,

	FAIL_WRITE_MEMORY_NO_TAG = 0x10,
	FAIL_WRITE_ERROR_CODE_BASE = 0xB0,

	FAIL_LOCK_NO_TAG = 0x13,
	FAIL_LOCK_ERROR_CODE_BASE = 0xC0,

	FAIL_KILL_NO_TAG = 0x12,
	FAIL_KILL_ERROR_CODE_BASE = 0xD0,

	FAIL_NXP_CHANGE_CONFIG_NO_TAG = 0x1A,
	FAIL_NXP_READPROTECT_NO_TAG = 0x2A,
	FAIL_NXP_RESET_READPROTECT_NO_TAG = 0x2B,
	FAIL_NXP_CHANGE_EAS_NO_TAG = 0x1B,
	FAIL_NXP_CHANGE_EAS_NOT_SECURE = 0x1C,
	FAIL_NXP_EAS_ALARM_NO_TAG = 0x1D,

	FAIL_IPJ_MONZA_QT_NO_TAG = 0x2E,

	FAIL_BLOCK_PERMALOCK_NO_TAG = 0x14,

	FAIL_READ_TEMPERATURE_NO_TAG = 0x3A,

	FAIL_CUSTOM_CMD_BASE = 0xE0
} fail_code_type;

enum error_code {
	FAIL_TAG_OTHER_ERROR = 0x00,
	FAIl_TAG_MEM_OVERRUN = 0x03,
	FAIL_TAG_MEM_LOCKED  = 0x04,
	FAIL_TAG_INSUFFICIENT_POWER = 0x0B,
	FAIL_TAG_NON_SPEC_ERROR = 0x0F
};
enum module_info_code {
	MODULE_HARDWARE_VERSION = 0x00,
	MODULE_SOFTWARE_VERSION = 0x01,
	MODULE_MANUFACTURE_INFO = 0x02
};

typedef struct {
	uint8_t byteLen;
	uint8_t dat[127];
} rfid_frame_data_t;

void uart_recvTask(os_event_t *events);
void rfid_begin_continuous_inventory(void);

#endif
