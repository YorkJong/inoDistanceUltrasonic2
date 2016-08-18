/**
 * @file DistanceUltrasonic2.ino
 *  This program shows an ultrasonic measured distance on a 4-bit LED Digital
 *  Tube.
 *
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/18 (initial version)
 * @date 2016/08/18 (last revision)
 * @version 1.0
 */

#define USE_PULSE_MODE

typedef enum {
    PIN_SCLK = 10,
    PIN_RCLK = 11,
    PIN_DIO = 12
} Pin;


void setup()
{
    LED4_init(PIN_SCLK, PIN_RCLK, PIN_DIO);
}


void loop()
{
    uint16_t len_mm = 1234;

    LED4_step(len_mm);
}


