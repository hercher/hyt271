#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "lib/iowkit.h"

/*
 * for temperatures over 0°C
 */
#define COEFF_A 7.5f
#define COEFF_B 237.3f
#define COEFF_C 6.1078f
#define GAS_CONSTANT 8314.3f
#define MOL_WEIGHT 18.016f
#define C2K(c) (c+273.15f)

/*
 * absolute humidity in
 * gram water per cubic meter air
 */
float calc_absolute_humidity(float h, float t) {
	float sdd;

	sdd = COEFF_C * powf(10, (COEFF_A * t) / (COEFF_B + t));
	return 1000 * (MOL_WEIGHT / GAS_CONSTANT) * ((h * sdd) / C2K(t));
}

int main() {
	IOWKIT56_SPECIAL_REPORT report;
	IOWKIT_HANDLE iow;
	ULONG numDevs;
	float temperature, humidity, abshum;
	uint16_t pid;
	int tries = 10;

	if((iow = IowKitOpenDevice()) == NULL) {
	  printf("failed to open device\n");
	  return -1;
	}
	if((numDevs = IowKitGetNumDevs()) != 1) {
	  printf("only one iowarrior supported at the moment\n");
	  IowKitCloseDevice(iow);
	  return -1;
	}
	pid = (uint16_t)IowKitGetProductId(iow);
	//printf("found iowarrior pid:0x%x\n", pid);
	if(pid != IOWKIT_PRODUCT_ID_IOW56) {
	  printf("only iowarrior56 supported at the moment\n");
	  IowKitCloseDevice(iow);
	  return -1;
	}
	/* switch to i2c mode */
	memset(&report, 0x00, IOWKIT56_SPECIAL_REPORT_SIZE);
	report.ReportID = 0x01; //I2C-Mode
	report.Bytes[0] = 0x01; //Enable
	report.Bytes[1] = (1<<7); //Disable internal pullups
	IowKitWrite(iow, IOW_PIPE_SPECIAL_MODE, (char*) &report, IOWKIT56_SPECIAL_REPORT_SIZE);

	while(tries-- > 0) {

		/* poke sensor */
		memset(&report, 0x00, IOWKIT56_SPECIAL_REPORT_SIZE);
		report.ReportID = 0x02; //I2C-write
		report.Bytes[0] = (1<<7)|(1<<6)|0x02;   //generate start, generate stop, send 2 Byte
		report.Bytes[1] = 0x50; //write adress of sensor
		IowKitWrite(iow, IOW_PIPE_SPECIAL_MODE, (char*) &report, IOWKIT56_SPECIAL_REPORT_SIZE);
		IowKitRead(iow, IOW_PIPE_SPECIAL_MODE, (char*) &report, IOWKIT56_SPECIAL_REPORT_SIZE);

		usleep(100 * 1000);

		/* read sensor */
		memset(&report, 0x00, IOWKIT56_SPECIAL_REPORT_SIZE);
		report.ReportID = 0x03; //I2C-Read
		report.Bytes[0] = 0x04; //read 4 bytes
		report.Bytes[1] = 0x51; //read adress of sensor
		IowKitWrite(iow, IOW_PIPE_SPECIAL_MODE, (char*) &report, IOWKIT56_SPECIAL_REPORT_SIZE);
		IowKitRead(iow, IOW_PIPE_SPECIAL_MODE, (char*) &report, IOWKIT56_SPECIAL_REPORT_SIZE);

#if 0
		printf("id:0x%x flags:0x%x data:0x%x 0x%x 0x%x 0x%x\n", 
			report.ReportID, report.Bytes[0], 
			report.Bytes[1], report.Bytes[2], report.Bytes[3], report.Bytes[4]);
#endif

		if(report.Bytes[0] == 0x04)
			break;
	}
	/*
         * Sensor reading are two bytes for humidity, and two bytes
         * for temperature, big-endian.  The top two bits of the
         * humidity value and the bottom two bits of the temperature
         * value are status bits (of undocumented purpose).  Humidity
         * readings range from 0 to 100%; temperature readings range
         * from -40 to 120 degrees C.  In both cases, the ranges
         * correspond to the full range of available bits.
         */
        humidity = ((report.Bytes[1] & 0x3f) << 8 | report.Bytes[2]) * (100.0 / 0x3fff);
        temperature = (report.Bytes[3] << 8 | (report.Bytes[4] & 0xfc)) * (165.0 / 0xfffc) - 40;
	abshum = calc_absolute_humidity(humidity, temperature);
#if 0
	printf("temperature is:%f\n", temperature);
	printf("rel humidity is:%f\n", humidity);
	printf("abs humidity is:%f\n", abshum);
#endif
	printf("%f %f %f\n", temperature, humidity, abshum);

	/* switch back to normal mode */
        memset(&report, 0x00, IOWKIT56_SPECIAL_REPORT_SIZE);
        report.ReportID = 0x01; //I2C-Mode
        report.Bytes[0] = 0x0; //Disable
        IowKitWrite(iow, IOW_PIPE_SPECIAL_MODE, (char*) &report, IOWKIT56_SPECIAL_REPORT_SIZE);

	IowKitCloseDevice(iow);

	/* value read fom hyt271 is most probably crap */
	if(tries == 0)
		return -1;
	return 0;
}

