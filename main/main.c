#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "nofrendo.h"
#include "esp_partition.h"
#include "8bkc-hal.h"
#include "appfs.h"
#include "powerbtn_menu.h"
#include "8bkc-ugui.h"
#include "ugui.h"
#include "8bkcgui-widgets.h"


//as selected in main menu
int rom_fd;

char *osd_getromdata() {
	int sz;
	char *romdata;
	spi_flash_mmap_handle_t hrom=NULL;
	appfsEntryInfo(rom_fd, NULL, &sz);
	esp_err_t err=appfsMmap(rom_fd, 0, sz, (const void**)&romdata, SPI_FLASH_MMAP_DATA, &hrom);
	if (err!=ESP_OK) {
		printf("Error: mmap for rom failed\n");
	}
	return (char*)romdata;
}


esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

static void debug_screen() {
	kcugui_cls();
	UG_FontSelect(&FONT_6X8);
	UG_SetForecolor(C_WHITE);
	UG_PutString(0, 0, "INFO");
	UG_SetForecolor(C_YELLOW);
	UG_PutString(0, 16, "Nofrendo");
	UG_PutString(0, 24, "Gitrev");
	UG_SetForecolor(C_WHITE);
	UG_PutString(0, 32, GITREV);
	UG_SetForecolor(C_YELLOW);
	UG_PutString(0, 40, "Compiled");
	UG_SetForecolor(C_WHITE);
	UG_PutString(0, 48, COMPILEDATE);
	kcugui_flush();

	while (kchal_get_keys()&KC_BTN_SELECT) vTaskDelay(100/portTICK_RATE_MS);
	while (!(kchal_get_keys()&KC_BTN_SELECT)) vTaskDelay(100/portTICK_RATE_MS);
}


static int fccallback(int button, void **glob, char **desc, void *usrptr) {
	if (button & KC_BTN_POWER) {
		int r=powerbtn_menu_show(kcugui_get_fb());
		//No need to save state or anything as we're not in a game.
		if (r==POWERBTN_MENU_EXIT) kchal_exit_to_chooser();
		if (r==POWERBTN_MENU_POWERDOWN) kchal_power_down();
	}
	if (button & KC_BTN_SELECT) debug_screen();
	return 0;
}


int app_main(void)
{
	kchal_init();
	nvs_flash_init();

	kcugui_init();
	rom_fd=kcugui_filechooser("*.nes,*.bin", "Select ROM", fccallback, NULL);
	kcugui_deinit();

	printf("NoFrendo start!\n");
	nofrendo_main(0, NULL);
	printf("NoFrendo died? WtF?\n");
	kchal_exit_to_chooser();
	return 0;
}

