/**
 * @file Digits.ino
 * Driver-without-delay of a 4-bit LED Digital Tube Module (four common-anode
 * digit LEDs connects to two 74HC595). 74HC595 is an 8-bit serial-in/serial or
 * parallel-out shift register with a storage register and 3-state outputs.
 *
 * @see https://world.taobao.com/item/527643034307.htm?fromSite=main&spm=
 *      a1z09.2.0.0.DLCbWI&_u=i27m1eabfb06
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/18 (initial version)
 * @date 2016/08/19 (last revision)
 * @version 1.0
 */
#include <assert.h>


//-----------------------------------------------------------------------------
// 4 digit display -- a 4-bit LED Digital Tube Module (common-anode LEDs)
//-----------------------------------------------------------------------------

/** Clears the display. */
void Digits_clear(void)
{
                            // common anode LEDs
    SIPO_shiftByte(0xFF);   // digit part;    1:off, 0:on
    SIPO_shiftByte(0);      // position part; 0:off, 1:on
    SIPO_store();
}


/** Initializes the display.
 * @param sclkPin shift register clock input (SHCP)
 * @param rclkPin storage register clock input (SRCP)
 * @param dioPin serial data input (DS)
 */
void Digits_init(uint8_t sclkPin, uint8_t rclkPin, uint8_t dioPin)
{
    SIPO_init(sclkPin, rclkPin, dioPin);
    Digits_clear();
}


/** Steps the 4 digit 7 segment display. This function show at most one digit
 * at once and show 4 digits in turn. Each show is with a 5ms interval. The
 * interval is controlled witout calling delay function.
 * @param number a 4-digit number to show
 */
void Digits_step(uint16_t number)
{
    enum {
        INTERVAL = 4    // milli-seconds; for 3.3V, 8MHz Arduino
    };
    static uint16_t num_bak = 9999;
    static uint16_t remainder, divisor, pos;
    unsigned long currMillis;
    static unsigned long endMillis = 0;

    if (number > 9999) {
        Digits_clear();
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
        Digits_showDigit(pos, remainder / divisor);
        remainder %= divisor;
        divisor /= 10;
    }
}


/** Shows a digit at a given position.
 * @param pos the position (0..3)
 * @param digit the digit (0..9)
 */
static void Digits_showDigit(int pos, int digit)
{
    assert ((0 <= pos) && (pos <= 3));
    assert ((0 <= digit) && (digit <=9));

    // Layout of LED segments of a digit:
    //       a
    //       -
    //     f| |b
    //       - g
    //     e| |c
    //       -
    //       d  .dp(h)
    const static uint8_t digit2seg[] = {
        //            hgfe dcba
        0x3F,   // 0: 0011 1111
        0x06,   // 1: 0000 0110
        0x5B,   // 2: 0101 1011
        0x4F,   // 3: 0100 1111
        0x66,   // 4: 0110 0110
        0x6D,   // 5: 0110 1101
        0x7D,   // 6: 0111 1101
        0x07,   // 7: 0000 0111
        0x7F,   // 8: 0111 1111
        0x6F,   // 9: 0110 1111
    };

    SIPO_shiftByte(~digit2seg[digit]);  // ~ is for common-anode LEDs
    SIPO_shiftByte(1 << pos);
    SIPO_store();
}


//-----------------------------------------------------------------------------
// 74HC595 -- an 8-bit serial-in/serial or parallel-out shift register
//-----------------------------------------------------------------------------

static uint8_t _shcpPin;    // SHCP: shift register clock input
static uint8_t _stcpPin;    // STCP: storage register clock input
static uint8_t _dsPin;      // DS: serial data input


/** Initializes 74HC595.
 * @param shcpPin shift register clock input
 * @param stcpPin storage register clock input
 * @param dsPin serial data input
 */
static void SIPO_init(uint8_t shcpPin, uint8_t stcpPin, uint8_t dsPin)
{
    _shcpPin = shcpPin;
    _stcpPin = stcpPin;
    _dsPin = dsPin;

    pinMode(_shcpPin, OUTPUT);
    pinMode(_stcpPin, OUTPUT);
    pinMode(_dsPin, OUTPUT);
}


/** Shifts one bit.
 * @param bit the bit to shift
 */
static void SIPO_shiftBit(bool bit)
{
    if (bit != 0)
        digitalWrite(_dsPin, HIGH);
    else
        digitalWrite(_dsPin, LOW);
    digitalWrite(_shcpPin, LOW);
    digitalWrite(_shcpPin, HIGH);
}


/** Shifts 8 bits.
 * @param bitmap a map of byte-width bits
 */
static void SIPO_shiftByte(uint8_t bitmap)
{
    uint8_t i;

    for (i=8; i>=1; i--) {
        SIPO_shiftBit((bitmap & 0x80) == 0x80);
        bitmap <<= 1;
    }
}


/** Stores/Loads to 8-bit storage registers. */
static void SIPO_store(void)
{
    digitalWrite(_stcpPin, LOW);
    digitalWrite(_stcpPin, HIGH);
}

//-----------------------------------------------------------------------------

