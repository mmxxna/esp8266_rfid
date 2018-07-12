#ifndef __USER_PARAM_H__
#define __USER_PARAM_H__


// This flash sector stores the user define parameters, for 4096KB(32M bit) flash, 1024+1024 map !!!
#define USER_PARAM_SECTOR    0xF8

#define USER_PARAM_TOTAL_LEN   ((4 + 32 + 64 + 4) / 4)  // in DWords, so the parameter must be 4 bytes aligned
typedef struct
{
	int32 param_total_len;  // this lenght includes all the bytes in the user_saved_param
	char ssid[32];
	char password[64];
	uint32 checksum;
} user_param_items;

typedef union
{
	uint32 dword[USER_PARAM_TOTAL_LEN];
	user_param_items items;
} user_saved_param;

extern user_saved_param user_param;

#endif
