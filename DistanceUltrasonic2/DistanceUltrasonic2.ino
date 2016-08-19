/**
 * @file DistanceUltrasonic2.ino
 *  This program shows an ultrasonic measured distance on a 4-bit LED Digital
 *  Tube. This program is for testing HC-SR04: a ultrasonic distance sensor.
 *
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/18 (initial version)
 * @date 2016/08/19 (last revision)
 * @version 1.0
 */

typedef enum {
    PIN_Trig = 2,
    PIN_Echo = 3,

    PIN_SCLK = 10,
    PIN_RCLK = 11,
    PIN_DIO = 12
} Pin;


void setup()
{
    HCSR04_init(PIN_Trig, PIN_Echo);
    Digits_init(PIN_SCLK, PIN_RCLK, PIN_DIO);
}


void loop()
{
    uint16_t len_mm;

    if (!HCSR04_measure(&len_mm) || !HCSR04_isValidDistance(len_mm)) {
        unsigned long endMillis = millis() + 1000;
        Digits_clear();
        while (millis() < endMillis)
            ;
        return;
    }

    unsigned long endMillis = millis() + 1000;
    while (millis() < endMillis)
        Digits_step(len_mm);
}


