# NuEVI
A project by wind controller enthusiasts wanting to save the endangered Electronic Valve Instrument.

Follow the project at https://hackaday.io/project/25756-diy-evi-style-windcontroller


## Building NuEVI

NuEVI is easiest to build using the Arduino IDE. You will also need to download and install
[Teensyduino](https://www.pjrc.com/teensy/td_download.html) to build for and upload to the Teensy.

### Libraries

A few libraries need to be added that are not part of the default Arduino install. These can be
added directly via the Library Manager in the Arduino IDE:
* Adafruit MPR121
* Adafruit GFX
* Adafruit SSD1306 (version 1.2.9 or above)

You also need to install [Edgar Bonet's Filters library](https://github.com/edgar-bonet/Filters),
specifically the `fix-integer-overflow` branch. One of the easiest way to do that is to download the
git repo [as a zip file](https://github.com/edgar-bonet/Filters/archive/fix-integer-overflow.zip),
and then add that in the Arduino IDE (under Sketch -> Include Library -> Add .ZIP library)


### Compile options

Open NuEVI.ino in the Arduino IDE. Under "Tools -> Board", select "Teensy 3.2 / 3.1". Then set
"Tools -> USB Type" to "MIDI".

If you have Teensyduino 1.4.0 or earlier, you also need to change an option in the code. In the
Arduino Editor (where you have NuEVI.ino open), comment out the line with `#define NEWTEENSYDUINO`. If
this does not match the Teensyduino version, pitch bend over USB-MIDI will not work properly.

### Building and uploading

Connect the NuEVI via USB to your computer, open the Teensy application and make sure the "Auto"
option is selected (the green round icon). In Arduino IDE, select "Sketch -> Verify/Compile" and
once that is complete press the reset button on the Teensy chip (you have to remove the top cover
on the NuEVI to access this). Upon resetting, it should upload the new firmware onto the NuEVI.

After uploading new firmware, you may need to reset the config memory of the NuEVI. It's a good idea
to do between version upgrades, since the config parameter format often changes. To do this, press
and hold the MENU and ENTER buttons while turning on the NuEVI.
