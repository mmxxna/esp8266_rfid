/** LICENSE Declaration **/

#ifndef __USER_SMART_CONFIG_H__
#define __USER_SMART_CONFIG_H__

#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"

#include "user_interface.h"
#include "smartconfig.h"
#include "airkiss.h"


#define DEVICE_TYPE 		"gh_9e2cff3dfa51" //wechat public number
#define DEVICE_ID 			"122475" //model ID

#define DEFAULT_LAN_PORT 	12476

LOCAL void airkiss_wifilan_time_callback(void);
LOCAL void airkiss_wifilan_recv_callbk(void *arg, char *pdata, unsigned short len);
void airkiss_start_discover(void);
void smartconfig_done(sc_status status, void *pdata);

#endif

