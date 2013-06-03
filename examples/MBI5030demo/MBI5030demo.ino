//
// To get a suitably high GSCLK frequency, the CLKO-FUSE of the ATMega168/328 has been programmed
// It will output it's system clock on PB0 ("digital pin" #8)
//
// Diecimila + ATmega168: lfuse: 0xBF
//                        hfuse: 0xDD
//                        efuse: 0x00 (or 0xF8)
//
// Uno + ATmega328:       lfuse: 0xBF
//                        hfuse: 0xDE
//                        efuse: 0x05 (or 0xFD)
//                        
// To revert to Arduino's default FUSE settings, change 0xBF to 0xFF.
//
// Use: "http://www.engbedded.com/fusecalc" and ".../arduino-XXX/hardware/arduino/boards.txt"
//
// Make sure to get the latest MBI5030 datasheet (at least version Jan. 2009)!
// 

#include <avr/io.h>
#include <stdint.h>
#include <MBI5030.h>

#define USE_12BIT_PWM

#ifdef USE_16BIT_PWM
#define BRIGHTNESS_MAX 65535
#define BRIGHTNESS_STEPSIZE 256
#define FADE_DELAY 1
#endif

#ifdef USE_12BIT_PWM
#define BRIGHTNESS_MAX 4095
#define BRIGHTNESS_STEPSIZE 64
#define FADE_DELAY 1
#endif

MBI5030 chip1(14, 17, 15, 16);	// spi_out, spi_in, spi_clk, spi_latch
MBI5030 chip2(7, 6, 5, 4);
uint16_t pwm_data[16];

void setup(void)
{
	Serial.begin(9600);
	memset(pwm_data, 0x0000, sizeof(pwm_data));
	chip1.spi_init();
	chip2.spi_init();

	//
	// The current-set resistor 'R-EXT' of the MBI5030 should be chosen such
	// that the maximum current is only reached if the digital current gain
	// is set to '2'. That way the chip should always operate within specs.
	// 
	// The chip starts up with a gain setting of '1', which means only half
	// of the maximum current is available. Next we'll change the configuration
	// register to get a gain of '2' for full brightness.
	//
	chip1.write_config(0x0000 | PWM_12BIT, 0xFF);	// 1st number: blank configuration bits (see header file of lib), 2nd number: current gain
	chip2.write_config(0x0000 | PWM_12BIT, 0xFF);

	chip1.update(pwm_data);
	chip2.update(pwm_data);
}

void loop(void)
{
	Serial.print("CHIP1 - LED error-status: ");
	Serial.println(chip1.read_error_report(), BIN);
	Serial.print("CHIP1 - config register: ");
	Serial.println(chip1.read_config(), BIN);
	Serial.print("CHIP2 - LED error-status: ");
	Serial.println(chip2.read_error_report(), BIN);
	Serial.print("CHIP2 - config register: ");
	Serial.println(chip2.read_config(), BIN);
	uint8_t counter;
	uint16_t brightness;

	brightness = 0;
	while (brightness <= BRIGHTNESS_MAX) {

		for (counter = 0; counter <= 15; counter++) {
			pwm_data[counter] = brightness;
		}

		chip1.update(pwm_data);
		chip2.update(pwm_data);

		if (brightness <= BRIGHTNESS_MAX - BRIGHTNESS_STEPSIZE) {
			brightness += BRIGHTNESS_STEPSIZE;
		} else if (brightness == BRIGHTNESS_MAX) {
			break;
		} else {
			brightness = BRIGHTNESS_MAX;
		}
		delay(FADE_DELAY);
	}

	while (brightness >= 0) {

		for (counter = 0; counter <= 15; counter++) {
			pwm_data[counter] = brightness;
		}

		chip1.update(pwm_data);
		chip2.update(pwm_data);

		if (brightness >= 0 + BRIGHTNESS_STEPSIZE) {
			brightness -= BRIGHTNESS_STEPSIZE;
		} else if (brightness == 0) {
			break;
		} else {
			brightness = 0;
		}
		delay(FADE_DELAY);
	}
	delay(250);
}
