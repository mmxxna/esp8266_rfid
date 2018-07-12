#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "user_param.h"

user_saved_param user_param;

bool ICACHE_FLASH_ATTR
user_param_load(void)
{
	uint32 check_sum = 0;
	uint32 i;
	spi_flash_read(USER_PARAM_SECTOR * SPI_FLASH_SEC_SIZE, (uint32 *)&user_param,
				sizeof(user_saved_param));
	os_printf("user_param load len: %d, ssid: %s\n", user_param.items.param_total_len, user_param.items.ssid);
	if (user_param.items.param_total_len > 0)	{
		for (i = 0; i < user_param.items.param_total_len - 1; i++) {
			check_sum += user_param.dword[i];
		}
		if (check_sum == user_param.items.checksum) {
			return true;
		}
	}
	os_printf("load failed, checksum: 0x%x", user_param.items.checksum);
	// the user_param loaded from flash is incorrect, so clear the first load user_param
	user_param.items.param_total_len = 0xFFFFFFFF;
	os_memset(user_param.items.ssid, 0, sizeof(user_param.items.ssid));
	os_memset(user_param.items.password, 0, sizeof(user_param.items.password));
	user_param.items.checksum = 0;
	return false;
}

void ICACHE_FLASH_ATTR
user_param_save(void)
{
	uint32 i;
	uint32 check_sum = 0;
	user_param.items.param_total_len = USER_PARAM_TOTAL_LEN;
	for (i = 0; i < USER_PARAM_TOTAL_LEN - 1; i++) {
		check_sum += user_param.dword[i];
	}
	user_param.items.checksum = check_sum;
	spi_flash_erase_sector(USER_PARAM_SECTOR);
	spi_flash_write((USER_PARAM_SECTOR) * SPI_FLASH_SEC_SIZE,
	    		(uint32 *)&user_param, sizeof(user_saved_param));
}
