#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <stdint.h>

// EEPROM addresses for settings
#define VERSION_ADDR 0
#define BREATH_THR_ADDR 2
#define BREATH_MAX_ADDR 4
#define PORTAM_THR_ADDR 6
#define PORTAM_MAX_ADDR 8
#define PITCHB_THR_ADDR 10
#define PITCHB_MAX_ADDR 12
#define TRANSP_ADDR 14
#define MIDI_ADDR 16
#define BREATH_CC_ADDR 18
#define BREATH_AT_ADDR 20
#define VELOCITY_ADDR 22
#define PORTAM_ADDR 24
#define PB_ADDR 26
#define EXTRA_ADDR 28
#define VIBRATO_ADDR 30
#define DEGLITCH_ADDR 32
#define EXTRAC_THR_ADDR 34
#define EXTRAC_MAX_ADDR 36
#define PATCH_ADDR 38
#define OCTAVE_ADDR 40
#define CTOUCH_THR_ADDR 42
#define BREATHCURVE_ADDR 44
#define VEL_SMP_DL_ADDR 46
#define VEL_BIAS_ADDR 48
#define PINKY_KEY_ADDR 50
#define FP1_ADDR 52
#define FP2_ADDR 54
#define FP3_ADDR 56
#define FP4_ADDR 58
#define FP5_ADDR 60
#define FP6_ADDR 62
#define FP7_ADDR 64
#define DIPSW_BITS_ADDR 66
#define PARAL_ADDR 68
#define ROTN1_ADDR 70
#define ROTN2_ADDR 72
#define ROTN3_ADDR 74
#define ROTN4_ADDR 76
#define PRIO_ADDR 78
#define VIB_SENS_ADDR 80
#define VIB_RETN_ADDR 82
#define VIB_SQUELCH_ADDR 84
#define VIB_DIRECTION_ADDR 86
#define BREATH_CC2_ADDR 88
#define BREATH_CC2_RISE_ADDR 90
#define VIB_SENS_BITE_ADDR 92
#define VIB_SQUELCH_BITE_ADDR 94
#define VIB_CONTROL_ADDR 96
#define TRILL3_INTERVAL_ADDR 98
#define DAC_MODE_ADDR 100
#define EXTRA2_ADDR 102
#define LEVEL_CC_ADDR 104
#define LEVEL_VAL_ADDR 106
#define FINGER_ADDR 108
#define LPINKY3_ADDR 110
#define BATTYPE_ADDR 112
#define HARMSET_ADDR 114
#define HARMSEL_ADDR 116
#define PARAB_ADDR 118
#define ROTB1_ADDR 120
#define ROTB2_ADDR 122
#define ROTB3_ADDR 124
#define ROTB4_ADDR 126
#define PARAC_ADDR 128
#define ROTC1_ADDR 130
#define ROTC2_ADDR 132
#define ROTC3_ADDR 134
#define ROTC4_ADDR 136
#define POLYSEL_ADDR 138
#define FWCTYPE_ADDR 140
#define HMZKEY_ADDR 150
#define FWCLCH_ADDR 152
#define FWCDP2_ADDR 154
#define HMZLIMIT_ADDR 156
#define BRINTERV_ADDR 158
#define OTFKEY_ADDR 160
#define PORTLIMIT_ADDR 162
#define LEVER_THR_ADDR 164
#define LEVER_MAX_ADDR 166
#define BRHARMSET_ADDR 168
#define BRHARMSEL_ADDR 170
#define BITECTL_ADDR 172
#define BITECC_ADDR 174
#define LEVERCTL_ADDR 176
#define LEVERCC_ADDR 178
#define CVTUNE_ADDR 180
#define CVSCALE_ADDR 182
#define CVRATE_ADDR 184
#define ROLLER_ADDR 186

#define EEPROM_SIZE 188 //Last address +2


//DAC output modes
#define DAC_MODE_BREATH 0
#define DAC_MODE_PITCH 1

#define DIPSW_FASTBOOT    0
#define DIPSW_LEGACY      1
#define DIPSW_LEGACYBRACT 2
#define DIPSW_WIDION      3
#define DIPSW_GATEOPEN    4
#define DIPSW_SPKEYENABLE 5
#define DIPSW_BCASMODE    6


//"factory" values for settings
#define EEPROM_VERSION 45

#define BREATH_THR_FACTORY 1400
#define BREATH_MAX_FACTORY 4000
#define PORTAM_THR_FACTORY 2600
#define PORTAM_MAX_FACTORY 3300
#define PORTPR_THR_FACTORY 1200
#define PORTPR_MAX_FACTORY 2000
#define PITCHB_THR_FACTORY 2000
#define PITCHB_MAX_FACTORY 3000
#define EXTRAC_THR_FACTORY 2800
#define EXTRAC_MAX_FACTORY 3500
#define LEVER_THR_FACTORY 1700
#define LEVER_MAX_FACTORY 1800
#define TRANSP_FACTORY 12   // 12 is 0 transpose
#define MIDI_FACTORY 1      // 1-16
#define BREATH_CC_FACTORY 2 //thats CC#2, see ccList
#define BREATH_AT_FACTORY 0 //aftertouch default off
#define VELOCITY_FACTORY 0  // 0 is dynamic/breath controlled velocity
#define PORTAM_FACTORY 2    // 0 - OFF, 1 - ON, 2 - SW
#define PB_FACTORY 1        // 0 - OFF, 1 - 12
#define EXTRA_FACTORY 1     // 0 - OFF, 1 - Modulation wheel, 2 - Foot pedal, 3 - Filter Cutoff, 4 - Sustain pedal
#define VIBRATO_FACTORY 4   // 0 - OFF, 1 - 9 depth
#define DEGLITCH_FACTORY 20 // 0 - OFF, 5 to 70 ms in steps of 5
#define PATCH_FACTORY 1     // MIDI program change 1-128
#define OCTAVE_FACTORY 3    // 3 is 0 octave change
#define CTOUCH_THR_FACTORY 125  // MPR121 touch threshold
#define BREATHCURVE_FACTORY 4 // 0 to 12 (-4 to +4, S1 to S4)
#define VEL_SMP_DL_FACTORY 20 // 0 to 30
#define VEL_BIAS_FACTORY 0  // 0 to 9
#define PINKY_KEY_FACTORY 12 // 0 - 11 (QuickTranspose -12 to -1), 12 (pb/2), 13 - 22 (QuickTranspose +1 to +12)
#define DIPSW_BITS_FACTORY 0 // virtual dip switch settings for special modes (work in progress)
#define PARAL_FACTORY 31 // 7 (+ 24) Rotator parallel
#define ROTN1_FACTORY 19 // -5 (+24) Rotation 1
#define ROTN2_FACTORY 14 // -10 (+24) Rotation 2
#define ROTN3_FACTORY 17 // -7 (+24) Rotation 3
#define ROTN4_FACTORY 10 // -14 (+24) Rotation 4
#define PRIO_FACTORY 0 // Mono priority 0 - BAS(e note), 1 - ROT(ating note)
#define VIB_SENS_FACTORY 6 // 1 least sensitive, higher more sensitive
#define VIB_RETN_FACTORY 2 // 0, no return, 1 slow return, higher faster return
#define VIB_SQUELCH_FACTORY 12 // 0 to 30, vib signal squelch
#define VIB_DIRECTION_FACTORY 0
#define BREATH_CC2_FACTORY 0  //OFF,1-127
#define BREATH_CC2_RISE_FACTORY 1
#define VIB_SENS_BITE_FACTORY 8
#define VIB_SQUELCH_BITE_FACTORY 15
#define VIB_CONTROL_FACTORY 0
#define TRILL3_INTERVAL_FACTORY 4
#define DAC_MODE_FACTORY DAC_MODE_PITCH
#define EXTRA2_FACTORY 0
#define LEVEL_CC_FACTORY 11
#define LEVEL_VAL_FACTORY 127
#define FINGER_FACTORY 0
#define LPINKY3_FACTORY 0
#define BATTYPE_FACTORY 0
#define HARMSET_FACTORY 0
#define HARMSEL_FACTORY 0
#define PARAB_FACTORY 31 // 7 (+ 24) Rotator parallel
#define ROTB1_FACTORY 19 // -5 (+24) Rotation 1
#define ROTB2_FACTORY 14 // -10 (+24) Rotation 2
#define ROTB3_FACTORY 17 // -7 (+24) Rotation 3
#define ROTB4_FACTORY 10 // -14 (+24) Rotation 4
#define PARAC_FACTORY 31 // 7 (+ 24) Rotator parallel
#define ROTC1_FACTORY 19 // -5 (+24) Rotation 1
#define ROTC2_FACTORY 14 // -10 (+24) Rotation 2
#define ROTC3_FACTORY 17 // -7 (+24) Rotation 3
#define ROTC4_FACTORY 10 // -14 (+24) Rotation 4
#define POLYSEL_FACTORY 0
#define FWCTYPE_FACTORY 0
#define HMZKEY_FACTORY 0
#define FWCLCH_FACTORY 0
#define FWCDP2_FACTORY 0
#define HMZLIMIT_FACTORY 5
#define BRINTERV_FACTORY 6
#define OTFKEY_FACTORY 0
#define PORTLIMIT_FACTORY 127
#define BRHARMSET_FACTORY 0
#define BRHARMSEL_FACTORY 0
#define BITECTL_FACTORY 2 // GLD
#define LEVERCTL_FACTORY 1 // VIB
#define BITECC_FACTORY 1 //Mod Wheel
#define LEVERCC_FACTORY 11 //Expression
#define CVTUNE_FACTORY 100 // 100 is zero tuning
#define CVSCALE_FACTORY 100 // 100 is zero scaling
#define CVRATE_FACTORY 3 // 3 is 5.5Hz
#define ROLLER_FACTORY 1

#define NO_CHECKSUM 0x7F007F00

void readEEPROM(const bool factoryReset);
void setBit(uint16_t &bitfield, const uint8_t pos, const uint16_t value);
uint16_t readSetting(const uint16_t address);
void writeSetting(const uint16_t address, const uint16_t value);
uint16_t readSettingBounded(const uint16_t address, const uint16_t min, const uint16_t max, const uint16_t defaultValue);

//Functions for config management mode
void sendSysexSettings();
void sendSysexMessage(const char* messageCode);
void sendSysexVersion();

void handleSysex(uint8_t *data, unsigned int length);
void handleSysexChunk(const uint8_t *data, uint16_t length, bool last);

uint32_t crc32(const uint8_t *message, const size_t length);

void configInitScreen();
void configShowMessage(const char* message);

void configModeSetup();
void configModeLoop();

#endif
