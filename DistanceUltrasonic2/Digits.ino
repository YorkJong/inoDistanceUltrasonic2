/**
 * @file Digits.ino
 * Driver-without-delay of a 4-bit LED Digital Tube Module (four common-anode
 * digit LEDs connects to two 74HC595). 74HC595 is an 8-bit serial-in/serial or
 * parallel-out shift register with a storage register and 3-state outputs.
 *
 * @see https://world.taobao.com/item/
 *      527643034307.htm?fromSite=main&spm=a1z09.2.0.0.DLCbWI&_u=i27m1eabfb06
 * @see https://www.arduino.cc/en/Tutorial/ShiftOut
 * @author Jiang Yu-Kuan <yukuan.jiang@gmail.com>
 * @date 2016/08/18 (initial version)
 * @date 2016/09/08 (last revision)
 * @version 2.0
 */
#include <assert.h>


#define ElemsOfArray(x) (sizeof(x) / sizeof(x[0]))


//-----------------------------------------------------------------------------
// 4 digit display -- a 4-bit LED Digital Tube Module (common-anode LEDs)
//-----------------------------------------------------------------------------

enum {
    TOTAL_POSITIONS = 4
};

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

//-----------------------------------------------------------------------------

/** Decides if two real number is equal.
 * @note This function is for support NAN comparing.
 */
static bool isEqual(float a, float b)
{
    if (a == b)
        return true;
    if (isnan(a) && isnan(b))
        return true;
    return false;
}


/** Steps/Updates the 4-digit display with a real number. This function show at
 * most one character at once and show 4 digits in turn. Each show is with a
 * 4ms interval. The interval is controlled witout calling delay function.
 * @param num a real number to show
 */
void Digits_step(float num)
{
    static float num_bak = 9999.;
    static char buf[] = "0.123";

    if (!isEqual(num_bak, num)) {
        num_bak = num;

        if (isnan(num)) {
            snprintf(buf, sizeof(buf), "ERR. ");    // error
        }
        else if (num > 9999.) {
            snprintf(buf, sizeof(buf), "OOR ");     // out of range
        }
        else {
            // NOTE: snprintf in Arduino does not support %f
            if (num < 10.)
                dtostrf(num, 5, 3, buf);    // fmt: "%5.3f"
            else if (num < 100)
                dtostrf(num, 5, 2, buf);    // fmt: "%5.2f"
            else if (num < 1000)
                dtostrf(num, 5, 1, buf);    // fmt: "%5.1f"
        }
    }

    Digits_step((const char*)buf);
}



/** Steps/Updates the 4-digit display with a C string. This function show at
 * most one character at once and show 4 digits in turn. Each show is with a
 * 4ms interval. The interval is controlled witout calling delay function.
 * @param buf the buffer to store the C string (zero terminated).
 */
void Digits_step(const char buf[])
{
    static uint8_t pos = 0;
    static uint8_t i, n;

    if (pos == 0) {
        pos = TOTAL_POSITIONS;
        i = 0;

        n = strlen(buf);
        assert (n <= TOTAL_POSITIONS*2);
    }

    enum {
        INTERVAL = 4    // milli-seconds; for 3.3V, 8MHz Arduino
    };
    static unsigned long endMillis = 0;
    unsigned long currMillis = millis();
    if (currMillis >= endMillis) {
        endMillis = currMillis + INTERVAL;

        bool withDot = ((i+1) < n) && (buf[i+1] == '.') && (buf[i] != '.');
        Digits_showChar(--pos, buf[i], withDot);
        if (withDot)
            ++i;
        ++i;
    }
}

//-----------------------------------------------------------------------------

/** Shows a digit at a given position.
 * @param pos the position (0..3)
 * @param digit the digit (0..9)
 * @param withDot decides if add dot after a digit
 */
static void Digits_showDigit(uint8_t pos, uint8_t digit, bool withDot)
{
    assert ((0 <= pos) && (pos <= 3));
    assert ((0 <= digit) && (digit <=9));

    // Layout of LED segments of a digit:
    //       a
    //       -
    //     f| |b
    //       - g
    //     e| |c
    //       -   .dp(h)
    //       d
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

    uint8_t bitmap = digit2seg[digit];
    if (withDot)
        bitSet(bitmap, 7);  // add dot ('.')

    SIPO_shiftByte(~bitmap);    // ~ is for common-anode LEDs
    SIPO_shiftByte(1 << pos);
    SIPO_store();
}


/** Shows a character at a given position.
 * @param pos the position (0..3)
 * @param ch the symbol
 * @param withDot decides if add dot after a digit
 */
static void Digits_showSymbol(uint8_t pos, char ch, bool withDot)
{
    // Layout of LED segments of a symbol:
    //       a
    //       -
    //     f| |b
    //       - g
    //     e| |c
    //       -   .dp(h)
    //       d
    const static struct {
        char ch;
        uint8_t bitmap;
    } map[] = {
        //                 hgfe dcba
        {' ', 0x00},    // 0000 0000
        {'_', 0x08},    // 0000 1000
        {'-', 0x40},    // 0100 0000
        {'.', 0x80},    // 1000 0000
        {'A', 0x77},    // 0111 0111
        {'B', 0x7F},    // 0111 1111
        {'b', 0x7C},    // 0111 1100
        {'C', 0x39},    // 0011 1001
        {'c', 0x58},    // 0101 1000
        {'d', 0x5E},    // 0101 1110
        {'E', 0x79},    // 0111 1001
        {'F', 0x71},    // 0111 0001
        {'g', 0x6F},    // 0110 1111
        {'H', 0x76},    // 0111 0110
        {'h', 0x74},    // 0111 0100
        {'I', 0x30},    // 0011 0000
        {'J', 0x0E},    // 0000 1110
        {'L', 0x38},    // 0011 1000
        {'n', 0x54},    // 0101 0100
        {'O', 0x3F},    // 0011 1111
        {'o', 0x5C},    // 0101 1100
        {'P', 0x73},    // 0111 0011
        {'q', 0x67},    // 0110 0111
        {'R', 0x77},    // 0111 0111
        {'S', 0x6D},    // 0110 1101
        {'U', 0x3E},    // 0011 1110
        {'V', 0x3E},    // 0011 1110
        {'y', 0x6E},    // 0110 1110
    };
    uint8_t bitmap = 0x00;  // initializes with space (' ')

    for (int i=0; i<ElemsOfArray(map); ++i) {
        if (ch == map[i].ch) {
            bitmap = map[i].bitmap;
            break;
        }
    }
    if (withDot)
        bitSet(bitmap, 7);  // add dot ('.')

    SIPO_shiftByte(~bitmap);    // ~ is for common-anode LEDs
    SIPO_shiftByte(1 << pos);
    SIPO_store();
}


static void Digits_showChar(uint8_t pos, char ch, bool withDot)
{
    if (isDigit(ch))
        Digits_showDigit(pos, ch-'0', withDot);
    else
        Digits_showSymbol(pos, ch, withDot);
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


/** Shifts 8 bits.
 * @param bitmap a map of byte-width bits
 */
static void SIPO_shiftByte(uint8_t bitmap)
{
    shiftOut(_dsPin, _shcpPin, MSBFIRST, bitmap);
}


/** Stores/Loads to 8-bit storage registers. */
static void SIPO_store(void)
{
    digitalWrite(_stcpPin, LOW);
    digitalWrite(_stcpPin, HIGH);
}

//-----------------------------------------------------------------------------

