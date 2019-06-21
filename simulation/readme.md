## Simulator for NuEVI

This is a simple SDL2 based simulator that runs the NuEVI firmware compiled for MacOS. This is for testing the menu and is not supposed to produce any midi events.


### Requirements

 * The Filters library must be installed in ~/Documents/Arduino/libraries/Filters.
 * SDL2 must be installed on your machine.
 * You probably need to have XCode and XCodes command line tools installed as well.

[Dear Imgui](https://github.com/ocornut/imgui) is pulled in as an git submodule. Run `git submodule init` and `git submodule update` to get the code. The code is tested with tag v1.70 of ImGui, so if you run into problems make sure that is the checked out version.


### Know limitations

Currently the only input simulated are the menu buttons (use the arrow keys). This means that all menu functions cannot be tested, since the rotator menu cannot be opened. There is also a limitation on some keyboard on how many buttons can be pressed at the same time.


### Future plans

 * Add a Dear ImGUI based UI for simulating all other inputs.
