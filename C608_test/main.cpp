#include "C608_test.h"

uint16_t cmd_read_status()
{
	char rcvd_val_h = 0;
	char rcvd_val_l = 0;

	csn = 0;
	spi.write(CMD_READ_STATUS);
	rcvd_val_h = spi.write(0x00);
	rcvd_val_l = spi.write(0x00);
	csn = 1;

	return (rcvd_val_h << 8 | rcvd_val_l);
}

uint16_t cmd_read_config()
{
	char rcvd_val_h = 0;
	char rcvd_val_l = 0;

	csn = 0;
	spi.write(CMD_READ_CONFIG);
	rcvd_val_h = spi.write(0x00);
	rcvd_val_l = spi.write(0x00);
	csn = 1;

	return (rcvd_val_h << 8 | rcvd_val_l);
}

uint16_t cmd_write_config(uint16_t w)
{
	char w_h = 0;
	char w_l = 0;

	w_h = (w & 0xff00) >> 8;
	w_l = w & 0x00ff;

	uint16_t st = cmd_read_status();
	if ((st & ST_ERROR_CMD) >> 7) {
		C608_reset();
	}

	csn = 0;
	spi.write(CMD_WRITE_CONFIG);
	spi.write(w_h);
	spi.write(w_l);
	csn = 1;

	return w;
}

uint16_t cmd_sleep()
{
	csn = 0;
	spi.write(CMD_SLEEP);
	csn = 1;

	return 3;
}

uint16_t cmd_standby()
{
	csn = 0;
	spi.write(CMD_STANDBY);
	csn = 1;

	return 4;
}

void cmd_fp_scan()
{
	pc.printf("FP SCAN ......");

	csn = 0;
	spi.write(CMD_FP_SCAN);

	// for clk until data_rdy high
	while (!data_rdy) { 
		spi.fastWrite(0x00);
		led_scan = 1;
	}

	spi.clearRX();
	csn = 1;

	scan_end = 1;
	led_scan = 0;

	pc.printf("Done!\n");
}

void cmd_read_fp_data()
{
	
	int rows, cols;

	if (current_dpi == 0) {
		rows = 120;		//240;
		cols = 160;

	}
	else {
		rows = 120;
		cols = 80;
	}

	//csn = 0;
	while (!data_rdy);
	// need timeout

	pc.printf("Read FP Data.....");

	csn = 0;
	spi.write(CMD_READ_FP_DATA);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			img_buffer[i][j] = spi.write(0x00);
		}
	}

	csn = 1;
	read_end = 1;

	pc.printf("Done!\n");
}
///////////////////////////////
void scan_read_fp()
{
	cmd_fp_scan();
	cmd_read_fp_data();
}

void save_data()
{
	FILE *fp;
	char fpath[] = "/sd/";
	char fn[8] = "\0";

	pc.printf("\nEnter file name: ");
	pc.scanf("%s", fn);
	char *ffn = (char *)malloc(strlen(fpath) + strlen(fn) + 1);

	strcpy(ffn, fpath);
	strcat(ffn, fn);
	pc.printf("ffn = %s\n", ffn);

	fp = fopen(ffn, "w");

	if (fp == NULL) {
		pc.printf("Could not open file for write.\n");
		return;
	}

	int rows, cols;

	if (current_dpi == 0) {
		rows = 120;		//240;
		cols = 160;

	}
	else {
		rows = 120;
		cols = 80;
	}

	pc.printf("Save Data ......");

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			fputc(img_buffer[i][j], fp);
		}
	}

	pc.printf("Done!\n");
	fclose(fp); 
	free(ffn);
}

void buffer_init()
{
	int rows, cols;

	if (current_dpi == 0) {
		rows = 120;		//240;
		cols = 160;

	}
	else {
		rows = 120;
		cols = 80;
	}

	img_buffer = (char **)malloc(rows * sizeof(char *));
	
	for (int i = 0; i < rows; i++) 
		img_buffer[i] = (char *)malloc(cols * sizeof(char));
}

void buffer_clear()
{
	//int rows;

	//rows = current_dpi ? 120 : 240;
	
	//for (int i = 0; i < rows; i++) 
	//	free(img_buffer[i]);

	free(img_buffer);

	pc.printf("Buffer cleared!\n");
}

/////////////////////////////
// configuration change utils

void C608_reset()
{
	pc.printf("C608 reset !!!\n");

	rst0n = 0;
	wait(0.5);	
	rst0n = 1;
	cmd_write_config(0x0110);	//reset value, 508 DPI

	uint16_t st = cmd_read_status();
	if ((st & ST_ERROR_CMD) >> 7) {
		rst0n = 0;
		wait(0.5);	
		rst0n = 1;
		cmd_write_config(0x0110);
	}
}

void change_led_w()
{
	uint16_t led_w;
	pc.printf("\n");
	pc.printf("LED Width = ");
	pc.scanf("%d", &led_w);

	uint16_t current = cmd_read_config();
	uint16_t led_mask = current & ~CFG_LED_ON_WIDTH;
	uint16_t new_cfg = led_w | led_mask;
	cmd_write_config(new_cfg);
}

void change_dpi()
{
	uint16_t dpi = 1;		// default, 508 dpi
	pc.printf("\n");
	pc.printf("DPI (0: 1016, 1: 508) = ");
	pc.scanf("%d", &dpi);

	current_dpi = dpi;

	uint16_t current = cmd_read_config();
	uint16_t dpi_mask = current & ~CFG_DPI;
	uint16_t new_cfg = (dpi << 8) | dpi_mask;
	cmd_write_config(new_cfg);
}

void change_4pd()
{
	uint16_t npd = 0;		// default 0
	pc.printf("\n");
	pc.printf("4PD (0: 4PD, 1: Merged 1PD) = ");
	pc.scanf("%d", &npd);

	uint16_t current = cmd_read_config();
	uint16_t pd_mask = current & ~CFG_4PD;
	uint16_t new_cfg = (npd << 11) | pd_mask;
	cmd_write_config(new_cfg);
}

void change_test_10_9()
{
	uint16_t v = 0;		// default 0
	pc.printf("\n");
	pc.printf("Test mode bits <10:9> = ");
	pc.scanf("%d", &v);

	uint16_t current = cmd_read_config();
	uint16_t b10_9_mask = current & ~CFG_TEST_10_9;
	uint16_t new_cfg = (v << 9) | b10_9_mask;
	cmd_write_config(new_cfg);
}

void change_test_13_12()
{
	uint16_t v = 0;		// default 0
	pc.printf("\n");
	pc.printf("Test mode bits <13:12> = ");
	pc.scanf("%d", &v);

	uint16_t current = cmd_read_config();
	uint16_t b13_12_mask = current & ~CFG_TEST_13_12;
	uint16_t new_cfg = (v << 12) | b13_12_mask;
	cmd_write_config(new_cfg);
}

void change_test_15_14()
{
	uint16_t v = 0;		// default 0
	pc.printf("\n");
	pc.printf("Test mode bits <15:14> = ");
	pc.scanf("%d", &v);

	uint16_t current = cmd_read_config();
	uint16_t b15_14_mask = current & ~CFG_TEST_15_14;
	uint16_t new_cfg = (v << 14) | b15_14_mask;
	cmd_write_config(new_cfg);
}

///////////////////////////////////
// 
void chk_status_signal()
{
	pc.printf("SIGNAL: LED_ON = %d, DATA_RDY = %d, ERROR = %d\n", (led_on ? 1 : 0), (data_rdy ? 1 : 0), (fp_error ? 1 : 0));
	pc.printf("====================================================\n");
}

void print_status()
{
	uint16_t st = cmd_read_status();
	// read status
	pc.printf("\n");
	pc.printf("=================== Status =========================\n");
	pc.printf("* Read Status = 0x%04x\n", st);
	//pc.printf("--- [15] 1\n");
	//pc.printf("--- [14] 1\n");
	//pc.printf("--- [13] 1\n");
	//pc.printf("--- [12] 1\n");
	pc.printf("--- [11] Error_Data_Read = %d\n", (st & ST_ERROR_DATA_RD) >> 11);
	pc.printf("--- [10] Error_WT_Config = %d\n", (st & ST_ERROR_WT_CFG) >> 10);
	pc.printf("--- [ 9] Error_RD_Status = %d\n", (st & ST_ERROR_RD_STATUS) >> 9);
	pc.printf("--- [ 8] Error_RD_Config = %d\n", (st & ST_ERROR_RD_CFG) >> 8);
	pc.printf("--- [ 7] Error_Command   = %d\n", (st & ST_ERROR_CMD) >> 7);
	pc.printf("--- [ 6] Ready for Read  = %d\n", (st & ST_READY) >> 6);
	pc.printf("--- [ 5] Busy            = %d\n", (st & ST_BUSY) >> 5);
	pc.printf("--- [ 4] Sleep           = %d\n", (st & ST_SLEEP) >> 4);
	pc.printf("--- [ 3] Standby         = %d\n", (st & ST_STANDBY) >> 3);
	//pc.printf("--- [ 2] 0\n");
	//pc.printf("--- [ 1] 0\n");
	//pc.printf("--- [ 0] 0\n");
	pc.printf("====================================================\n");
}

void print_config()
{
	uint16_t cfg = cmd_read_config();
	pc.printf("\n");
	pc.printf("==================================== Configuration ============================\n");
	pc.printf("* Read configuration = 0x%04x\n", cfg);
	pc.printf("--- [15:14] Test mode: Ground level (0-3)                             = %d\n", (cfg & CFG_TEST_15_14) >> 14);
	pc.printf("--- [13:12] Test mode: Slope (0-3)                                    = %d\n", (cfg & CFG_TEST_13_12) >> 12);
	pc.printf("--- [   11] 0:4PD, 1:merged to 1PD                                    = %d\n", (cfg & CFG_4PD) >> 11);
	pc.printf("--- [ 10:9] Test mode: (0:normal, 1:vramp on rst, 2:pd on rst, 3:n/a) = %d\n", (cfg & CFG_TEST_10_9) >>9);
	pc.printf("--- [    8] Current DPI                                               = %d\n", ((cfg & CFG_DPI) >> 8) ? 508 : 1016);
	pc.printf("--- [  7:0] LED ON Width (4-255)                                      = %d\n", (cfg & CFG_LED_ON_WIDTH));
	pc.printf("===============================================================================\n");
}

void print_status_signal()
{
	print_status();
	chk_status_signal();
}

uint16_t print_menu()
{
	uint16_t sel;

	print_menu1();
	pc.scanf("%d", &sel);

	return sel;
}

void print_menu1()
{
	pc.printf("### C608 Test (ver.0.1) ###\n\n");

	for (int i = 0; i < MENU_SZ; i++) {
		pc.printf("%s\n", menu[i]);
	}

	pc.printf("\nSelect menu: ");
}

void spi_config()
{
	spi.format(8, 0);
	spi.frequency(19000000);	// 24MHz, maximum frequency
	spi.setFormat();			// for fastWrite
}
//
void led_on_rise_ISR()
{
	pc.printf("\nLED ON rise interrupt!!!\n\n");
}

void data_rdy_rise_ISR()
{
	pc.printf("\nDATA_RDY rise interrupt!!!\n\n");
}

void fp_error_rise_ISR()
{
	pc.printf("\nFP_ERROR rise interrupt!!!\n\n");
}

void led_on_fall_ISR()
{
	pc.printf("\nLED ON fall interrupt!!!\n\n");
}

void data_rdy_fall_ISR()
{
	pc.printf("\nDATA_RDY fall interrupt!!!\n\n");
}

void fp_error_fall_ISR()
{
	pc.printf("\nFP_ERROR fall interrupt!!!\n\n");
}
/*
// interrupt setting
void isr_set()
{
	led_on.rise(&led_on_rise_ISR);
	data_rdy.rise(&data_rdy_rise_ISR);
	fp_error.rise(&fp_error_rise_ISR);

	led_on.fall(&led_on_fall_ISR);
	data_rdy.fall(&data_rdy_fall_ISR);
	fp_error.fall(&fp_error_fall_ISR);
}
*/
////////////////////////////////////
/*
void save_bmp()
{
	BMP *bmp = (BMP *)malloc(sizeof(BMP));	// bmp file header and info header 

	FILE *fp;
	char fpath[] = "/sd/";
	char fn[25] = "\0";

	pc.printf("\nEnter file name (*.bmp): ");
	pc.scanf("%s", fn);
	char *ffn = (char *)malloc(strlen(fpath) + strlen(fn) + 1);

	strcpy(ffn, fpath);
	strcat(ffn, fn);
	pc.printf("bmp file = %s\n", ffn);

	fp = fopen(ffn, "w");

	if (fp == NULL) {
		pc.printf("Could not open file for write.\n");
		return;
	}

	bmp->fileheader.signature[0] = 'B';
	bmp->fileheader.signature[1] = 'M';
	bmp->fileheader.filesize = BMP_FILE_SZ;
	bmp->fileheader.reserved = 0x00000000;
	bmp->fileheader.offset = BMP_OFFSET;
	bmp->infoheader.dib_header_size = BMP_INFO_HEADER_SZ;
	bmp->infoheader.width = BMP_WIDTH;
	bmp->infoheader.height = BMP_HEIGHT;
	bmp->infoheader.planes = BMP_PLANES;
	bmp->infoheader.bits_per_pixel = BMP_BIT_PER_PIXEL;
	bmp->infoheader.compression = BMP_COMPRESSION;
	bmp->infoheader.image_size = BMP_IMAGE_SZ;
	bmp->infoheader.x_pixel_per_meter = BMP_XPIXEL_PER_METER;
	bmp->infoheader.y_pixel_per_meter = BMP_YPIXEL_PER_METER;
	bmp->infoheader.num_color_pallette = BMP_COLOR_USED;
	bmp->infoheader.important_color = BMP_COLOR_IMPORTANT;
	
	for (int i = 0; i < 256; i++) {
		bmp->colors[i].blue = 0;
		bmp->colors[i].green = 0;
		bmp->colors[i].red = 0;
		bmp->colors[i].rgb_reserved = 0;
	}

	fwrite(bmp, 1, sizeof(BMP), fp);
	fwrite(img_buffer, 1, sizeof(img_buffer), fp);
	fclose(fp);

	free(bmp);
	free(ffn);
}
*/
//
int main()
{
	pc.baud(38400);
	spi_config();
	//
	C608_reset();
	//change_dpi();
	buffer_init();
	//
	while(1) {
		pc.printf("\n\n");
		uint16_t menu = print_menu();
		ptr_func[menu]();

		scan_end = 0;
		read_end = 0;
	}
}


