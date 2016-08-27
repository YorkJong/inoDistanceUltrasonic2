/**
 * @file DistanceUltrasonic2.ino
 *  This program shows an ultrasonic measured distance on a 4-bit LED Digital
 *  Tube. This program is for testing HC-SR04: a ultrasonic distance sensor.
 *
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/18 (initial version)
 * @date 2016/08/27 (last revision)
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
    Serial.begin(19200);
}


void loop()
{
    enum {
        REPEAT_PERIOD = 2000 // in milliseconds
    };
    static unsigned long endMillis = 0;
    static float len_cm;

    if (millis() >= endMillis) {
        endMillis = millis() + REPEAT_PERIOD;

        uint16_t len_mm = 0;

        if (!HCSR04_measure(&len_mm))
            len_cm = NAN;   // error
        else if (!HCSR04_isValidDistance(len_mm))
            len_cm = 10000; // out of range
        else
            len_cm = (float)len_mm/10.;

        Serial.println(len_cm);
    }

    Digits_step(len_cm);
}


