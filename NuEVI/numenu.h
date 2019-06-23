#ifndef __NUMENU_H
#define __NUMENU_H

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
  byte state;
};

struct MenuPage {
  const char* title;
  uint16_t flags;
  byte cursor;
  byte parentPage;
  byte numEntries;
  const MenuEntry** entries;
};

struct MenuPageCustom {
  const char* title;
  uint16_t flags;
  bool (*menuUpdateFunc)(void);
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
