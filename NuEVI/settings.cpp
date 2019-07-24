#include "settings.h"
#include "globals.h"
#include "menu.h"

//Read settings from eeprom. Returns wether or not anything was written (due to factory reset or upgrade)
bool readEEPROM() {
    bool factoryReset = !digitalRead(ePin) && !digitalRead(mPin);
    bool hasWritten = false;

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
            writeSetting(BCAS_MODE_ADDR, BCAS_MODE_FACTORY);
            writeSetting(DAC_MODE_ADDR, DAC_MODE_FACTORY);
            writeSetting(FASTBOOT_ADDR, FASTBOOT_FACTORY);
        }

        writeSetting(VERSION_ADDR, EEPROM_VERSION);
        hasWritten = true;
    }
 

    // read all settings from EEPROM
    breathThrVal = readSetting(BREATH_THR_ADDR);
    breathMaxVal = readSetting(BREATH_MAX_ADDR);
    portamThrVal = readSetting(PORTAM_THR_ADDR);
    portamMaxVal = readSetting(PORTAM_MAX_ADDR);
    pitchbThrVal = readSetting(PITCHB_THR_ADDR);
    pitchbMaxVal = readSetting(PITCHB_MAX_ADDR);
    transpose    = readSetting(TRANSP_ADDR);
    MIDIchannel  = readSetting(MIDI_ADDR);
    breathCC     = readSetting(BREATH_CC_ADDR);
    breathAT     = readSetting(BREATH_AT_ADDR);
    velocity     = readSetting(VELOCITY_ADDR);
    portamento   = readSetting(PORTAM_ADDR);
    PBdepth      = readSetting(PB_ADDR);
    extraCT      = readSetting(EXTRA_ADDR);
    vibrato      = readSetting(VIBRATO_ADDR);
    deglitch     = readSetting(DEGLITCH_ADDR);
    extracThrVal = readSetting(EXTRAC_THR_ADDR);
    extracMaxVal = readSetting(EXTRAC_MAX_ADDR);
    patch        = readSetting(PATCH_ADDR);
    octave       = readSetting(OCTAVE_ADDR);
    ctouchThrVal = readSetting(CTOUCH_THR_ADDR);
    curve        = readSetting(BREATHCURVE_ADDR);
    velSmpDl     = readSetting(VEL_SMP_DL_ADDR);
    velBias      = readSetting(VEL_BIAS_ADDR);
    pinkySetting = readSetting(PINKY_KEY_ADDR);
    fastPatch[0] = readSetting(FP1_ADDR);
    fastPatch[1] = readSetting(FP2_ADDR);
    fastPatch[2] = readSetting(FP3_ADDR);
    fastPatch[3] = readSetting(FP4_ADDR);
    fastPatch[4] = readSetting(FP5_ADDR);
    fastPatch[5] = readSetting(FP6_ADDR);
    fastPatch[6] = readSetting(FP7_ADDR);
    dipSwBits    = readSetting(DIPSW_BITS_ADDR);
    parallel     = readSetting(PARAL_ADDR);
    rotations[0] = readSetting(ROTN1_ADDR);
    rotations[1] = readSetting(ROTN2_ADDR);
    rotations[2] = readSetting(ROTN3_ADDR);
    rotations[3] = readSetting(ROTN4_ADDR);
    priority     = readSetting(PRIO_ADDR);
    vibSens      = readSetting(VIB_SENS_ADDR);
    vibRetn      = readSetting(VIB_RETN_ADDR);
    vibSquelch   = readSetting(VIB_SQUELCH_ADDR);
    vibDirection = readSetting(VIB_DIRECTION_ADDR);
    breathCC2    = readSetting(BREATH_CC2_ADDR);
    breathCC2Rise   = readSetting(BREATH_CC2_RISE_ADDR);
    vibSensBite     = readSetting(VIB_SENS_BITE_ADDR);
    vibSquelchBite  = readSetting(VIB_SQUELCH_BITE_ADDR);
    vibControl      = readSetting(VIB_CONTROL_ADDR);

    bcasMode = readSetting(BCAS_MODE_ADDR);
    
    trill3_interval = readSetting(TRILL3_INTERVAL_ADDR);
    if(trill3_interval<3 || trill3_interval > 4) trill3_interval = TRILL3_INTERVAL_FACTORY; //Deal with possible bad values
    
    fastBoot = readSetting(FASTBOOT_ADDR);
    dacMode = readSetting(DAC_MODE_ADDR);

    return hasWritten;
}