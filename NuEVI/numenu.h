#ifndef __NUMENU_H
#define __NUMENU_H

#include <cstdint>
#include <functional>

#define MAX_DEPTH 16

class Adafruit_SSD1306;

class NuMenu
{
public:
    NuMenu(Adafruit_SSD1306 &gfx);

    bool init();
    void update(uint16_t buttonState);

    void setEnabled(bool state) { _enabled = state; }

    static void drawMenuItems(const char* title, const char* entries[], int count, int selection, int offset = 0);

private:
    bool _enabled;
    // MenuPageState _rootMenu;
    // MenuPageState _pageStack[MAX_DEPTH];

    Adafruit_SSD1306 &_display;
};

#endif
