/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 * Copyright (c) 2017 Victor Perez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * Software SPI functions originally from Arduino Sd2Card Library
 * Copyright (c) 2009 by William Greiman
 */

#include "../../sd/cardreader.h"

/**
 * Adapted to the STM32F1 HAL
 */

#ifdef __STM32F1__

#include "../../inc/MarlinConfig.h"
#include <SPI.h>

// ------------------------
// Public Variables
// ------------------------

static SPISettings spiConfig;

// ------------------------
// Public functions
// ------------------------

#if ENABLED(SOFTWARE_SPI)

  // ------------------------
  // Software SPI
  // ------------------------
  #error "Software SPI not supported for STM32F1. Use hardware SPI."

#else

// ------------------------
// Hardware SPI
// ------------------------

/**
 * VGPV SPI speed start and F_CPU/2, by default 72/2 = 36Mhz
 */

/**
 * @brief  Begin SPI port setup
 *
 * @return Nothing
 *
 * @details Only configures SS pin since libmaple creates and initialize the SPI object
 */
void spiBegin() {
  #if !PIN_EXISTS(SS)
    #error "SS_PIN not defined!"
  #endif
  OUT_WRITE(SS_PIN, HIGH);
}

/**
 * @brief  Initialize SPI port to required speed rate and transfer mode (MSB, SPI MODE 0)
 *
 * @param  spiRate Rate as declared in HAL.h (speed do not match AVR)
 * @return Nothing
 *
 * @details
 */
void spiInit(uint8_t spiRate) {
  uint8_t  clock;
  switch (spiRate) {
    case SPI_FULL_SPEED:    clock = SPI_CLOCK_DIV2 ; break;
    case SPI_HALF_SPEED:    clock = SPI_CLOCK_DIV4 ; break;
    case SPI_QUARTER_SPEED: clock = SPI_CLOCK_DIV8 ; break;
    case SPI_EIGHTH_SPEED:  clock = SPI_CLOCK_DIV16; break;
    case SPI_SPEED_5:       clock = SPI_CLOCK_DIV32; break;
    case SPI_SPEED_6:       clock = SPI_CLOCK_DIV64; break;
    default:                clock = SPI_CLOCK_DIV2; // Default from the SPI library
  }
  
  spiConfig = SPISettings(clock, MSBFIRST, SPI_MODE0);
  
  SPI.setModule(SPI_DEVICE);
  SPI.begin();
  SPI.setClockDivider(clock);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
}

/**
 * @brief  Receive a single byte from the SPI port.
 *
 * @return Byte received
 *
 * @details
 */
uint8_t spiRec(void) {
  uint8_t returnByte;  
  if(card.isFileOpen()) {
    SPI.beginTransaction(spiConfig);
    returnByte = SPI.transfer(0xFF);
    SPI.endTransaction();
  }
  else {
    returnByte = SPI.transfer(0xFF);
  }
  
  return returnByte;
}

/**
 * @brief  Receive a number of bytes from the SPI port to a buffer
 *
 * @param  buf   Pointer to starting address of buffer to write to.
 * @param  nbyte Number of bytes to receive.
 * @return Nothing
 *
 * @details Uses DMA
 */
void spiRead(uint8_t* buf, uint16_t nbyte) {  
  if(card.isFileOpen()) {
    SPI.beginTransaction(spiConfig);
    SPI.dmaTransfer(0, const_cast<uint8_t*>(buf), nbyte);
    SPI.endTransaction();
  }
  else {
    SPI.dmaTransfer(0, const_cast<uint8_t*>(buf), nbyte);
  }
}

/**
 * @brief  Send a single byte on SPI port
 *
 * @param  b Byte to send
 *
 * @details
 */
void spiSend(uint8_t b) {  
  if(card.isFileOpen()) {
    SPI.beginTransaction(spiConfig);
    SPI.send(b);
    SPI.endTransaction();
  }
  else {
    SPI.send(b);
  }
}

/**
 * @brief  Write token and then write from 512 byte buffer to SPI (for SD card)
 *
 * @param  buf   Pointer with buffer start address
 * @return Nothing
 *
 * @details Use DMA
 */
void spiSendBlock(uint8_t token, const uint8_t* buf) {  
  if(card.isFileOpen()) {
    SPI.beginTransaction(spiConfig);
    SPI.send(token);
    SPI.dmaSend(const_cast<uint8_t*>(buf), 512);
    SPI.endTransaction();
  }
  else {
    SPI.send(token);
    SPI.dmaSend(const_cast<uint8_t*>(buf), 512);
  }
}

#if ENABLED(SPI_EEPROM)

// Read single byte from specified SPI channel
uint8_t spiRec(uint32_t chan) { return SPI.transfer(ff); }

// Write single byte to specified SPI channel
void spiSend(uint32_t chan, byte b) { SPI.send(b); }

// Write buffer to specified SPI channel
void spiSend(uint32_t chan, const uint8_t* buf, size_t n) {
  for (size_t p = 0; p < n; p++) spiSend(chan, buf[p]);
}

#endif // SPI_EEPROM

#endif // SOFTWARE_SPI

#endif // __STM32F1__
