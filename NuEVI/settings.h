
#ifndef __SETTINGS_H
#define __SETTINGS_H



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

//"factory" values for settings
#define VERSION 32
#define BREATH_THR_FACTORY 1400
#define BREATH_MAX_FACTORY 4000
#define PORTAM_THR_FACTORY 2600
#define PORTAM_MAX_FACTORY 3300
#define PORTPR_THR_FACTORY 1200
#define PORTPR_MAX_FACTORY 2000
#define PITCHB_THR_FACTORY 1400
#define PITCHB_MAX_FACTORY 2300
#define EXTRAC_THR_FACTORY 1200
#define EXTRAC_MAX_FACTORY 2400
#define TRANSP_FACTORY 12   // 12 is 0 transpose
#define MIDI_FACTORY 1      // 1-16
#define BREATH_CC_FACTORY 2 //thats CC#2, see ccList
#define BREATH_AT_FACTORY 0 //aftertouch default off
#define VELOCITY_FACTORY 0  // 0 is dynamic/breath controlled velocity
#define PORTAM_FACTORY 2    // 0 - OFF, 1 - ON, 2 - SW
#define PB_FACTORY 1        // 0 - OFF, 1 - 12
#define EXTRA_FACTORY 2     // 0 - OFF, 1 - Modulation wheel, 2 - Foot pedal, 3 - Filter Cutoff, 4 - Sustain pedal
#define VIBRATO_FACTORY 4   // 0 - OFF, 1 - 9 depth
#define DEGLITCH_FACTORY 20 // 0 - OFF, 5 to 70 ms in steps of 5
#define PATCH_FACTORY 1     // MIDI program change 1-128
#define OCTAVE_FACTORY 3    // 3 is 0 octave change
#define CTOUCH_THR_FACTORY 125  // MPR121 touch threshold
#define BREATHCURVE_FACTORY 4 // 0 to 12 (-4 to +4, S1 to S4)
#define VEL_SMP_DL_FACTORY 15 // 0 to 30
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
#define VIB_SENS_BITE_FACTORY 6
#define VIB_SQUELCH_BITE_FACTORY 12
#define VIB_CONTROL_FACTORY 0

#endif
