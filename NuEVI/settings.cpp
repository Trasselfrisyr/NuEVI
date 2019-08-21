#include <Arduino.h>
#include <EEPROM.h>

#include "settings.h"
#include "globals.h"
#include "menu.h"
#include "hardware.h"
#include "config.h"

//Read settings from eeprom. Returns wether or not anything was written (due to factory reset or upgrade)
void readEEPROM() {
    bool factoryReset = !digitalRead(ePin) && !digitalRead(mPin);

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
            if (digitalRead(biteJumperPin)){ //PBITE (if pulled low with jumper, pressure sensor is used instead of capacitive bite sensing)
                writeSetting(PORTAM_THR_ADDR, PORTAM_THR_FACTORY);  
                writeSetting(PORTAM_MAX_ADDR, PORTAM_MAX_FACTORY); 
            } else {
                writeSetting(PORTAM_THR_ADDR, PORTPR_THR_FACTORY);  
                writeSetting(PORTAM_MAX_ADDR, PORTPR_MAX_FACTORY); 
            }
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
    breathCC        = readSettingBounded(BREATH_CC_ADDR, 0, 127, BREATH_CC_FACTORY);
    breathAT        = readSettingBounded(BREATH_AT_ADDR, 0, 1, BREATH_AT_FACTORY);
    velocity        = readSettingBounded(VELOCITY_ADDR, 0, 127, VELOCITY_FACTORY);
    portamento      = readSettingBounded(PORTAM_ADDR, 0, 2, PORTAM_FACTORY);
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
    pinkySetting    = readSettingBounded(PINKY_KEY_ADDR, 0, 28, PINKY_KEY_FACTORY);
    fastPatch[0]    = readSettingBounded(FP1_ADDR, 0, 127, 0);
    fastPatch[1]    = readSettingBounded(FP2_ADDR, 0, 127, 0);
    fastPatch[2]    = readSettingBounded(FP3_ADDR, 0, 127, 0);
    fastPatch[3]    = readSettingBounded(FP4_ADDR, 0, 127, 0);
    fastPatch[4]    = readSettingBounded(FP5_ADDR, 0, 127, 0);
    fastPatch[5]    = readSettingBounded(FP6_ADDR, 0, 127, 0);
    fastPatch[6]    = readSettingBounded(FP7_ADDR, 0, 127, 0);
    dipSwBits       = readSetting(DIPSW_BITS_ADDR);
    parallel        = readSettingBounded(PARAL_ADDR, 0, 48, PARAL_FACTORY);
    rotations[0]    = readSettingBounded(ROTN1_ADDR, 0, 48, ROTN1_FACTORY);
    rotations[1]    = readSettingBounded(ROTN2_ADDR, 0, 48, ROTN2_FACTORY);
    rotations[2]    = readSettingBounded(ROTN3_ADDR, 0, 48, ROTN3_FACTORY);
    rotations[3]    = readSettingBounded(ROTN4_ADDR, 0, 48, ROTN4_FACTORY);
    priority        = readSettingBounded(PRIO_ADDR, 0, 1, PRIO_FACTORY);
    vibSens         = readSettingBounded(VIB_SENS_ADDR, 1, 12, VIB_SENS_FACTORY);
    vibRetn         = readSettingBounded(VIB_RETN_ADDR, 0, 4, VIB_RETN_FACTORY);
    vibSquelch      = readSettingBounded(VIB_SQUELCH_ADDR, 1, 30, VIB_SQUELCH_FACTORY);
    vibDirection    = readSettingBounded(VIB_DIRECTION_ADDR, 0, 1, VIB_DIRECTION_FACTORY);
    breathCC2       = readSettingBounded(BREATH_CC2_ADDR, 0, 127, BREATH_CC2_FACTORY);
    breathCC2Rise   = readSettingBounded(BREATH_CC2_RISE_ADDR, 1, 10, BREATH_CC2_RISE_FACTORY);
    vibSensBite     = readSettingBounded(VIB_SENS_BITE_ADDR, 1, 12, VIB_SENS_BITE_FACTORY);
    vibSquelchBite  = readSettingBounded(VIB_SQUELCH_BITE_ADDR, 1, 30, VIB_SQUELCH_BITE_FACTORY);
    vibControl      = readSettingBounded(VIB_CONTROL_ADDR, 0, 1, VIB_CONTROL_FACTORY);
    dacMode         = readSettingBounded(DAC_MODE_ADDR, DAC_MODE_BREATH, DAC_MODE_PITCH, DAC_MODE_FACTORY);
    trill3_interval = readSettingBounded(TRILL3_INTERVAL_ADDR, 3, 4, TRILL3_INTERVAL_FACTORY);
    extraCT2        = readSettingBounded(EXTRA2_ADDR, 0, 127, EXTRA2_FACTORY);
    levelCC         = readSettingBounded(LEVEL_CC_ADDR, 0, 127, LEVEL_CC_FACTORY);
    levelVal        = readSettingBounded(LEVEL_VAL_ADDR, 0, 127, LEVEL_VAL_FACTORY);

    //Flags stored in bit field
    fastBoot         = (dipSwBits & (1<<DIPSW_FASTBOOT))?1:0;
    legacy           = (dipSwBits & (1<<DIPSW_LEGACY))?1:0;
    legacyBrAct      = (dipSwBits & (1<<DIPSW_LEGACYBRACT))?1:0;
    slowMidi         = (dipSwBits & (1<<DIPSW_SLOWMIDI))?1:0;
    gateOpenEnable   = (dipSwBits & (1<<DIPSW_GATEOPEN))?1:0;
    specialKeyEnable = (dipSwBits & (1<<DIPSW_SPKEYENABLE))?1:0;
    bcasMode         = (dipSwBits & (1<<DIPSW_BCASMODE))?1:0;

}



//Poke at a certain bit in a bit field
void setBit(uint16_t &bitfield, const uint8_t pos, const uint16_t value) {
  bitfield = (bitfield & ~(1<<pos)) | ((value?1:0)<<pos);
}


//Read and write EEPROM data
void writeSetting(uint16_t address, uint16_t value) {
    union {
        uint8_t v[2];
        uint16_t val;
    } data;
    data.val = value;
    EEPROM.update(address, data.v[0]);
    EEPROM.update(address+1, data.v[1]);
}

uint16_t readSetting(uint16_t address) {
    union {
        uint8_t v[2];
        uint16_t val;
    } data;
    data.v[0] = EEPROM.read(address);
    data.v[1] = EEPROM.read(address+1);
    return data.val;
}

uint16_t readSettingBounded(uint16_t address, uint16_t min, uint16_t max, uint16_t defaultValue) {
    uint16_t val = readSetting(address);
    if(val < min || val > max) {
        val = defaultValue;
        writeSetting(address, val);
    }
    return val;
}
