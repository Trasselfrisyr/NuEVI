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
* Adafruit SSD1306

You also need to install [Edgar Bonet's Filters library](https://github.com/edgar-bonet/Filters),
specifically the `fix-integer-overflow` branch. One of the easiest way to do that is to download the
git repo [as a zip file](https://github.com/edgar-bonet/Filters/archive/fix-integer-overflow.zip),
and then add that in the Arduino IDE (under Sketch -> Include Library -> Add .ZIP library)

The SSD1306 display driver library then needs to be patched to support the right kind of display.
To do this, you need to find the Adafruit_SSD1306.h file used by the Arduino IDE. Exactly where it
is depends on your OS and how you installed the library, but the default location when installed
via library manager should be something close to either of these.

* MacOS: `/Applications/Arduino.app/Contents/Java/hardware/teensy/avr/libraries/Adafruit_SSD1306/Adafruit_SSD1306.h`
* Windows: `My Documents\Arduino\libraries\Adafruit_SSD1306/Adafruit_SSD1306.h`
* Linux (at least some distributions): `/usr/share/arduino/libraries/usr/share/arduino/libraries/Adafruit_SSD1306/Adafruit_SSD1306.h`

In that file, there is a section around line 69 (at the time of writing) that determines the type
of display. There, make sure the `#define SSD1306_128_64` is not commented out, but that the others
next to it are.

### Compile options

Open NuEVI.ino in the Arduino IDE. Under "Tools -> Board", select "Teensy 3.2 / 3.1". Then set
"Tools -> USB Type" to "MIDI".

If you have Teensyduino 1.4.1 or later, you also need to change an option in the code. In the
Arduino Editor (where you have NuEVI.ino open), uncomment the line with `#define NEWTEENSYDUINO`. If
this does not match the Teensyduino version, pitch bend over USB-MIDI will not work properly.

### Building and uploading

Connect the NuEVI via USB to your computer, open the Teensy application and make sure the "Auto"
option is selected (the green round icon). In Arduino IDE, select "Sketch -> Verify/Compile" and
once that is complete press the reset button on the Teensy chip (you have to remove the top cover
on the NuEVI to access this). Upon resetting, it should upload the new firmware onto the NuEVI.

After uploading new firmware, you may need to reset the config memory of the NuEVI. It's a good idea
to do between version upgrades, since if the config parameter format often changes. To do this, press
and hold the MENU and ENTER buttons while turning on the NuEVI.
