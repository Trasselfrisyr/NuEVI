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
* NuEVI also includes on the [Filters](https://github.com/JonHub/Filters) library by Jonathan Driscoll, but that is no longer an external dependency.


### Compile options

Open NuEVI.ino in the Arduino IDE. Under "Tools -> Board", select "Teensy 3.2 / 3.1". Then set
"Tools -> USB Type" to "MIDI".

### Building and uploading

Connect the NuEVI via USB to your computer, open the Teensy application and make sure the "Auto"
option is selected (the green round icon). In Arduino IDE, select "Sketch -> Verify/Compile" and
once that is complete press the reset button on the Teensy chip (you have to remove the top cover
on the NuEVI to access this). Upon resetting, it should upload the new firmware onto the NuEVI.

After uploading new firmware, you may need to reset the config memory of the NuEVI. It's a good idea
to do between version upgrades, since the config parameter format often changes. To do this, press
and hold the MENU and ENTER buttons while turning on the NuEVI.
