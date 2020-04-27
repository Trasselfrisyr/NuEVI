#ifndef __NUMENU_H
#define __NUMENU_H

#include <stdint.h>

//***********************************************************

struct KeyState {
  uint8_t current;
  uint8_t changed;
};

//***********************************************************

enum MenuType {
  ESub,
  EStateChange,
};

enum MenuEntryFlags {
  ENone = 0,
  EMenuEntryWrap = (1u<<0),
  EMenuEntryCustom = (1u<<1),
  EMenuEntryEnterHandler = (1u<<2),
};

enum MenuPageFlags {
  EMenuPageCustom = (1u<<0),
  EMenuPageRoot = (1u<<1),
  EMenuCustomTitle = (1u << 2),
};


struct MenuEntry {
  enum MenuType type;
  const char* title;
};

struct MenuEntrySub;
typedef const MenuEntrySub& SubMenuRef;

struct MenuEntrySub {
  enum MenuType type;
  const char* title;
  const char* subTitle;
  uint16_t* valuePtr;
  uint16_t min;
  uint16_t max;  
  uint16_t flags;
  void (*getSubTextFunc)(SubMenuRef, char*textBuffer, const char**label);
  void (*applyFunc)(SubMenuRef);
  bool (*onEnterFunc)(void);
};

struct MenuEntryStateCh {
  enum MenuType type;
  const char* title;
  uint8_t state;
};

struct MenuPage {
  const char* title;
  uint16_t flags;
  uint8_t cursor;
  uint8_t parentPage;
  uint8_t numEntries;
  const MenuEntry** entries;
};

struct MenuPageCustom {
  const char* title;
  uint16_t flags;
  bool (*menuUpdateFunc)(KeyState &input, uint32_t timeNow);
};

//***********************************************************

struct AdjustValue {
  uint16_t *value;
  uint16_t limitLow;
  uint16_t limitHigh;
};

struct AdjustMenuEntry {
  const char* title;
  AdjustValue entries[2];
  void (*saveFunc)(const AdjustMenuEntry&);
};

#endif
