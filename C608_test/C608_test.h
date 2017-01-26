#ifndef C608_TEST_H
#define C608_TEST_H

#include "mbed.h"
#include "BurstSPI.h"
#include "SDFileSystem.h"

// command
#define CMD_READ_STATUS 0x3C
#define CMD_READ_CONFIG 0x5A
#define CMD_WRITE_CONFIG 0xA5
#define CMD_SLEEP 0x24
#define CMD_STANDBY 0xD8
#define CMD_FP_SCAN 0x78
#define CMD_READ_FP_DATA 0xC3

// configuration bits
#define CFG_LED_ON_WIDTH ( 0x00ff << 0 )
#define CFG_DPI ( 0x0001 << 8 )
#define CFG_TEST_10_9 ( 0x0003 << 9 )		// 00: normal, 01: test, vramp on rst0n, 10: test, pd on rst0n, 11: n/a
#define CFG_4PD ( 0x0001 << 11 )
#define CFG_TEST_13_12 ( 0x0003 << 12 )
#define CFG_TEST_15_14 ( 0x0003 << 14 )

// status bits
#define ST_ZERO ( 0x0007 << 0 )
#define ST_STANDBY ( 0x0001 << 3 )
#define ST_SLEEP ( 0x0001 << 4 )
#define ST_BUSY ( 0x0001 << 5 )
#define ST_READY ( 0x0001 << 6 )
#define ST_ERROR_CMD ( 0x0001 << 7 )
#define ST_ERROR_RD_CFG ( 0x0001 << 8 )
#define ST_ERROR_RD_STATUS ( 0x0001 << 9 )
#define ST_ERROR_WT_CFG ( 0x0001 << 10 )
#define ST_ERROR_DATA_RD ( 0x0001 << 11 )
#define ST_ALWAYS_ONE ( 0x000f << 12 )
//
#define DPI508_ROWS 120
#define DPI508_COLS 80
#define DPI1016_ROWS 60		// 240
#define DPI1016_COLS 160	// 160
//
#define SPI_MODE_0 0
#define SPI_MODE_1 1
#define SPI_MODE_2 2
#define SPI_MODE_3 3

#define SPI_BIT_FORMAT 8
#define SPI_SCK 19000000		// 19MHz
//
#define USB_SERIAL_BAUD 38400
//
Serial pc(USBTX, USBRX);
DigitalOut scan_end(LED1);
DigitalOut read_end(LED2);
DigitalOut led_scan(p19);
SDFileSystem sd(p11, p12, p13, p14, "sd");

//SPI spi(p5, p6, p7);
BurstSPI spi(p5, p6, p7);
DigitalOut csn(p8);
DigitalOut rst0n(p9);

InterruptIn led_on(p21);
//InterruptIn fp_error(p15);
//InterruptIn data_rdy(p16);
//DigitalIn led_on(p21);
DigitalIn fp_error(p15);
DigitalIn data_rdy(p16);
BusIn status_pin(p15, p16, p21);

//char **img_buffer;
char **img_buffer __attribute__ ((section("AHBSRAM0")));
//char bmp_buffer[10678] __attribute__ ((section("AHBSRAM0")));
//
uint16_t current_cfg = 0xD010;	// Slope = 1, gnd = 3, 1016 dpi, led_on = 16
uint16_t current_dpi = 0;		// 0: 1016 dpi, 1: 508 dpi
//
void led_on_rise_ISR();
void led_on_fall_ISR();
void data_rdy_rise_ISR();
void data_rdy_fall_ISR();
void fp_error_rise_ISR();
void fp_error_fall_ISR();
//
void chk_status_signal();
void C608_reset();
//void spi_config(int, int, int);
void spi_config();
void save_data();
void print_status();
void print_config();
void print_config_status_signal();
void buffer_init();
void buffer_clear();
void change_led_w();
void change_dpi();
void change_4pd();
void change_test_10_9();
void change_test_13_12();
void change_test_15_14();
void scan_read_fp();
uint16_t print_menu();
void print_menu1();
void save_bmp();
void isr_set();
void led_on_rise_ISR();
void ied_on_fall_ISR();


//
uint16_t cmd_read_status();
uint16_t cmd_read_config();
uint16_t cmd_write_config(uint16_t);
uint16_t cmd_sleep();
uint16_t cmd_standby();
void cmd_fp_scan();
void cmd_read_fp_data();

#define MENU_SZ 10

char *menu[] = {
	(char *)("0. Reset C608"),
	(char *)("1. Display current configuration/status"),
	(char *)("2. Change DPI (0: 1016, 1: 508)"),
	(char *)("3. Change LED On Width (0 - 255)"),
	(char *)("4. Change 4PD (0, 1: merger to 1PD)"),
	(char *)("5. Change Test Mode [10: 9] (0-3)"),
	(char *)("6. Change Slope     [13:12] (0-3)"),
	(char *)("7. Change GND Level [15:14] (0-3)"),
	(char *)("8. Scan and Read FP"),
	(char *)("9. Save Data to file")
};

void (*ptr_func[MENU_SZ])(void) = { 
	C608_reset,
	//print_config,
	print_config_status_signal,
	change_dpi,
	change_led_w,
	change_4pd,
	change_test_10_9,
	change_test_13_12,
	change_test_15_14,
	scan_read_fp,
	save_data
	//save_bmp
};

//
#define BMP_FILE_SZ 0x000029b6
#define BMP_OFFSET 0x00000436
#define BMP_INFO_HEADER_SZ 40
#define BMP_WIDTH 80
#define BMP_HEIGHT 120
#define BMP_PLANES 1
#define BMP_BIT_PER_PIXEL 8
#define BMP_COMPRESSION 0
#define BMP_IMAGE_SZ 9600
#define BMP_XPIXEL_PER_METER 2835
#define BMP_YPIXEL_PER_METER 2835
#define BMP_COLOR_USED 256
#define BMP_COLOR_IMPORTANT 256

typedef struct {
	char signature[2];
	uint32_t filesize;
	uint32_t reserved;
	uint32_t offset;
} BMP_fileheader;

typedef struct {
	uint32_t dib_header_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bits_per_pixel;
	uint32_t compression;
	uint32_t image_size;
	uint32_t x_pixel_per_meter;
	uint32_t y_pixel_per_meter;
	uint32_t num_color_pallette;
	uint32_t important_color;
} BMP_infoheader;

typedef struct {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t rgb_reserved;
} RGBQUARD;


typedef struct {
	BMP_fileheader fileheader;
	BMP_infoheader infoheader;
	RGBQUARD colors[256];
} BMP;

#endif

