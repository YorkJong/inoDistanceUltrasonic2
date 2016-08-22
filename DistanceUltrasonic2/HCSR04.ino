/**
 * @file HCSR04.ino
 *  UC-SR04 ultrasonic distance sensor
 * @see http://www.instructables.com/id/
 *      Easy-ultrasonic-4-pin-sensor-monitoring-hc-sr04/
 *
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/19 (initial version)
 * @date 2016/08/19 (last revision)
 * @version 1.0
 */

static uint8_t _trigPin;
static uint8_t _echoPin;

/** Initializes the HC-SR04.
 * @param trigPin assign the Trig pin.
 * @param echoPin assign the Echo pin.
 */
void HCSR04_init(uint8_t trigPin, uint8_t echoPin)
{
    _trigPin = trigPin;
    _echoPin = echoPin;

    pinMode(_trigPin, OUTPUT);   // set Trig pin output mode.
    pinMode(_echoPin, INPUT);    // set Echo pin input mode.
}


/** Measures a distance.
 * @param[out] len_mm the measured distance.
 * @retval true in the case of success to get the distance;
 * @retval fasle otherwise.
 */
bool HCSR04_measure(uint16_t *len_mm)
{
    unsigned long echo_us;

    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);           // Set the pulse width of >= 2us

    // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(5);           // Set the pulse width of >= 2us
    digitalWrite(_trigPin, LOW);    // The end of the pulse

    // A pulse width calculating HC-SR04 returned
    echo_us = pulseIn(_echoPin, HIGH);

    // pluseIn works on pulses from 10us to 180*1000000us in length.
    // Normal distance range: (1mm, 10000mm)
    // -> Pulse effective range: (10us, 60000us).
    if ((10 < echo_us) && (echo_us < 60000)) {
        // Millimeters = PulseWidth * 34 / 100 / 2
        *len_mm = echo_us * 34/100 / 2;

        return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// HC-SR04 Utility Functions
//-----------------------------------------------------------------------------

/** Decides if a measureed distance is valid. */
bool HCSR04_isValidDistance(uint16_t len_mm)
{
    // normal distance should between 1mm and 10000mm (1mm, 10m)
    enum {
        LEN_MIN = 20,
        LEN_MAX = 4500
    };  // length in mm

    if (len_mm < LEN_MIN)
        return false;
    if (len_mm > LEN_MAX)
        return false;

    return true;
}


//-----------------------------------------------------------------------------


