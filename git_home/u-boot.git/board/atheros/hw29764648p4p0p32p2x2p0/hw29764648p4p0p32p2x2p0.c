/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <atheros_hw29764648p4p0p32p2x2p0.h>
#ifdef CFG_NMRP
extern int NmrpState;
extern ulong NmrpAliveTimerStart;
extern ulong NmrpAliveTimerBase;
extern int NmrpAliveTimerTimeout;
#endif
extern flash_info_t flash_info[];

/* Record LED status. */
#if LED_NUM > 16
#	error Too many led numbers. Please use larger type.
#else
u16 led_status;
#endif

extern int ath_ddr_initial_config(uint32_t refresh);
extern int ath_ddr_find_size(void);

/* forward reference */
void flush_led_status(u16 new_led_status, short led_num);

#define LSB(bits)		(bits & 0x1)

/*
 * Turn on/off LEDs in LED status variable.
 *
 * It assumes all LEDs are Active LOW.
 *
 * arguments:
 *     led_mask: bit value == 1 means the LED is switched on/off
 *
 * global variables:
 *     led_status: Record LED status. Every bit represents an LED.
 */
#define led_on_no_flush(led_mask)	led_status &= ~(led_mask)
#define led_off_no_flush(led_mask)	led_status |= led_mask

/*
 * Turn on/off LEDs in LED status variable and then flush the variable through
 * SIPO (Serial-In Parallel-Out) shift register.
 *
 * arguments:
 *     led_mask: bit value == 1 means the LED is switched on/off
 *
 * global variables:
 *     led_status: Record LED status. Every bit represents an LED.
 *
 * macros:
 *     LED_NUM: number of LEDs
 */
#define led_on(led_mask)	do {			\
		led_on_no_flush(led_mask);		\
		flush_led_status(led_status, LED_NUM);	\
} while (0)

#define led_off(led_mask)	do {			\
		led_off_no_flush(led_mask);		\
		flush_led_status(led_status, LED_NUM);	\
} while (0)

#ifdef COMPRESSED_UBOOT
#	define prmsg(...)
#	define args		char *s
#	define board_str(a)	do {			\
	char ver[] = "0";				\
	strcpy(s, a " - Honey Bee 1.");			\
	ver[0] += ath_reg_rd(RST_REVISION_ID_ADDRESS)	\
						& 0xf;	\
	strcat(s, ver);					\
} while (0)
#else
#	define prmsg	printf
#	define args		void
#	define board_str(a)				\
	printf(a " - Honey Bee 1.%d", ath_reg_rd	\
			(RST_REVISION_ID_ADDRESS) & 0xf)
#endif

void
ath_usb_initial_config(void)
{
#define unset(a)	(~(a))

	if (ath_reg_rd(RST_BOOTSTRAP_ADDRESS) & RST_BOOTSTRAP_TESTROM_ENABLE_MASK) {

		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_HOST_RESET_SET(1));
		udelay(1000);
		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_PHY_RESET_SET(1));
		udelay(1000);

		ath_reg_wr(PHY_CTRL5_ADDRESS, PHY_CTRL5_RESET_1);
		udelay(1000);

		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_PHY_PLL_PWD_EXT_SET(1));
		udelay(1000);
		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_PHY_ARESET_SET(1));
		udelay(1000);

		ath_reg_rmw_clear(RST_CLKGAT_EN_ADDRESS, RST_CLKGAT_EN_USB1_SET(1));

		return;
	}

	ath_reg_wr_nf(SWITCH_CLOCK_SPARE_ADDRESS,
		ath_reg_rd(SWITCH_CLOCK_SPARE_ADDRESS) |
		SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_SET(2));
	udelay(1000);

	ath_reg_rmw_set(RST_RESET_ADDRESS,
				RST_RESET_USB_PHY_SUSPEND_OVERRIDE_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_ARESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_RESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_HOST_RESET_SET(1));
	udelay(1000);

	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_PLL_PWD_EXT_SET(1));
	udelay(10);
}

void ath_gpio_config(void)
{
	/* Set GPIOs of buttons as input pins */
	ath_reg_rmw_set(GPIO_OE_ADDRESS, ALL_BUTTONS_GPIO_MASK);

	/* disable the CLK_OBS on GPIO_4 and set GPIO4 as input */
	ath_reg_rmw_clear(GPIO_OE_ADDRESS, (1 << 4));
	ath_reg_rmw_clear(GPIO_OUT_FUNCTION1_ADDRESS, GPIO_OUT_FUNCTION1_ENABLE_GPIO_4_MASK);
	ath_reg_rmw_set(GPIO_OUT_FUNCTION1_ADDRESS, GPIO_OUT_FUNCTION1_ENABLE_GPIO_4_SET(0x80));
	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 4));

	/* Disalbe JTAG on GPIO[3:0] because they are used by other functions */
	ath_reg_rmw_set(GPIO_FUNCTION_ADDRESS, GPIO_FUNCTION_DISABLE_JTAG_SET(0x1));

	/* Set GPIOs to SIPO (Serial-In Parallel-Out) shift register LED
	 * controller as output pins */
	ath_reg_rmw_clear(GPIO_OE_ADDRESS, ALL_LED_SIPO_GPIO_MASK);
}

int
ath_mem_config(void)
{
	unsigned int type, reg32, *tap;
	extern uint32_t *ath_ddr_tap_cal(void);

#if !defined(CONFIG_ATH_EMULATION)
	type = ath_ddr_initial_config(CFG_DDR_REFRESH_VAL);

	tap = ath_ddr_tap_cal();
	prmsg("tap = 0x%p\n", tap);

	tap = (uint32_t *)0xbd001f10;
	prmsg("Tap (low, high) = (0x%x, 0x%x)\n", tap[0], tap[1]);

	tap = (uint32_t *)TAP_CONTROL_0_ADDRESS;
	prmsg("Tap values = (0x%x, 0x%x, 0x%x, 0x%x)\n",
		tap[0], tap[2], tap[2], tap[3]);

	/* Take WMAC out of reset */
	reg32 = ath_reg_rd(RST_RESET_ADDRESS);
	reg32 = reg32 & ~RST_RESET_RTC_RESET_SET(1);
	ath_reg_wr_nf(RST_RESET_ADDRESS, reg32);

	ath_usb_initial_config();

	ath_gpio_config();
#endif /* !defined(CONFIG_ATH_EMULATION) */

	return ath_ddr_find_size();
}

long int initdram(int board_type)
{
	return (ath_mem_config());
}

int	checkboard(args)
{
	board_str("U-boot dni29 V0.8 for DNI HW ID: 29764648 flash 4MB RAM 32MB 1st Radio 2x2\n");
	return 0;
}

/*
 * Flush all LED ON/OFF status through SIPO (Serial-In Parallel-Out) shift
 * register controller.
 *
 * arguments:
 *     new_led_status: integer with every bit representing a new LED status
 *
 *     led_num: number of LEDs to determine when all LEDs are flushed
 */
void
flush_led_status(u16 new_led_status, short led_num)
{
	/* Do not use CLEAR pin of SIPO shift register */
	ath_reg_wr_nf(GPIO_SET_ADDRESS, (1 << LED_CLEAR_GPIO));

	for (; led_num > 0; led_num--) {
		/* Negative egde of LED_CLK */
		ath_reg_wr_nf(GPIO_CLEAR_ADDRESS, (1 << LED_CLK_GPIO));

		if (LSB(new_led_status)) {
			ath_reg_wr_nf(GPIO_SET_ADDRESS, (1 << LED_DATA_GPIO));
		} else {
			ath_reg_wr_nf(GPIO_CLEAR_ADDRESS, (1 << LED_DATA_GPIO));
		}

		/* Positive egde of LED_CLK */
		udelay(LED_CLK_DELAY_USEC);
		ath_reg_wr_nf(GPIO_SET_ADDRESS, (1 << LED_CLK_GPIO));

		new_led_status = (new_led_status >> 1);
	}
	return;
}

/*ledstat 0:on; 1:off*/
void board_power_led(int ledstat)
{
	if (ledstat == 1){
		led_off(1 << POWER_GLED);
	} else {
		led_off_no_flush(1 << STATUS_ALED);
		led_on(1 << POWER_GLED);
	}
}

/*ledstat 0:on; 1:off*/
void board_test_led(int ledstat)
{
	if (ledstat == 1){
		led_off(1 << STATUS_ALED);
	} else {
		led_off_no_flush(1 << POWER_GLED);
		led_on(1 << STATUS_ALED);
	}
}

void board_reset_default_LedSet(void)
{
	static int DiagnosLedCount = 1;
	if ((DiagnosLedCount++ % 2) == 1) {
		/*power on test led 0.25s */
		board_test_led(0);
		NetSetTimeout((CFG_HZ * 1) / 4, board_reset_default_LedSet);
	} else {
		/*power off test led 0.75s */
		board_test_led(1);
		NetSetTimeout((CFG_HZ * 3) / 4, board_reset_default_LedSet);
	}
}

/*return value 0: not pressed, 1: pressed*/
int board_reset_button_is_press()
{
	if (ath_reg_rd(GPIO_IN_ADDRESS) & (1 << RESET_BUTTON_GPIO))
		return 0;
	return 1;
}

extern int flash_sect_erase (ulong, ulong);

/*erase the config sector of flash*/
void board_reset_default()
{
	printf("Restore to factory default\n");
	flash_sect_erase(CFG_FLASH_CONFIG_BASE,
	      CFG_FLASH_CONFIG_BASE + CFG_FLASH_CONFIG_PARTITION_SIZE - 1);
#ifdef CFG_NMRP
	if (NmrpState != 0)
		return;
#endif
	printf("Rebooting...\n");
	do_reset(NULL, 0, 0, NULL);
}

int misc_init_r(void)
{
	/* Switch on only STATUS_ALED */
	led_status = 0xffff;
	led_on(1 << STATUS_ALED);

	return 0;
}

#if defined(CFG_SINGLE_FIRMWARE)
void board_upgrade_string_table(unsigned char *load_addr, int table_number, unsigned int file_size)
{
    unsigned char *string_table_addr, *addr2;
    unsigned long offset;
    unsigned int table_length;
    unsigned char high_bit, low_bit;
    unsigned long passed;
    int offset_num;
    uchar *src_addr;
    ulong target_addr;

    /* set different CFG_STRING_TABLE_ADDR_BEGIN for JNR3300 */
    char board_model_id[BOARD_MODEL_ID_LENGTH + 1];
    unsigned int cfg_string_table_addr_begin = CFG_STRING_TABLE_ADDR_BEGIN;
    memset(board_model_id, 0, sizeof(board_model_id));
    get_board_data(BOARD_MODEL_ID_OFFSET, BOARD_MODEL_ID_LENGTH, (u8 *)board_model_id);
    if(strcmp(board_model_id,"JNR3300") == 0)
        cfg_string_table_addr_begin = CFG_FLASH_BASE + 0x3b0000;

    /* Read whole string table partition from Flash to RAM(load_addr+128k) */
    string_table_addr = load_addr + 0x20000;
    memset(string_table_addr, 0, CFG_STRING_TABLE_TOTAL_LEN);
    memcpy(string_table_addr, cfg_string_table_addr_begin, CFG_STRING_TABLE_TOTAL_LEN);

    /* Save string table checksum to (100K - 1) */
    memcpy(load_addr + CFG_STRING_TABLE_LEN - 1, load_addr + file_size- 1, 1);
    /* Remove checksum from string table file's tail */
    memset(load_addr + file_size - 1, 0, 1);

    table_length = file_size - 1;
    printf("string table length is %d\n", table_length);

    /* Save (string table length / 1024)to 100K-3 */
    high_bit = table_length / 1024;
    addr2 = load_addr + CFG_STRING_TABLE_LEN - 3;
    memcpy(addr2, &high_bit, sizeof(high_bit));

    /* Save (string table length % 1024)to 100K-2 */
    low_bit = table_length % 1024;
    addr2 = load_addr + CFG_STRING_TABLE_LEN - 2;
    memcpy(addr2, &low_bit, sizeof(low_bit));

    /* Copy processed string table from load_addr to RAM */
    offset = (table_number - 1) * CFG_STRING_TABLE_LEN;
    memcpy(string_table_addr + offset, load_addr, CFG_STRING_TABLE_LEN);

    /* Write back string tables from RAM to Flash */
    printf("updating %x to %x\n", cfg_string_table_addr_begin, CFG_STRING_TABLE_ADDR_END);
    update_data(string_table_addr, CFG_STRING_TABLE_TOTAL_LEN, cfg_string_table_addr_begin, 1);
    return;
}
#endif

#if defined(NETGEAR_BOARD_ID_SUPPORT)
/*
 * item_name_want could be "device" to get Model Id, "version" to get Version
 * or "hd_id" to get Hardware ID.
 */
void board_get_image_info(ulong fw_image_addr, char *item_name_want, char *buf)
{
    char image_header[HEADER_LEN];
    char item_name[HEADER_LEN];
    char *item_value;
    char *parsing_string;

    memset(image_header, 0, HEADER_LEN);
    memcpy(image_header, fw_image_addr, HEADER_LEN);

    parsing_string = strtok(image_header, "\n");
    while (parsing_string != NULL) {
       memset(item_name, 0, sizeof(item_name));
       strncpy(item_name, parsing_string, (int)(strchr(parsing_string, ':') - parsing_string));

       if (strcmp(item_name, item_name_want) == 0) {
           item_value = strstr(parsing_string, ":") + 1;

           memcpy(buf, item_value, strlen(item_value));
       }

       parsing_string = strtok(NULL, "\n");
    }
}

int board_match_image_hw_id (ulong fw_image_addr)
{
    char board_hw_id[BOARD_HW_ID_LENGTH + 1];
    char image_hw_id[BOARD_HW_ID_LENGTH + 1];
    char image_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char image_version[20];

    /*get hardward id from board */
    memset(board_hw_id, 0, sizeof(board_hw_id));
    memcpy(board_hw_id, (void *)(BOARDCAL + BOARD_HW_ID_OFFSET),
           BOARD_HW_ID_LENGTH);
    printf("HW ID on board: %s\n", board_hw_id);

    /*get hardward id from image */
    memset(image_hw_id, 0, sizeof(image_hw_id));
    board_get_image_info(fw_image_addr, "hd_id", (char*)image_hw_id);
    printf("HW ID on image: %s\n", image_hw_id);

    if (memcmp(board_hw_id, image_hw_id, BOARD_HW_ID_LENGTH) != 0) {
            printf("Firmware Image HW ID do not match Board HW ID\n");
            return 0;
    }
    printf("Firmware Image HW ID matched Board HW ID\n\n");
    return 1;
}

int board_match_image_model_id (ulong fw_image_addr)
{
    char board_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char image_model_id[BOARD_MODEL_ID_LENGTH + 1];

    /*get hardward id from board */
    memset(board_model_id, 0, sizeof(board_model_id));
    memcpy(board_model_id, (void *)(BOARDCAL + BOARD_MODEL_ID_OFFSET),
           BOARD_MODEL_ID_LENGTH);
    printf("MODEL ID on board: %s\n", board_model_id);

    /*get hardward id from image */
    memset(image_model_id, 0, sizeof(image_model_id));
    board_get_image_info(fw_image_addr, "device", (char*)image_model_id);
    printf("MODEL ID on image: %s\n", image_model_id);

    if (memcmp(board_model_id, image_model_id, BOARD_MODEL_ID_LENGTH) != 0) {
            printf("Firmware Image MODEL ID do not match Board model ID\n");
            return 0;
    }
    printf("Firmware Image MODEL ID matched Board model ID\n\n");
    return 1;
}

void board_update_image_model_id (ulong fw_image_addr)
{
    char board_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char image_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char sectorBuff[CFG_FLASH_SECTOR_SIZE];

    /* Read boarddata */
    memcpy(sectorBuff, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    /*get model id from board */
    memset(board_model_id, 0, sizeof(board_model_id));
    memcpy(board_model_id, (void *)(BOARDCAL + BOARD_MODEL_ID_OFFSET),
           BOARD_MODEL_ID_LENGTH);
    printf("Original board MODEL ID: %s\n", board_model_id);

    /*get model id from image */
    memset(image_model_id, 0, sizeof(image_model_id));
    board_get_image_info(fw_image_addr, "device", (char*)image_model_id);
    printf("New MODEL ID from image: %s\n", image_model_id);

    printf("Updating MODEL ID\n");
    memcpy(sectorBuff + BOARD_MODEL_ID_OFFSET, image_model_id,
	    BOARD_MODEL_ID_LENGTH);
    flash_erase(flash_info, CAL_SECTOR, CAL_SECTOR);
    write_buff(flash_info, sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    printf("done\n\n");
}
#endif	/* BOARD_ID */
