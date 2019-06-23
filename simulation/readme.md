# Simulator for NuEVI

This is a simple SDL2 based simulator that runs the NuEVI firmware compiled for MacOS. This is for testing the menu and is not supposed to produce any midi events.

## Requirements

* The Filters library must be installed in ~/Documents/Arduino/libraries/Filters.
* [SDL2.framework](https://www.libsdl.org/download-2.0.php) must be installed on your machine in /Library/Frameworks/ (or ~/Library/Frameworks/).
* You probably need to have XCode and XCodes command line tools installed as well, but using brew to install make and clang might be enough.

[Dear Imgui](https://github.com/ocornut/imgui) is pulled in as an git submodule. Run `git submodule init` and `git submodule update` to get the code. The code is tested with tag v1.70 of ImGui, so if you run into problems make sure that is the checked out version.

## Know limitations

Currently only some input is simulated, and the default values are not based on real hardware. There is also a limitation on some keyboards on how many buttons can be pressed at the same time. This means that all menu functions cannot be tested.

## Future plans

* Add simulation for all inputs
* Show MIDI status in UI
* Add in-app log window
* Fake real breath input by keypress
