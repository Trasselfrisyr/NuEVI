#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>


#include "settings.h"
#include "globals.h"
#include "menu.h"
#include "hardware.h"
#include "config.h"
#include "midi.h"
#include "led.h"

//Read settings from eeprom. Returns wether or not anything was written (due to factory reset or upgrade)
void readEEPROM(const bool factoryReset) {

    // if stored settings are not for current version, or Enter+Menu are pressed at startup, they are replaced by factory settings
    uint16_t settingsVersion = readSetting(VERSION_ADDR);

    // blank eeprom will be 0xFFFF. For a full reset, call it "version 0" so everything gets overwritten.
    if (factoryReset || settingsVersion == 0xffffu) {
        settingsVersion = 0;
    }


    if(settingsVersion != EEPROM_VERSION) {

        if(settingsVersion < 24) { //Oldest version from which any settings are recognized
            writeSetting(BREATH_THR_ADDR, BREATH_THR_FACTORY);
            writeSetting(BREATH_MAX_ADDR, BREATH_MAX_FACTORY);
            #if defined(NURAD)
            writeSetting(PORTAM_THR_ADDR, PORTPR_THR_FACTORY);
            writeSetting(PORTAM_MAX_ADDR, PORTPR_MAX_FACTORY);
            #else
            if (digitalRead(biteJumperPin)){ //PBITE (if pulled low with jumper, pressure sensor is used instead of capacitive bite sensing)
                writeSetting(PORTAM_THR_ADDR, PORTAM_THR_FACTORY);
                writeSetting(PORTAM_MAX_ADDR, PORTAM_MAX_FACTORY);
            } else {
                writeSetting(PORTAM_THR_ADDR, PORTPR_THR_FACTORY);
                writeSetting(PORTAM_MAX_ADDR, PORTPR_MAX_FACTORY);
            }
            #endif
            writeSetting(PITCHB_THR_ADDR, PITCHB_THR_FACTORY);
            writeSetting(PITCHB_MAX_ADDR, PITCHB_MAX_FACTORY);
            writeSetting(EXTRAC_THR_ADDR, EXTRAC_THR_FACTORY);
            writeSetting(EXTRAC_MAX_ADDR, EXTRAC_MAX_FACTORY);
            writeSetting(CTOUCH_THR_ADDR, CTOUCH_THR_FACTORY);

            writeSetting(TRANSP_ADDR, TRANSP_FACTORY);
            writeSetting(MIDI_ADDR, MIDI_FACTORY);
            writeSetting(BREATH_CC_ADDR, BREATH_CC_FACTORY);
            writeSetting(BREATH_AT_ADDR, BREATH_AT_FACTORY);
            writeSetting(VELOCITY_ADDR, VELOCITY_FACTORY);
            writeSetting(PORTAM_ADDR, PORTAM_FACTORY);
            writeSetting(PB_ADDR, PB_FACTORY);
            writeSetting(EXTRA_ADDR, EXTRA_FACTORY);
            writeSetting(VIBRATO_ADDR, VIBRATO_FACTORY);
            writeSetting(DEGLITCH_ADDR, DEGLITCH_FACTORY);
            writeSetting(PATCH_ADDR, PATCH_FACTORY);
            writeSetting(OCTAVE_ADDR, OCTAVE_FACTORY);
            writeSetting(BREATHCURVE_ADDR, BREATHCURVE_FACTORY);
            writeSetting(VEL_SMP_DL_ADDR, VEL_SMP_DL_FACTORY);
            writeSetting(VEL_BIAS_ADDR, VEL_BIAS_FACTORY);
            writeSetting(PINKY_KEY_ADDR, PINKY_KEY_FACTORY);
        }

        if(settingsVersion < 26) {
            writeSetting(FP1_ADDR, 0);
            writeSetting(FP2_ADDR, 0);
            writeSetting(FP3_ADDR, 0);
            writeSetting(FP4_ADDR, 0);
            writeSetting(FP5_ADDR, 0);
            writeSetting(FP6_ADDR, 0);
            writeSetting(FP7_ADDR, 0);
            writeSetting(DIPSW_BITS_ADDR, DIPSW_BITS_FACTORY);
        }

        if(settingsVersion < 28) {
            writeSetting(PARAL_ADDR, PARAL_FACTORY);
            writeSetting(ROTN1_ADDR, ROTN1_FACTORY);
            writeSetting(ROTN2_ADDR, ROTN2_FACTORY);
            writeSetting(ROTN3_ADDR, ROTN3_FACTORY);
            writeSetting(ROTN4_ADDR, ROTN4_FACTORY);
            writeSetting(PRIO_ADDR, PRIO_FACTORY);
        }

        if(settingsVersion < 29) {
            writeSetting(VIB_SENS_ADDR, VIB_SENS_FACTORY);
            writeSetting(VIB_RETN_ADDR, VIB_RETN_FACTORY);
        }

        if(settingsVersion < 31) {
            writeSetting(VIB_SQUELCH_ADDR, VIB_SQUELCH_FACTORY);
            writeSetting(VIB_DIRECTION_ADDR, VIB_DIRECTION_FACTORY);
        }

        if(settingsVersion < 32) {
            writeSetting(BREATH_CC2_ADDR, BREATH_CC2_FACTORY);
            writeSetting(BREATH_CC2_RISE_ADDR, BREATH_CC2_RISE_FACTORY);
            writeSetting(VIB_SENS_BITE_ADDR, VIB_SENS_BITE_FACTORY);
            writeSetting(VIB_SQUELCH_BITE_ADDR, VIB_SQUELCH_BITE_FACTORY);
            writeSetting(VIB_CONTROL_ADDR, VIB_CONTROL_FACTORY);
            writeSetting(TRILL3_INTERVAL_ADDR, TRILL3_INTERVAL_FACTORY);
            writeSetting(DAC_MODE_ADDR, DAC_MODE_FACTORY);
        }

        if(settingsVersion < 33) {
            writeSetting(EXTRA2_ADDR, EXTRA2_FACTORY);
            writeSetting(LEVEL_CC_ADDR, LEVEL_CC_FACTORY);
            writeSetting(LEVEL_VAL_ADDR, LEVEL_VAL_FACTORY);
        }
        
        if(settingsVersion < 34) {
            writeSetting(FINGER_ADDR, FINGER_FACTORY);
            writeSetting(LPINKY3_ADDR, LPINKY3_FACTORY);
        }
        
        if(settingsVersion < 35) {
            writeSetting(BATTYPE_ADDR, BATTYPE_FACTORY);
            writeSetting(HARMSET_ADDR, HARMSET_FACTORY);
            writeSetting(HARMSEL_ADDR, HARMSEL_FACTORY);
        }
        
        if(settingsVersion < 36) {
            writeSetting(PARAB_ADDR, PARAB_FACTORY);
            writeSetting(ROTB1_ADDR, ROTB1_FACTORY);
            writeSetting(ROTB2_ADDR, ROTB2_FACTORY);
            writeSetting(ROTB3_ADDR, ROTB3_FACTORY);
            writeSetting(ROTB4_ADDR, ROTB4_FACTORY);
            writeSetting(PARAC_ADDR, PARAC_FACTORY);
            writeSetting(ROTC1_ADDR, ROTC1_FACTORY);
            writeSetting(ROTC2_ADDR, ROTC2_FACTORY);
            writeSetting(ROTC3_ADDR, ROTC3_FACTORY);
            writeSetting(ROTC4_ADDR, ROTC4_FACTORY);
            writeSetting(POLYSEL_ADDR, POLYSEL_FACTORY);
            writeSetting(FWCTYPE_ADDR, FWCTYPE_FACTORY);
            writeSetting(HMZKEY_ADDR, HMZKEY_FACTORY);
        }

        if(settingsVersion < 37) {
            writeSetting(FWCLCH_ADDR, FWCLCH_FACTORY);
            writeSetting(FWCDP2_ADDR, FWCDP2_FACTORY);
        }

        
        if(settingsVersion < 38) {
            writeSetting(HMZLIMIT_ADDR, HMZLIMIT_FACTORY);
        }

        if(settingsVersion < 39) {
            writeSetting(BRINTERV_ADDR, BRINTERV_FACTORY);
            writeSetting(OTFKEY_ADDR, OTFKEY_FACTORY);
        }

        if(settingsVersion < 40) {
            writeSetting(PORTLIMIT_ADDR, PORTLIMIT_FACTORY);
            writeSetting(LEVER_THR_ADDR, LEVER_THR_FACTORY);
            writeSetting(LEVER_MAX_ADDR, LEVER_MAX_FACTORY);
        }

        if(settingsVersion < 41) {
            writeSetting(BRHARMSET_ADDR, BRHARMSET_FACTORY);
            writeSetting(BRHARMSEL_ADDR, BRHARMSEL_FACTORY);
        }

        if(settingsVersion < 42) {
            writeSetting(BITECTL_ADDR, BITECTL_FACTORY);
            writeSetting(BITECC_ADDR, BITECC_FACTORY);
            writeSetting(LEVERCTL_ADDR, LEVERCTL_FACTORY);
            writeSetting(LEVERCC_ADDR, LEVERCC_FACTORY);
        }
        
        if(settingsVersion < 43) {
            writeSetting(CVTUNE_ADDR, CVTUNE_FACTORY);
            writeSetting(CVSCALE_ADDR, CVSCALE_FACTORY);
        }

        if(settingsVersion < 44) {
            writeSetting(CVRATE_ADDR, CVRATE_FACTORY);
        }

        if(settingsVersion < 45) {
            writeSetting(ROLLER_ADDR, ROLLER_FACTORY);
        }

        writeSetting(VERSION_ADDR, EEPROM_VERSION);
    }

    // read all settings from EEPROM
    breathThrVal    = readSettingBounded(BREATH_THR_ADDR, breathLoLimit, breathHiLimit, BREATH_THR_FACTORY);
    breathMaxVal    = readSettingBounded(BREATH_MAX_ADDR, breathLoLimit, breathHiLimit, BREATH_MAX_FACTORY);
    portamThrVal    = readSettingBounded(PORTAM_THR_ADDR, portamLoLimit, portamHiLimit, PORTAM_THR_FACTORY);
    portamMaxVal    = readSettingBounded(PORTAM_MAX_ADDR, portamLoLimit, portamHiLimit, PORTAM_MAX_FACTORY);
    pitchbThrVal    = readSettingBounded(PITCHB_THR_ADDR, pitchbLoLimit, pitchbHiLimit, PITCHB_THR_FACTORY);
    pitchbMaxVal    = readSettingBounded(PITCHB_MAX_ADDR, pitchbLoLimit, pitchbHiLimit, PITCHB_MAX_FACTORY);
    transpose       = readSettingBounded(TRANSP_ADDR, 0, 24, TRANSP_FACTORY);
    MIDIchannel     = readSettingBounded(MIDI_ADDR, 1, 16, MIDI_FACTORY);
    breathCC        = readSettingBounded(BREATH_CC_ADDR, 0, 10, BREATH_CC_FACTORY);
    breathAT        = readSettingBounded(BREATH_AT_ADDR, 0, 1, BREATH_AT_FACTORY);
    velocity        = readSettingBounded(VELOCITY_ADDR, 0, 127, VELOCITY_FACTORY);
    portamento      = readSettingBounded(PORTAM_ADDR, 0, 5, PORTAM_FACTORY);
    PBdepth         = readSettingBounded(PB_ADDR, 0, 12, PB_FACTORY);
    extraCT         = readSettingBounded(EXTRA_ADDR, 0, 4, EXTRA_FACTORY);
    vibrato         = readSettingBounded(VIBRATO_ADDR, 0, 9, VIBRATO_FACTORY);
    deglitch        = readSettingBounded(DEGLITCH_ADDR, 0, 70, DEGLITCH_FACTORY);
    extracThrVal    = readSettingBounded(EXTRAC_THR_ADDR, extracLoLimit, extracHiLimit, EXTRAC_THR_FACTORY);
    extracMaxVal    = readSettingBounded(EXTRAC_MAX_ADDR, extracLoLimit, extracHiLimit, EXTRAC_MAX_FACTORY);
    patch           = readSettingBounded(PATCH_ADDR, 0, 127, PATCH_FACTORY);
    octave          = readSettingBounded(OCTAVE_ADDR, 0, 6, OCTAVE_FACTORY);
    ctouchThrVal    = readSettingBounded(CTOUCH_THR_ADDR, ctouchLoLimit, ctouchHiLimit, CTOUCH_THR_FACTORY);
    curve           = readSettingBounded(BREATHCURVE_ADDR, 0, 12, BREATHCURVE_FACTORY);
    velSmpDl        = readSettingBounded(VEL_SMP_DL_ADDR, 0, 30, VEL_SMP_DL_FACTORY);
    velBias         = readSettingBounded(VEL_BIAS_ADDR, 0, 9, VEL_BIAS_FACTORY);
    pinkySetting    = readSettingBounded(PINKY_KEY_ADDR, 0, 31, PINKY_KEY_FACTORY);
    fastPatch[0]    = readSettingBounded(FP1_ADDR, 0, 127, 0);
    fastPatch[1]    = readSettingBounded(FP2_ADDR, 0, 127, 0);
    fastPatch[2]    = readSettingBounded(FP3_ADDR, 0, 127, 0);
    fastPatch[3]    = readSettingBounded(FP4_ADDR, 0, 127, 0);
    fastPatch[4]    = readSettingBounded(FP5_ADDR, 0, 127, 0);
    fastPatch[5]    = readSettingBounded(FP6_ADDR, 0, 127, 0);
    fastPatch[6]    = readSettingBounded(FP7_ADDR, 0, 127, 0);
    dipSwBits       = readSetting(DIPSW_BITS_ADDR);
    rotations_a.parallel        = readSettingBounded(PARAL_ADDR, 0, 48, PARAL_FACTORY);
    rotations_a.rotations[0]    = readSettingBounded(ROTN1_ADDR, 0, 48, ROTN1_FACTORY);
    rotations_a.rotations[1]    = readSettingBounded(ROTN2_ADDR, 0, 48, ROTN2_FACTORY);
    rotations_a.rotations[2]    = readSettingBounded(ROTN3_ADDR, 0, 48, ROTN3_FACTORY);
    rotations_a.rotations[3]    = readSettingBounded(ROTN4_ADDR, 0, 48, ROTN4_FACTORY);
    priority        = readSettingBounded(PRIO_ADDR, 0, 1, PRIO_FACTORY);
    vibSens         = readSettingBounded(VIB_SENS_ADDR, 1, 12, VIB_SENS_FACTORY);
    vibRetn         = readSettingBounded(VIB_RETN_ADDR, 0, 4, VIB_RETN_FACTORY);
    vibSquelch      = readSettingBounded(VIB_SQUELCH_ADDR, 1, 30, VIB_SQUELCH_FACTORY);
    vibDirection    = readSettingBounded(VIB_DIRECTION_ADDR, 0, 1, VIB_DIRECTION_FACTORY);
    breathCC2       = readSettingBounded(BREATH_CC2_ADDR, 0, 127, BREATH_CC2_FACTORY);
    breathCC2Rise   = readSettingBounded(BREATH_CC2_RISE_ADDR, 1, 10, BREATH_CC2_RISE_FACTORY);
    vibSensBite     = readSettingBounded(VIB_SENS_BITE_ADDR, 1, 17, VIB_SENS_BITE_FACTORY);
    vibSquelchBite  = readSettingBounded(VIB_SQUELCH_BITE_ADDR, 1, 30, VIB_SQUELCH_BITE_FACTORY);
    vibControl      = readSettingBounded(VIB_CONTROL_ADDR, 0, 2, VIB_CONTROL_FACTORY);
    dacMode         = readSettingBounded(DAC_MODE_ADDR, DAC_MODE_BREATH, DAC_MODE_PITCH, DAC_MODE_FACTORY);
    trill3_interval = readSettingBounded(TRILL3_INTERVAL_ADDR, 3, 4, TRILL3_INTERVAL_FACTORY);
    extraCT2        = readSettingBounded(EXTRA2_ADDR, 0, 127, EXTRA2_FACTORY);
    levelCC         = readSettingBounded(LEVEL_CC_ADDR, 0, 127, LEVEL_CC_FACTORY);
    levelVal        = readSettingBounded(LEVEL_VAL_ADDR, 0, 127, LEVEL_VAL_FACTORY);
    #if defined(NURAD)
    fingering       = readSettingBounded(FINGER_ADDR, 0, 4, FINGER_FACTORY);
    #else
    fingering       = readSettingBounded(FINGER_ADDR, 0, 3, FINGER_FACTORY);
    #endif
    lpinky3         = readSettingBounded(LPINKY3_ADDR, 0, 25, LPINKY3_FACTORY);
    batteryType     = readSettingBounded(BATTYPE_ADDR, 0, 2, BATTYPE_FACTORY);
    harmSetting     = readSettingBounded(HARMSET_ADDR, 0, 6, HARMSET_FACTORY);
    harmSelect      = readSettingBounded(HARMSEL_ADDR, 0, 7, HARMSEL_FACTORY);
    polySelect      = (PolySelect)readSettingBounded(POLYSEL_ADDR, 0, 10, POLYSEL_FACTORY);
    fwcType         = readSettingBounded(FWCTYPE_ADDR, 0, 4, FWCTYPE_FACTORY);
    fwcLockH        = readSettingBounded(FWCLCH_ADDR, 0, 1, FWCLCH_FACTORY);
    fwcDrop2        = readSettingBounded(FWCDP2_ADDR, 0, 1, FWCDP2_FACTORY);
    hmzKey          = readSettingBounded(HMZKEY_ADDR, 0, 11, HMZKEY_FACTORY);
    hmzLimit        = readSettingBounded(HMZLIMIT_ADDR, 2, 5, HMZLIMIT_FACTORY);
    rotations_b.parallel       = readSettingBounded(PARAB_ADDR, 0, 48, PARAB_FACTORY);
    rotations_b.rotations[0]   = readSettingBounded(ROTB1_ADDR, 0, 48, ROTB1_FACTORY);
    rotations_b.rotations[1]   = readSettingBounded(ROTB2_ADDR, 0, 48, ROTB2_FACTORY);
    rotations_b.rotations[2]   = readSettingBounded(ROTB3_ADDR, 0, 48, ROTB3_FACTORY);
    rotations_b.rotations[3]   = readSettingBounded(ROTB4_ADDR, 0, 48, ROTB4_FACTORY);
    rotations_c.parallel       = readSettingBounded(PARAC_ADDR, 0, 48, PARAC_FACTORY);
    rotations_c.rotations[0]   = readSettingBounded(ROTC1_ADDR, 0, 48, ROTC1_FACTORY);
    rotations_c.rotations[1]   = readSettingBounded(ROTC2_ADDR, 0, 48, ROTC2_FACTORY);
    rotations_c.rotations[2]   = readSettingBounded(ROTC3_ADDR, 0, 48, ROTC3_FACTORY);
    rotations_c.rotations[3]   = readSettingBounded(ROTC4_ADDR, 0, 48, ROTC4_FACTORY);
    otfKey          = readSettingBounded(OTFKEY_ADDR, 0, 1, OTFKEY_FACTORY);
    breathInterval  = readSettingBounded(BRINTERV_ADDR, 3, 15, BRINTERV_FACTORY);
    portLimit       = readSettingBounded(PORTLIMIT_ADDR, 1, 127, PORTLIMIT_FACTORY);
    leverThrVal     = readSettingBounded(LEVER_THR_ADDR, leverLoLimit, leverHiLimit, LEVER_THR_FACTORY);
    leverMaxVal     = readSettingBounded(LEVER_MAX_ADDR, leverLoLimit, leverHiLimit, LEVER_MAX_FACTORY);
    brHarmSetting   = readSettingBounded(BRHARMSET_ADDR, 0, 6, BRHARMSET_FACTORY);
    brHarmSelect    = readSettingBounded(BRHARMSEL_ADDR, 0, 3, BRHARMSEL_FACTORY);
    biteControl     = readSettingBounded(BITECTL_ADDR, 0, 3, BITECTL_FACTORY);
    leverControl    = readSettingBounded(LEVERCTL_ADDR, 0, 3, LEVERCTL_FACTORY);
    biteCC          = readSettingBounded(BITECC_ADDR, 0, 127, BITECC_FACTORY);
    leverCC         = readSettingBounded(LEVERCC_ADDR, 0, 127, LEVERCC_FACTORY);
    cvTune          = readSettingBounded(CVTUNE_ADDR, 1, 199, CVTUNE_FACTORY);
    cvScale         = readSettingBounded(CVSCALE_ADDR, 1, 199, CVSCALE_FACTORY);
    cvVibRate       = readSettingBounded(CVRATE_ADDR, 0, 8, CVRATE_FACTORY);
    rollerMode      = readSettingBounded(ROLLER_ADDR, 0, 3, ROLLER_FACTORY);
    
    //Flags stored in bit field
    fastBoot         = (dipSwBits & (1<<DIPSW_FASTBOOT))?1:0;
    legacy           = (dipSwBits & (1<<DIPSW_LEGACY))?1:0;
    legacyBrAct      = (dipSwBits & (1<<DIPSW_LEGACYBRACT))?1:0;
    widiOn           = (dipSwBits & (1<<DIPSW_WIDION))?1:0;
    gateOpenEnable   = (dipSwBits & (1<<DIPSW_GATEOPEN))?1:0;
    specialKeyEnable = (dipSwBits & (1<<DIPSW_SPKEYENABLE))?1:0;
    bcasMode         = (dipSwBits & (1<<DIPSW_BCASMODE))?1:0;

}



//Poke at a certain bit in a bit field
void setBit(uint16_t &bitfield, const uint8_t pos, const uint16_t value) {
  bitfield = (bitfield & ~(1<<pos)) | ((value?1:0)<<pos);
}


//Read and write EEPROM data
void writeSetting(const uint16_t address, const uint16_t value) {
    union {
        uint8_t v[2];
        uint16_t val;
    } data;
    data.val = value;
    EEPROM.update(address, data.v[0]);
    EEPROM.update(address+1, data.v[1]);
}

uint16_t readSetting(const uint16_t address) {
    union {
        uint8_t v[2];
        uint16_t val;
    } data;
    data.v[0] = EEPROM.read(address);
    data.v[1] = EEPROM.read(address+1);
    return data.val;
}

uint16_t readSettingBounded(const uint16_t address, const uint16_t min, const uint16_t max, const uint16_t defaultValue) {
    uint16_t val = readSetting(address);
    if(val < min || val > max) {
        val = defaultValue;
        writeSetting(address, val);
    }
    return val;
}




//Functions to send and receive config (and other things) via USB MIDI SysEx messages
uint32_t crc32(const uint8_t *message, const size_t length) {
   size_t pos=0;
   uint32_t crc=0xFFFFFFFF;

   while (pos<length) {
      crc ^= message[pos++]; //Get next byte and increment position
      for (uint8_t j=0; j<8; ++j) { //Mask off 8 next bits
         crc = (crc >> 1) ^ (0xEDB88320 &  -(crc & 1));
      }
   }
   return ~crc;
}


/*

Send EEPROM config dump as sysex message. Message format is structured like this:

+------------------------------------------------------------------------------------+
| vendor(3) | "NuEVIc01" (8) | Payload size (2) | EEPROM data (variable) | crc32 (4) |
+------------------------------------------------------------------------------------+

Payload size is for the EEPROM data chunk (not including anything else before or after
CRC32 covers the entire buffer up to and including the eeprom data (but not the checksum itself)

This currently operates under the assumption that the whole EEPROM chunk only consists of unsigned 16 bit ints, only using the range 0-16383

*/
void sendSysexSettings() {
  const char *header = "NuEVIc01"; //NuEVI config dump 01

  //Build a send buffer of all the things
  size_t sysex_size = 3 + strlen(header) + 2 + EEPROM_SIZE + 4;
  uint8_t *sysex_data = (uint8_t*)malloc(sysex_size);

  //Positions (offsets) of parts in send buffer
  int header_pos = 3;
  int size_pos = header_pos + strlen(header);
  int payload_pos = size_pos + 2;
  int checksum_pos = payload_pos + EEPROM_SIZE;

  //SysEX manufacturer ID
  memcpy(sysex_data, sysex_id, 3);

  //Header with command code
  memcpy(sysex_data+header_pos, header, strlen(header));

  //Payload length
  *(uint16_t*)(sysex_data+size_pos) = convertToMidiValue(EEPROM_SIZE);

  //Config data
  uint16_t* config_buffer_start = (uint16_t*)(sysex_data+payload_pos);

  //Read one settings item at a time, change data format, and put in send buffer
  for(uint16_t idx=0; idx<EEPROM_SIZE/2; idx++) {
    uint16_t eepromval = readSetting(idx*2);
    config_buffer_start[idx] = convertToMidiValue(eepromval);
  }

  uint32_t checksum = crc32(sysex_data, checksum_pos);

  *(uint32_t*)(sysex_data+checksum_pos) = convertToMidiCRC(checksum);

  usbMIDI.sendSysEx(sysex_size, sysex_data);

  free(sysex_data);
}

//Send a simple 3-byte message code as sysex
void sendSysexMessage(const char* messageCode) {
  char sysexMessage[] = "vvvNuEVIccc"; //Placeholders for vendor and code

  memcpy(sysexMessage, sysex_id, 3);
  memcpy(sysexMessage+8, messageCode, 3);

  usbMIDI.sendSysEx(11, (const uint8_t *)sysexMessage);
}


bool receiveSysexSettings(const uint8_t* data, const uint16_t length) {

  //Expected size of data (vendor+NuEVIc02+len+payload+crc32)
  uint16_t expected_size = 3 + 8 + 2 + EEPROM_SIZE + 4;


  //Positions (offsets) of parts in buffer
  int size_pos = 11;
  int payload_pos = size_pos + 2;
  int checksum_pos = payload_pos + EEPROM_SIZE;

  //Make sure length of receive buffer is enough to read all we need to. We can accept extra junk at the end though.
  if(length<expected_size) {
    configShowMessage("Invalid config format");
    return false;
  }

  //No need to verify vendor or header/command, already done before we get here.

  //Calculate checksum of stuff received (everything before checksum), transform to midi format
  //(being a one-way operation, we can't do the reverse anyway)
  uint32_t crc=convertToMidiCRC(crc32(data, checksum_pos));
  uint32_t crc_rcv;
  memcpy(&crc_rcv, data+checksum_pos, 4);
  if(crc != crc_rcv && crc_rcv != NO_CHECKSUM) {
    configShowMessage("Invalid checksum");
    return false;
  }

  //Verify that payload size matches the size of our EEPROM config
  uint16_t payload_size = convertFromMidiValue(data+size_pos);
  if(payload_size != EEPROM_SIZE) {
    configShowMessage("Invalid config size");
    return false;
  }


  uint16_t eeprom_version_rcv = convertFromMidiValue(data+(payload_pos+VERSION_ADDR));
  if(eeprom_version_rcv != EEPROM_VERSION) {
    configShowMessage("Invalid config version");
    return false;
  }

  //Grab all the items in payload and save to EEPROM
  for(uint16_t i=0; i<payload_size/2; i++) {
    uint16_t addr = i*2;
    uint16_t val;
    val = convertFromMidiValue(data+(payload_pos+addr));

    //Skip sensor calibration values if they are "out of bounds". This makes it possible to send a config that does
    //not overwrite sensor calibration.
    if(addr == BREATH_THR_ADDR || addr == BREATH_MAX_ADDR) {
      if(val<breathLoLimit || val>breathHiLimit) continue;
    }

    if(addr == PORTAM_THR_ADDR || addr == PORTAM_MAX_ADDR) {
      if(val<portamLoLimit || val>portamHiLimit) continue;
    }

    if(addr == PITCHB_THR_ADDR || addr == PITCHB_MAX_ADDR) {
      if(val<pitchbLoLimit || val>pitchbHiLimit) continue;
    }

    if(addr == EXTRAC_THR_ADDR || addr == EXTRAC_MAX_ADDR) {
      if(val<extracLoLimit || val>extracHiLimit) continue;
    }

    if(addr == CTOUCH_THR_ADDR) {
      if(val<ctouchLoLimit || val>ctouchHiLimit) continue;
    }

    writeSetting(addr, val);
  }

  //All went well
  return true;
}

//Send EEPROM and firmware versions
void sendSysexVersion() {
  char sysexMessage[] = "vvvNuEVIc04eevvvvvvvv"; //Placeholders for vendor and code
  uint8_t fwStrLen = min(strlen(FIRMWARE_VERSION), 8); //Limit firmware version string to 8 bytes

  memcpy(sysexMessage, sysex_id, 3);
  memcpy(sysexMessage+13, FIRMWARE_VERSION, fwStrLen);

  *(uint16_t*)(sysexMessage+11) = convertToMidiValue(EEPROM_VERSION);

  uint8_t message_length = 13+fwStrLen;

  usbMIDI.sendSysEx(message_length, (const uint8_t *)sysexMessage);
}

extern Adafruit_SSD1306 display;

void configShowMessage(const char* message) {
  display.fillRect(0,32,128,64,BLACK);
  display.setCursor(0,32);
  display.setTextColor(WHITE);

  display.print(message);

  display.display();
}

uint8_t* sysex_rcv_buffer = NULL;
uint16_t sysex_buf_size = 0;


void handleSysexChunk(const uint8_t *data, uint16_t length, bool last) {
  uint16_t pos;

  if(!sysex_rcv_buffer) {
    //Start out with an empty buffer
    pos = 0;
    sysex_buf_size = length;
    sysex_rcv_buffer = (uint8_t *)malloc(sysex_buf_size);
  } else {
    //Increase size of current buffer
    pos = sysex_buf_size;
    sysex_buf_size += length;
    sysex_rcv_buffer = (uint8_t *)realloc(sysex_rcv_buffer, sysex_buf_size);
  }

  //Append this chunk to buffer
  memcpy(sysex_rcv_buffer + pos, data, length);

  //If it's the last one, call the regular handler to process it
  if(last) {
    handleSysex(sysex_rcv_buffer, sysex_buf_size);

    //Discard the buffer
    free(sysex_rcv_buffer);
    sysex_rcv_buffer = NULL;
    sysex_buf_size = 0;
  }
}


void handleSysex(uint8_t *data, unsigned int length) {
  //Note: Sysex data as received here contains sysex start and end markers (0xF0 and 0xF7)

  //Too short to even contain a 3-byte vendor id is not for us.
  if(length<4) return;

  //Verify vendor
  if(strncmp((char*)(data+1), sysex_id, 3)) return; //Silently ignore different vendor id

  //Verify header. Min length is 3+5+3 bytes (vendor+header+message code)
  if(length<12 || strncmp((char*)(data+4), "NuEVI", 5)) {
    configShowMessage("Invalid message.");
    sendSysexMessage("e00");
    return;
  }

  //Get message code
  char messageCode[3];
  strncpy(messageCode, (char*)(data+9), 3);

  if(!strncmp(messageCode, "c00", 3)) { //Config dump request
    configShowMessage("Sending config...");
    sendSysexSettings();
    configShowMessage("Config sent.");
  } else if(!strncmp(messageCode, "c03", 3)) { //Version info request
    configShowMessage("Sending version.");
    sendSysexVersion();
  } else if(!strncmp(messageCode, "c02", 3)) { //New config incoming
    configShowMessage("Receiving config...");

    //Tell receiveSysexSettings about what's between sysex start and end markers
    if(receiveSysexSettings(data+1, length-2)) configShowMessage("New config saved.");
  } else {
    configShowMessage("Unknown message.");
    sendSysexMessage("e01"); //Unimplemented message code
  }
}

void configModeSetup() {
    statusLedFlash(500);

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.setTextSize(0);

    display.println("Config mgmt");
    display.println("Power off NuEVI");
    display.println("to exit");
    display.display();

    usbMIDI.setHandleSystemExclusive(handleSysexChunk);

    statusLedFlash(500);

    sendSysexVersion(); //Friendly hello

    configShowMessage("Ready.");
}

//"Main loop". Just sits and wait for midi messages and lets the sysex handler do all the work.
void configModeLoop() {
    usbMIDI.read();
}
