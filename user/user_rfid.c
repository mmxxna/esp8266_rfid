#include "user_rfid.h"
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "driver/uart_register.h"
#include "mem.h"
#include "os_type.h"

LOCAL os_timer_t rfid_timer;

void ICACHE_FLASH_ATTR
rfid_send_cmd_frame(uint8_t cmd_type, rfid_frame_data_t * frame_data)
{
	uint8_t i;
	uint8_t checksum;
	uart_tx_one_char(UART0, FRAME_HEAD);
	uart_tx_one_char(UART0, FRAME_CMD);
	uart_tx_one_char(UART0, cmd_type);
	uart_tx_one_char(UART0, 0x00);
	checksum = FRAME_CMD + cmd_type + 0x00;
	if (frame_data != NULL) {
		uart_tx_one_char(UART0,frame_data->byteLen);
		checksum += frame_data->byteLen;
		for (i = 0; i < frame_data->byteLen; i++) {
			checksum += frame_data->dat[i];
			uart_tx_one_char(UART0, frame_data->dat[i]);
		}
	} else {
		uart_tx_one_char(UART0,0x00);
	}
	uart_tx_one_char(UART0, checksum);
	uart_tx_one_char(UART0, FRAME_END);
}

void ICACHE_FLASH_ATTR
rfid_single_inventory(void)
{
	rfid_send_cmd_frame(CMD_SINGLE_ID, NULL);
}

void ICACHE_FLASH_ATTR
rfid_begin_continuous_inventory(void)
{
	os_timer_disarm();
	os_timer_setfn(&rfid_timer, (os_timer_func_t *)rfid_single_inventory, NULL);
	os_timer_arm(&rfid_timer, 8, 1);
}

