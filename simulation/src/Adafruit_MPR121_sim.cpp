/*!
 * @file Adafruit_MPR121.cpp
 *
 * @mainpage Adafruit MPR121 arduino driver
 *
 * @section intro_sec Introduction
 *
  This is a library for the MPR121 I2C 12-chan Capacitive Sensor

  Designed specifically to work with the MPR121 sensor from Adafruit
  ----> https://www.adafruit.com/products/1982

  These sensors use I2C to communicate, 2+ pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.  
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_MPR121.h"

/**
 *****************************************************************************************
 *  @brief      Default constructor
 ****************************************************************************************/
Adafruit_MPR121::Adafruit_MPR121() {
}

/**
 *****************************************************************************************
 *  @brief      Begin an MPR121 object on a given I2C bus. This function resets the
 *              device and writes the default settings.
 *
 *  @param      i2caddr the i2c address the device can be found on. Defaults to 0x5A.
 *  @returns    true on success, false otherwise
 ****************************************************************************************/
boolean Adafruit_MPR121::begin(uint8_t i2caddr) {
  _i2caddr = i2caddr;

  return true;
}

/**
 *****************************************************************************************
 *  @brief      DEPRECATED. Use Adafruit_MPR121::setThresholds(uint8_t touch, uint8_t release) instead.
 *
 *  @param      touch see Adafruit_MPR121::setThresholds(uint8_t touch, uint8_t release)
 *  @param      release see Adafruit_MPR121::setThresholds(uint8_t touch, uint8_t release)
 ****************************************************************************************/
void Adafruit_MPR121::setThreshholds(uint8_t __attribute__((unused)) touch, uint8_t __attribute__((unused)) release) {
  // setThresholds(touch, release);
}

/**
 *****************************************************************************************
 *  @brief      Set the touch and release thresholds for all 13 channels on the device to the
 *              passed values. The threshold is defined as a deviation value from the baseline value, 
 *              so it remains constant even baseline value changes. Typically the touch 
 *              threshold is a little bigger than the release threshold to touch debounce and hysteresis.
 * 
 *              For typical touch application, the value can be in range 0x05~0x30 for example. The setting 
 *              of the threshold is depended on the actual application. For the operation details and how to set the threshold refer to
 *              application note AN3892 and MPR121 design guidelines.
 *
 *  @param      touch the touch threshold value from 0 to 255.
 *  @param      release the release threshold from 0 to 255.
 ****************************************************************************************/
void Adafruit_MPR121::setThresholds(uint8_t __attribute__((unused)) touch, __attribute__((unused)) uint8_t release) {
  // for (uint8_t i=0; i<12; i++) {
  //   writeRegister(MPR121_TOUCHTH_0 + 2*i, touch);
  //   writeRegister(MPR121_RELEASETH_0 + 2*i, release);
  // }
}

/**
 *****************************************************************************************
 *  @brief      Read the filtered data from channel t. The ADC raw data outputs run through 3 
 *              levels of digital filtering to filter out the high frequency and low frequency noise
 *              encountered. For detailed information on this filtering see page 6 of the device datasheet.
 *
 *  @param      t the channel to read
 *  @returns    the filtered reading as a 10 bit unsigned value
 ****************************************************************************************/
uint16_t  Adafruit_MPR121::filteredData(uint8_t t) {
  if (t > 12) return 0;
  return readRegister16(MPR121_FILTDATA_0L + t*2);
}

/**
 *****************************************************************************************
 *  @brief      Read the baseline value for the channel. The 3rd level filtered result is internally 10bit 
 *              but only high 8 bits are readable from registers 0x1E~0x2A as the baseline value output for each channel.
 * 
 *  @param      t the channel to read.
 *  @returns    the baseline data that was read
 ****************************************************************************************/
uint16_t  Adafruit_MPR121::baselineData(uint8_t t) {
  if (t > 12) return 0;
  uint16_t bl = readRegister8(MPR121_BASELINE_0 + t);
  return (bl << 2);
}

/**
 *****************************************************************************************
 *  @brief      Read the touch status of all 13 channels as bit values in a 12 bit integer.
 *              
 *  @returns    a 12 bit integer with each bit corresponding to the touch status of a sensor.
 *              For example, if bit 0 is set then channel 0 of the device is currently deemed to be touched.
 ****************************************************************************************/
uint16_t  Adafruit_MPR121::touched(void) {
  uint16_t t = readRegister16(MPR121_TOUCHSTATUS_L);
  return t & 0x0FFF;
}

/*********************************************************************/

/**
 *****************************************************************************************
 *  @brief      Read the contents of an 8 bit device register.
 * 
 *  @param      reg the register address to read from
 *  @returns    the 8 bit value that was read.
 ****************************************************************************************/
uint8_t Adafruit_MPR121::readRegister8(uint8_t reg) {

    return this->_registers[reg];
    // Wire.beginTransmission(_i2caddr);
    // Wire.write(reg);
    // Wire.endTransmission(false);
    // Wire.requestFrom(_i2caddr, 1);
    // if (Wire.available() < 1)
    //   return 0;
    // return (Wire.read());
}

/**
 *****************************************************************************************
 *  @brief      Read the contents of a 16 bit device register.
 * 
 *  @param      reg the register address to read from
 *  @returns    the 16 bit value that was read.
 ****************************************************************************************/
uint16_t Adafruit_MPR121::readRegister16(uint8_t reg) {
    return _registers[reg] | (_registers[reg+1] << 8);
    // Wire.beginTransmission(_i2caddr);
    // Wire.write(reg);
    // Wire.endTransmission(false);
    // Wire.requestFrom(_i2caddr, 2);
    // if (Wire.available() < 2)
    //   return 0;
    // uint16_t v = Wire.read();
    // v |=  ((uint16_t) Wire.read()) << 8;
    // return v;
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register

    @param  reg the register address to write to
    @param  value the value to write
*/
/**************************************************************************/
void Adafruit_MPR121::writeRegister(uint8_t reg, uint8_t value) {

    _registers[reg] = value;
    // Wire.beginTransmission(_i2caddr);
    // Wire.write((uint8_t)reg);
    // Wire.write((uint8_t)(value));
    // Wire.endTransmission();
}


/*
  Simulator specifics code..
 */

