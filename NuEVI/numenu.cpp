/*
Notes on the original menu implementation

# Menus

## Main Menu

### Transpose
Sub menu with values -12 to 12.

### Octave
Sub menu with values -3 to +3

### MIDI CH

Sub menu with values 0 to 16. Should be 1 to 16, but there might be a bug
either in my simulation code, my changes to the menu or a bug in the original
menu.

### Adjust

This is a special option where the Adjust menu mode is entered. It take over
the display and draw horizontal indicators for threshold and such. More on
this in a later section.

### SETUP BR

Breath setup. Opens a new menu with breath specific stuff.


### SETUP CTL

Controls setup. Opens a new menu.



 */

#include <cstring>
#include <Adafruit_SSD1306.h>
#include "numenu.h"
#include "menu.h"

NuMenu::NuMenu(Adafruit_SSD1306& display)
 : _display(display)
{
}

bool NuMenu::init()
{
    // memset(_pageStack, 0, sizeof(_pageStack));
    // _rootMenu = MenuPageState(root, 0, 0);
    _enabled = false;
    return true;
}

void NuMenu::update(uint16_t buttonState)
{
    if(_enabled)
    {
        // int 
    }
}
extern Adafruit_SSD1306 display;

void NuMenu::drawMenuItems(const char* title, const char* entries[], int count, int selection, int offset)
{
    //Initialize display and draw menu header + line
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(title);
    display.drawLine(0,MENU_ROW_HEIGHT,127,MENU_ROW_HEIGHT, WHITE);

    int rowPixel = MENU_HEADER_OFFSET + MENU_ROW_HEIGHT;

    for(int index = offset, count = 0; ((index-offset) < MENU_NUM_ROWS) && index < count; index++ )
    {
        // int rowPixel = (row+1)*MENU_ROW_HEIGHT + MENU_HEADER_OFFSET;
        const char* lineText = entries[index];
        display.setCursor(0,rowPixel);
        rowPixel += (MENU_ROW_HEIGHT+1);
        display.println(lineText);
    }

    // TODO: Fix cursor
    // if(selection>=0)
    //     drawMenuCursor(selection, WHITE);

}



// This is for the SUB MENU
// void NuMenu::drawSelection(const char* title, const char* entries[], int count, int* selection)
// {
//     _display.fillRect(63,11,64,52,BLACK);
//     _display.drawRect(63,11,64,52,WHITE);
//     _display.setTextColor(WHITE);
//     _display.setTextSize(1);

//     _display.setCursor(68,15);
//     _display.println(title);

//     const char* entryTxt = entries[*selection];
//     int len = strlen(entryTxt);

//     _display.setTextSize(2);

//     _display.setCursor(91 - 4*len,33);
//     _display.println(entryTxt);

//     _display.display();
// }
