/*!
 * @file Adafruit_SSD1306.cpp
 *
 * @mainpage Arduino library for monochrome OLEDs based on SSD1306 drivers.
 *
 * @section intro_sec Introduction
 *
 * This is documentation for Adafruit's SSD1306 library for monochrome
 * OLED displays: http://www.adafruit.com/category/63_98
 *
 * These displays use I2C or SPI to communicate. I2C requires 2 pins
 * (SCL+SDA) and optionally a RESET pin. SPI requires 4 pins (MOSI, SCK,
 * select, data/command) and optionally a reset pin. Hardware SPI or
 * 'bitbang' software SPI are both supported.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit-GFX-Library">
 * Adafruit_GFX</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * @section license License
 *
 * BSD license, all text above, and the splash screen included below,
 * must be included in any redistribution.
 *
 */

// #ifdef __AVR__
//  #include <avr/pgmspace.h>
// #elif defined(ESP8266) || defined(ESP32)
//  #include <pgmspace.h>
// #else
 #define pgm_read_byte(addr) \
  (*(const unsigned char *)(addr)) ///< PROGMEM workaround for non-AVR
// #endif

// #if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
//  #include <util/delay.h>
// #endif

#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

// #if defined(BUFFER_LENGTH)
//  #define WIRE_MAX BUFFER_LENGTH          ///< AVR or similar Wire lib
// #elif defined(SERIAL_BUFFER_SIZE)
//  #define WIRE_MAX (SERIAL_BUFFER_SIZE-1) ///< Newer Wire uses RingBuffer
// #else
//  #define WIRE_MAX 32                     ///< Use common Arduino core default
// #endif

#define ssd1306_swap(a, b) \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

// #if ARDUINO >= 100
//  #define WIRE_WRITE wire->write ///< Wire write function in recent Arduino lib
// #else
//  #define WIRE_WRITE wire->send  ///< Wire write function in older Arduino lib
// #endif

// #ifdef HAVE_PORTREG
//  #define SSD1306_SELECT       *csPort &= ~csPinMask; ///< Device select
//  #define SSD1306_DESELECT     *csPort |=  csPinMask; ///< Device deselect
//  #define SSD1306_MODE_COMMAND *dcPort &= ~dcPinMask; ///< Command mode
//  #define SSD1306_MODE_DATA    *dcPort |=  dcPinMask; ///< Data mode
// #else
//  #define SSD1306_SELECT       digitalWrite(csPin, LOW);  ///< Device select
//  #define SSD1306_DESELECT     digitalWrite(csPin, HIGH); ///< Device deselect
//  #define SSD1306_MODE_COMMAND digitalWrite(dcPin, LOW);  ///< Command mode
//  #define SSD1306_MODE_DATA    digitalWrite(dcPin, HIGH); ///< Data mode
// #endif

// #if (ARDUINO >= 157) && !defined(ARDUINO_STM32_FEATHER)
//  #define SETWIRECLOCK wire->setClock(wireClk)    ///< Set before I2C transfer
//  #define RESWIRECLOCK wire->setClock(restoreClk) ///< Restore after I2C xfer
// #else // setClock() is not present in older Arduino Wire lib (or WICED)
//  #define SETWIRECLOCK ///< Dummy stand-in define
//  #define RESWIRECLOCK ///< keeps compiler happy
// #endif

// #if defined(SPI_HAS_TRANSACTION)
//  #define SPI_TRANSACTION_START spi->beginTransaction(spiSettings) ///< Pre-SPI
//  #define SPI_TRANSACTION_END   spi->endTransaction()              ///< Post-SPI
// #else // SPI transactions likewise not present in older Arduino SPI lib
//  #define SPI_TRANSACTION_START ///< Dummy stand-in define
//  #define SPI_TRANSACTION_END   ///< keeps compiler happy
// #endif

// The definition of 'transaction' is broadened a bit in the context of
// this library -- referring not just to SPI transactions (if supported
// in the version of the SPI library being used), but also chip select
// (if SPI is being used, whether hardware or soft), and also to the
// beginning and end of I2C transfers (the Wire clock may be sped up before
// issuing data to the display, then restored to the default rate afterward
// so other I2C device types still work).  All of these are encapsulated
// in the TRANSACTION_* macros.

// Check first if Wire, then hardware SPI, then soft SPI:
// #define TRANSACTION_START   \
//  if(wire) {                 \
//    SETWIRECLOCK;            \
//  } else {                   \
//    if(spi) {                \
//      SPI_TRANSACTION_START; \
//    }                        \
//    SSD1306_SELECT;          \
//  } ///< Wire, SPI or bitbang transfer setup
// #define TRANSACTION_END     \
//  if(wire) {                 \
//    RESWIRECLOCK;            \
//  } else {                   \
//    SSD1306_DESELECT;        \
//    if(spi) {                \
//      SPI_TRANSACTION_END;   \
//    }                        \
//  } ///< Wire, SPI or bitbang transfer end

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for I2C-interfaced SSD1306 displays.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  twi
            Pointer to an existing TwoWire instance (e.g. &Wire, the
            microcontroller's primary I2C bus).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  clkDuring
            Speed (in Hz) for Wire transmissions in SSD1306 library calls.
            Defaults to 400000 (400 KHz), a known 'safe' value for most
            microcontrollers, and meets the SSD1306 datasheet spec.
            Some systems can operate I2C faster (800 KHz for ESP32, 1 MHz
            for many other 32-bit MCUs), and some (perhaps not all)
            SSD1306's can work with this -- so it's optionally be specified
            here and is not a default behavior. (Ignored if using pre-1.5.7
            Arduino software, which operates I2C at a fixed 100 KHz.)
    @param  clkAfter
            Speed (in Hz) for Wire transmissions following SSD1306 library
            calls. Defaults to 100000 (100 KHz), the default Arduino Wire
            speed. This is done rather than leaving it at the 'during' speed
            because other devices on the I2C bus might not be compatible
            with the faster rate. (Ignored if using pre-1.5.7 Arduino
            software, which operates I2C at a fixed 100 KHz.)
    @return Adafruit_SSD1306 object.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_SSD1306::Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi,
  int8_t rst_pin, uint32_t clkDuring, uint32_t clkAfter) :
  Adafruit_GFX(w, h), spi(NULL), wire(twi), buffer(NULL),
  mosiPin(-1), clkPin(-1), dcPin(-1), csPin(-1), rstPin(rst_pin),
  wireClk(clkDuring), restoreClk(clkAfter)
{
}

/*!
    @brief  Constructor for SPI SSD1306 displays, using software (bitbang)
            SPI.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  mosi_pin
            MOSI (master out, slave in) pin (using Arduino pin numbering).
            This transfers serial data from microcontroller to display.
    @param  sclk_pin
            SCLK (serial clock) pin (using Arduino pin numbering).
            This clocks each bit from MOSI.
    @param  dc_pin
            Data/command pin (using Arduino pin numbering), selects whether
            display is receiving commands (low) or data (high).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  cs_pin
            Chip-select pin (using Arduino pin numbering) for sharing the
            bus with other devices. Active low.
    @return Adafruit_SSD1306 object.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_SSD1306::Adafruit_SSD1306(uint8_t w, uint8_t h,
  int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin,
  int8_t cs_pin) : Adafruit_GFX(w, h), spi(NULL), wire(NULL), buffer(NULL),
  mosiPin(mosi_pin), clkPin(sclk_pin), dcPin(dc_pin), csPin(cs_pin),
  rstPin(rst_pin) {
}

/*!
    @brief  Constructor for SPI SSD1306 displays, using native hardware SPI.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  spi
            Pointer to an existing SPIClass instance (e.g. &SPI, the
            microcontroller's primary SPI bus).
    @param  dc_pin
            Data/command pin (using Arduino pin numbering), selects whether
            display is receiving commands (low) or data (high).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  cs_pin
            Chip-select pin (using Arduino pin numbering) for sharing the
            bus with other devices. Active low.
    @param  bitrate
            SPI clock rate for transfers to this display. Default if
            unspecified is 8000000UL (8 MHz).
    @return Adafruit_SSD1306 object.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_SSD1306::Adafruit_SSD1306(uint8_t w, uint8_t h, SPIClass *spi,
  int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t __attribute__((unused)) bitrate) :
  Adafruit_GFX(w, h), spi(spi), wire(NULL), buffer(NULL),
  mosiPin(-1), clkPin(-1), dcPin(dc_pin), csPin(cs_pin), rstPin(rst_pin) {
#ifdef SPI_HAS_TRANSACTION
  spiSettings = SPISettings(bitrate, MSBFIRST, SPI_MODE0);
#endif
}

/*!
    @brief  Destructor for Adafruit_SSD1306 object.
*/
Adafruit_SSD1306::~Adafruit_SSD1306(void) {
  if(buffer) {
    free(buffer);
    buffer = NULL;
  }
}

// LOW-LEVEL UTILS ---------------------------------------------------------

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  vcs
            VCC selection. Pass SSD1306_SWITCHCAPVCC to generate the display
            voltage (step up) from the 3.3V source, or SSD1306_EXTERNALVCC
            otherwise. Most situations with Adafruit SSD1306 breakouts will
            want SSD1306_SWITCHCAPVCC.
    @param  addr
            I2C address of corresponding SSD1306 display (or pass 0 to use
            default of 0x3C for 128x32 display, 0x3D for all others).
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required (pass 0 or any value really,
            it will simply be ignored). Default if unspecified is 0.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple SSD1306 displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @param  periphBegin
            If true, and if a hardware peripheral is being used (I2C or SPI,
            but not software SPI), call that peripheral's begin() function,
            else (false) it has already been done in one's sketch code.
            Cases where false might be used include multiple displays or
            other devices sharing a common bus, or situations on some
            platforms where a nonstandard begin() function is available
            (e.g. a TwoWire interface on non-default pins, as can be done
            on the ESP8266 and perhaps others).
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
boolean Adafruit_SSD1306::begin(uint8_t __attribute__((unused)) vcs, uint8_t __attribute__((unused)) addr, boolean __attribute__((unused)) reset,
                                boolean __attribute__((unused)) periphBegin) {

  if((!buffer) && !(buffer = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    return false;

  enabled_ = true;

  clearDisplay();
//   if(HEIGHT > 32) {
//     drawBitmap((WIDTH - splash1_width) / 2, (HEIGHT - splash1_height) / 2,
//       splash1_data, splash1_width, splash1_height, 1);
//   } else {
//     drawBitmap((WIDTH - splash2_width) / 2, (HEIGHT - splash2_height) / 2,
//       splash2_data, splash2_width, splash2_height, 1);
//   }

  return true; // Success
}

// DRAWING FUNCTIONS -------------------------------------------------------

/*!
    @brief  Set/clear/invert a single pixel. This is also invoked by the
            Adafruit_GFX library in generating many higher-level graphics
            primitives.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  color
            Pixel color, one of: BLACK, WHITE or INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch(getRotation()) {
     case 1:
      ssd1306_swap(x, y);
      x = WIDTH - x - 1;
      break;
     case 2:
      x = WIDTH  - x - 1;
      y = HEIGHT - y - 1;
      break;
     case 3:
      ssd1306_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }
    switch(color) {
     case WHITE:   buffer[x + (y/8)*WIDTH] |=  (1 << (y&7)); break;
     case BLACK:   buffer[x + (y/8)*WIDTH] &= ~(1 << (y&7)); break;
     case INVERSE: buffer[x + (y/8)*WIDTH] ^=  (1 << (y&7)); break;
    }
  }
}

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::clearDisplay(void) {
  memset(buffer, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/*!
    @brief  Draw a horizontal line. This is also invoked by the Adafruit_GFX
            library in generating many higher-level graphics primitives.
    @param  x
            Leftmost column -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  w
            Width of line, in pixels.
    @param  color
            Line color, one of: BLACK, WHITE or INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawFastHLine(
  int16_t x, int16_t y, int16_t w, uint16_t color) {
  boolean bSwap = false;
  switch(rotation) {
   case 1:
    // 90 degree rotation, swap x & y for rotation, then invert x
    bSwap = true;
    ssd1306_swap(x, y);
    x = WIDTH - x - 1;
    break;
   case 2:
    // 180 degree rotation, invert x and y, then shift y around for height.
    x  = WIDTH  - x - 1;
    y  = HEIGHT - y - 1;
    x -= (w-1);
    break;
   case 3:
    // 270 degree rotation, swap x & y for rotation,
    // then invert y and adjust y for w (not to become h)
    bSwap = true;
    ssd1306_swap(x, y);
    y  = HEIGHT - y - 1;
    y -= (w-1);
    break;
  }

  if(bSwap) drawFastVLineInternal(x, y, w, color);
  else      drawFastHLineInternal(x, y, w, color);
}

void Adafruit_SSD1306::drawFastHLineInternal(
  int16_t x, int16_t y, int16_t w, uint16_t color) {

  if((y >= 0) && (y < HEIGHT)) { // Y coord in bounds?
    if(x < 0) { // Clip left
      w += x;
      x  = 0;
    }
    if((x + w) > WIDTH) { // Clip right
      w = (WIDTH - x);
    }
    if(w > 0) { // Proceed only if width is positive
      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x],
               mask = 1 << (y & 7);
      switch(color) {
       case WHITE:               while(w--) { *pBuf++ |= mask; }; break;
       case BLACK: mask = ~mask; while(w--) { *pBuf++ &= mask; }; break;
       case INVERSE:             while(w--) { *pBuf++ ^= mask; }; break;
      }
    }
  }
}

/*!
    @brief  Draw a vertical line. This is also invoked by the Adafruit_GFX
            library in generating many higher-level graphics primitives.
    @param  x
            Column of display -- 0 at left to (screen width -1) at right.
    @param  y
            Topmost row -- 0 at top to (screen height - 1) at bottom.
    @param  h
            Height of line, in pixels.
    @param  color
            Line color, one of: BLACK, WHITE or INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawFastVLine(
  int16_t x, int16_t y, int16_t h, uint16_t color) {
  boolean bSwap = false;
  switch(rotation) {
   case 1:
    // 90 degree rotation, swap x & y for rotation,
    // then invert x and adjust x for h (now to become w)
    bSwap = true;
    ssd1306_swap(x, y);
    x  = WIDTH - x - 1;
    x -= (h-1);
    break;
   case 2:
    // 180 degree rotation, invert x and y, then shift y around for height.
    x = WIDTH  - x - 1;
    y = HEIGHT - y - 1;
    y -= (h-1);
    break;
   case 3:
    // 270 degree rotation, swap x & y for rotation, then invert y
    bSwap = true;
    ssd1306_swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }

  if(bSwap) drawFastHLineInternal(x, y, h, color);
  else      drawFastVLineInternal(x, y, h, color);
}

void Adafruit_SSD1306::drawFastVLineInternal(
  int16_t x, int16_t __y, int16_t __h, uint16_t color) {

  if((x >= 0) && (x < WIDTH)) { // X coord in bounds?
    if(__y < 0) { // Clip top
      __h += __y;
      __y = 0;
    }
    if((__y + __h) > HEIGHT) { // Clip bottom
      __h = (HEIGHT - __y);
    }
    if(__h > 0) { // Proceed only if height is now positive
      // this display doesn't need ints for coordinates,
      // use local byte registers for faster juggling
      uint8_t  y = __y, h = __h;
      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x];

      // do the first partial byte, if necessary - this requires some masking
      uint8_t mod = (y & 7);
      if(mod) {
        // mask off the high n bits we want to set
        mod = 8 - mod;
        // note - lookup table results in a nearly 10% performance
        // improvement in fill* functions
        // uint8_t mask = ~(0xFF >> mod);
        static const uint8_t PROGMEM premask[8] =
          { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        uint8_t mask = pgm_read_byte(&premask[mod]);
        // adjust the mask if we're not going to reach the end of this byte
        if(h < mod) mask &= (0XFF >> (mod - h));

        switch(color) {
         case WHITE:   *pBuf |=  mask; break;
         case BLACK:   *pBuf &= ~mask; break;
         case INVERSE: *pBuf ^=  mask; break;
        }
        pBuf += WIDTH;
      }

      if(h >= mod) { // More to go?
        h -= mod;
        // Write solid bytes while we can - effectively 8 rows at a time
        if(h >= 8) {
          if(color == INVERSE) {
            // separate copy of the code so we don't impact performance of
            // black/white write version with an extra comparison per loop
            do {
              *pBuf ^= 0xFF;  // Invert byte
              pBuf  += WIDTH; // Advance pointer 8 rows
              h     -= 8;     // Subtract 8 rows from height
            } while(h >= 8);
          } else {
            // store a local value to work with
            uint8_t val = (color != BLACK) ? 255 : 0;
            do {
              *pBuf = val;    // Set byte
              pBuf += WIDTH;  // Advance pointer 8 rows
              h    -= 8;      // Subtract 8 rows from height
            } while(h >= 8);
          }
        }

        if(h) { // Do the final partial byte, if necessary
          mod = h & 7;
          // this time we want to mask the low bits of the byte,
          // vs the high bits we did above
          // uint8_t mask = (1 << mod) - 1;
          // note - lookup table results in a nearly 10% performance
          // improvement in fill* functions
          static const uint8_t PROGMEM postmask[8] =
            { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
          uint8_t mask = pgm_read_byte(&postmask[mod]);
          switch(color) {
           case WHITE:   *pBuf |=  mask; break;
           case BLACK:   *pBuf &= ~mask; break;
           case INVERSE: *pBuf ^=  mask; break;
          }
        }
      }
    } // endif positive height
  } // endif x in bounds
}

/*!
    @brief  Return color of a single pixel in display buffer.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @return true if pixel is set (usually WHITE, unless display invert mode
            is enabled), false if clear (BLACK).
    @note   Reads from buffer contents; may not reflect current contents of
            screen if display() has not been called.
*/
boolean Adafruit_SSD1306::getPixel(int16_t x, int16_t y) {
  if((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch(getRotation()) {
     case 1:
      ssd1306_swap(x, y);
      x = WIDTH - x - 1;
      break;
     case 2:
      x = WIDTH  - x - 1;
      y = HEIGHT - y - 1;
      break;
     case 3:
      ssd1306_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }
    return (buffer[x + (y / 8) * WIDTH] & (1 << (y & 7)));
  }
  return false; // Pixel out of bounds
}

/*!
    @brief  Get base address of display buffer for direct reading or writing.
    @return Pointer to an unsigned 8-bit array, column-major, columns padded
            to full byte boundary if needed.
*/
uint8_t *Adafruit_SSD1306::getBuffer(void) {
  return buffer;
}

// REFRESH DISPLAY ---------------------------------------------------------

/*!
    @brief  Push data currently in RAM to SSD1306 display.
    @return None (void).
    @note   Drawing operations are not visible until this function is
            called. Call after each graphics command, or after a whole set
            of graphics commands, as best needed by one's own application.
*/
void Adafruit_SSD1306::display(void) {

  // TODO: Update SDL surface with content of `buffer`

}

// SCROLLING FUNCTIONS -----------------------------------------------------

/*!
    @brief  Activate a right-handed scroll for all or part of the display.
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// To scroll the whole display, run: display.startscrollright(0x00, 0x0F)
void Adafruit_SSD1306::startscrollright(uint8_t __attribute__((unused)) start, uint8_t __attribute__((unused)) stop) {

  // not used for now

}

/*!
    @brief  Activate a left-handed scroll for all or part of the display.
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// To scroll the whole display, run: display.startscrollleft(0x00, 0x0F)
void Adafruit_SSD1306::startscrollleft(uint8_t __attribute__((unused)) start, uint8_t __attribute__((unused)) stop) {
  // not used for now
}

/*!
    @brief  Activate a diagonal scroll for all or part of the display.
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// display.startscrolldiagright(0x00, 0x0F)
void Adafruit_SSD1306::startscrolldiagright(uint8_t __attribute__((unused)) start, uint8_t __attribute__((unused)) stop) {
  // not used for now
}

/*!
    @brief  Activate alternate diagonal scroll for all or part of the display.
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// To scroll the whole display, run: display.startscrolldiagleft(0x00, 0x0F)
void Adafruit_SSD1306::startscrolldiagleft(uint8_t __attribute__((unused)) start, uint8_t __attribute__((unused)) stop) {
  // not used for now
}

/*!
    @brief  Cease a previously-begun scrolling action.
    @return None (void).
*/
void Adafruit_SSD1306::stopscroll(void) {
  // not used for now
}

// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white).
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed, rather a
            different pixel mode of the display hardware is used. When
            enabled, drawing BLACK (value 0) pixels will actually draw white,
            WHITE (value 1) will draw black.
*/
void Adafruit_SSD1306::invertDisplay(boolean i) {
  // not used for now
        inverted_ = i;
}

/*!
    @brief  Dim the display.
    @param  dim
            true to enable lower brightness mode, false for full brightness.
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed.
*/
void Adafruit_SSD1306::dim(boolean dim)
{
  dimmed_ = dim;
}

void Adafruit_SSD1306::ssd1306_command(uint8_t cmd)
{
  // TODO: parse command and act on it...
  switch(cmd)
  {
    case SSD1306_DISPLAYOFF:
      enabled_ = false;
      break;
    case SSD1306_DISPLAYON:
      enabled_ = true;
      break;

    default: break;
  }
}