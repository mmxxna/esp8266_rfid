/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "../include/user_smart_config.h"
#include "user_param.h"
#include "ets_sys.h"
#include "osapi.h"
#include "espconn.h"
#include "c_types.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "driver/uart.h"
#include "user_rfid.h"
#include "user_LLRP.h"

LOCAL struct espconn esp_conn;
LOCAL esp_tcp esptcp;
LOCAL os_timer_t connect_test_timer;

//os_timer_t hello_world_timer;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR
hello_world(void)
{
	struct scan_config config;
	char ssid[32];
	os_printf("\r\nHello world !\r\n");
}

void system_done()
{
	/***
	os_timer_disarm(&hello_world_timer);
	os_timer_setfn(&hello_world_timer, (os_timer_func_t *)hello_world, NULL);
	os_timer_arm(&hello_world_timer, 3000, 1);  **/

}

/******************************************************************************
 * FunctionName : tcp_server_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_sent_cb(void *arg)
{
   //data sent successfully

//    os_printf("tcp sent cb \r\n");
}

/******************************************************************************
 * FunctionName : tcp_server_recv_cb
 * Description  : receive callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
/****
LOCAL void ICACHE_FLASH_ATTR
tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
   //received some data from tcp connection

   struct espconn *pespconn = arg;
//   os_printf("tcp recv : %s \r\n", pusrdata);
   char rtc_time[24];
   os_sprintf(rtc_time, "rtc cal: %d\n", system_rtc_clock_cali_proc()>>12);
   espconn_sent(pespconn, rtc_time, os_strlen(rtc_time));

   if ((pusrdata != NULL) && (length > 0)) {
       if (pusrdata[0] == 'b') {
           rfid_begin_continuous_inventory();
       } else if (pusrdata[0], 'e') {
           rfid_stop_continuous_inventory();
       }
   }

}
*****/

/******************************************************************************
 * FunctionName : tcp_server_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_discon_cb(void *arg)
{
   //tcp disconnect successfully
    system_os_post(tcp_recvTaskPrio, TCP_SIG_DISCONNECTED, 0);
    os_printf("tcp disconnect succeed !!! \r\n");
}

/******************************************************************************
 * FunctionName : tcp_server_recon_cb
 * Description  : reconnect callback, error occured in TCP connection.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_recon_cb(void *arg, sint8 err)
{
   //error occured , tcp connection broke.

    os_printf("tcp reconnect callback, error code %d !!! \r\n",err);
}

LOCAL void tcp_server_multi_send(void)
{
   struct espconn *pesp_conn = &esp_conn;

   remot_info *premot = NULL;
   uint8 count = 0;
   sint8 value = ESPCONN_OK;
   if (espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK){
      char *pbuf = "tcp_server_multi_send\n";
      for (count = 0; count < pesp_conn->link_cnt; count ++){
         pesp_conn->proto.tcp->remote_port = premot[count].remote_port;

         pesp_conn->proto.tcp->remote_ip[0] = premot[count].remote_ip[0];
         pesp_conn->proto.tcp->remote_ip[1] = premot[count].remote_ip[1];
         pesp_conn->proto.tcp->remote_ip[2] = premot[count].remote_ip[2];
         pesp_conn->proto.tcp->remote_ip[3] = premot[count].remote_ip[3];

         espconn_sent(pesp_conn, pbuf, os_strlen(pbuf));
      }
   }
}


/******************************************************************************
 * FunctionName : tcp_server_listen
 * Description  : TCP server listened a connection successfully
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_listen(void *arg)
{
   struct espconn *pesp_conn = arg;
   os_printf("tcp_server_listen !!! \r\n");

   // create system task for LLRP stream
   init_tcp_llrp_task();

   espconn_regist_recvcb(pesp_conn, tcp_recv_cb);
   espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
   espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);

   espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
//   tcp_server_multi_send();
}

/******************************************************************************
 * FunctionName : user_tcpserver_init
 * Description  : parameter initialize as a TCP server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_tcpserver_init(uint32 port)
{
    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, tcp_server_listen);

    sint8 ret = espconn_accept(&esp_conn);

    espconn_regist_time(&esp_conn, 120, 0); // set timeout 120 seconds

    os_printf("espconn_accept [%d] !!! \r\n", ret);

}

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;

   //disarm timer first
    os_timer_disarm(&connect_test_timer);

   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {

      os_printf("got ip !!! \n");
      user_tcpserver_init(SERVER_LOCAL_PORT);

    } else {

        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {

        os_printf("connect fail, begin esp-touch or airkiss!!! \r\n");

        smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
        smartconfig_start(smartconfig_done);

        } else {

           //re-arm timer to check ip
            os_timer_setfn(&connect_test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
            os_timer_arm(&connect_test_timer, 10000, 0);
        }
    }
}

/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
   struct station_config stationConf;

   //need not mac address
   stationConf.bssid_set = 0;

   //Set ap settings
   os_memcpy(&(stationConf.ssid), user_param.items.ssid, sizeof(stationConf.ssid));
   os_memcpy(&(stationConf.password), user_param.items.password, sizeof(stationConf.password));
   wifi_station_set_config(&(stationConf));

   //set a timer to check whether got ip from router succeed or not.
   os_timer_disarm(&connect_test_timer);
   os_timer_setfn(&connect_test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
   os_timer_arm(&connect_test_timer, 6000, 0);
}

void ICACHE_FLASH_ATTR
uart_recvTask(os_event_t *events)
{
    if(events->sig == 0){
    #if  UART_BUFF_EN
        Uart_rx_buff_enq();
    #else
        uint8 fifo_len = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
        uint8 d_tmp = 0;
        uint8 idx=0;

        struct espconn *pesp_conn = &esp_conn;
        remot_info *premot = NULL;
        uint8 count = 0;
        sint8 value = ESPCONN_OK;

        uint8 rx_data[126];

        for(idx=0;idx<fifo_len;idx++) {
            d_tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
            rx_data[idx] = d_tmp;
        }
        if (espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK){
            espconn_send(pesp_conn, rx_data, fifo_len);
        }

        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
        uart_rx_intr_enable(UART0);
    #endif
    }else if(events->sig == 1){
    #if UART_BUFF_EN
	 //already move uart buffer output to uart empty interrupt
        //tx_start_uart_buffer(UART0);
    #else

    #endif
    }
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	bool has_connection_information = false;

//	system_uart_swap();

	uart_init(115200, 115200); // using uart1 as debug output
	system_os_task(uart_recvTask, uart_recvTaskPrio, uart_recvTaskQueue, uart_recvTaskQueueLen);

    os_printf("\r\nSDK version:%s ,", system_get_sdk_version());
	os_printf(" Compile time:%s %s\r\n", __DATE__, __TIME__);

//	system_init_done_cb(system_done);

	if (user_param_load()) {
		has_connection_information = true;
	}

	//Set  station mode
	wifi_set_opmode(STATION_MODE);

	// ESP8266 connect to router.
	if (!has_connection_information)	{
        // begin smart config
        smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
        smartconfig_start(smartconfig_done);
    } else {
        user_set_station_config();
    }

	// if the saved AP can not be connected, it should reset and clear the saved user_param to run ESPTOUCH or Airkiss

	// start TCP server after connection established during user_esp_platform_check_ip
//	user_tcpserver_init(SERVER_LOCAL_PORT);

//	os_printf("free heap size: %d\n", system_get_free_heap_size());  // about 53K

}
