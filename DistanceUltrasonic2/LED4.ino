/**
 * @file LED4.ino
 * Driver-without-delay of a 4-bit LED Digital Tube Module (four digit LEDs
 * connects to two 74HC595). 74HC595 is an 8-bit serial-in/serial or parallel-
 * out shift register with a storage register and 3-state outputs.
 *
 * @see https://world.taobao.com/item/527643034307.htm?fromSite=main&spm=
 *      a1z09.2.0.0.DLCbWI&_u=i27m1eabfb06
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/18 (initial version)
 * @date 2016/08/18 (last revision)
 * @version 1.0
 */
#include <assert.h>


static uint8_t _sclkPin;    // SHCP: shift register clock input
static uint8_t _rclkPin;    // STCP: storage register clock input
static uint8_t _dioPin;     // DS: serial data input


/** Initializes the display relative pins as output mode. */
static void LED4_initPin(void)
{
    pinMode(_sclkPin, OUTPUT);
    pinMode(_rclkPin, OUTPUT);
    pinMode(_dioPin, OUTPUT);
}


/** Clears the display. */
void LED4_clear(void)
{
    shiftOut(0xFF); // digit part;    1:off, 0:on
    shiftOut(0);    // position part; 0:off, 1:on

    // load to 8-bit storage registers
    digitalWrite(_rclkPin, LOW);
    digitalWrite(_rclkPin, HIGH);
}


/** Initializes the display. */
void LED4_init(uint8_t sclkPin, uint8_t rclkPin, uint8_t dioPin)
{
    _sclkPin = sclkPin;
    _rclkPin = rclkPin;
    _dioPin = dioPin;

    LED4_initPin();
    LED4_clear();
}


/** Steps the 4 digit 7 segment display. This function show at most one digit
 * at once and show 4 digits in turn. Each show is with a 5ms interval. The
 * interval is controlled witout calling delay function.
 * @param number a 4-digit number to show
 */
void LED4_step(uint16_t number)
{
    enum {
        INTERVAL = 4    // milli-seconds; for 3.3V, 8MHz Arduino
    };
    static uint16_t num_bak = 9999;
    static uint16_t remainder, divisor, pos;
    unsigned long currMillis;
    static unsigned long endMillis = 0;

    if (number > 9999) {
        LED4_clear();
        return;
    }

    if ((num_bak != number) || (pos == 0)) {
        remainder = num_bak = number;
        divisor = 1000;
        pos = 4;
    }

    currMillis = millis();
    if (currMillis >= endMillis) {
        endMillis = currMillis + INTERVAL;

        --pos;
        LED4_showDigit(pos, remainder / divisor);
        remainder %= divisor;
        divisor /= 10;
    }
}


/** Shows a digit at a given position.
 * @param pos the position (0..3)
 * @param digit the digit (0..9)
 */
static void LED4_showDigit(int pos, int digit)
{
    // BIT:  7 6 5 4 3 2 1 0
    // LED: dp g f e d c b a
    const static uint8_t digit2LED[] = {
        // 0     1     2     3     4     5     6     7     8     9
        0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
        //// A     b     C     d     E     F       -
        //0x8C, 0x43, 0xC6, 0xA1, 0x86, 0xFF, 0xBF
    };
    const static uint8_t pos2LED[] = {
        1, 2, 4, 8
    };

    assert ((0 <= pos) && (pos <= 3));
    assert ((0 <= digit) && (digit <=9));

    shiftOut(digit2LED[digit]);
    shiftOut(pos2LED[pos]);

    // load to 8-bit storage registers
    digitalWrite(_rclkPin, LOW);
    digitalWrite(_rclkPin, HIGH);
}


/** Shift out the shift registers (74HC595). */
static void shiftOut(uint8_t bitmap)
{
    uint8_t i;

    for (i=8; i>=1; i--) {
        if (bitmap & 0x80)
            digitalWrite(_dioPin, HIGH);
        else
            digitalWrite(_dioPin, LOW);
        bitmap <<= 1;

        // shift one bit
        digitalWrite(_sclkPin, LOW);
        digitalWrite(_sclkPin, HIGH);
    }
}

