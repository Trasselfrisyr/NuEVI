#ifndef __NUMENU_H
#define __NUMENU_H

//***********************************************************

enum MenuType {
  ESub,
  ESubNew,
  ESubRotator,
  EStateChange,
};

struct MenuEntry {
  enum MenuType type;
  const char* title;
};

struct MenuEntrySub {
  enum MenuType type;
  const char* title;
  const char* subTitle;
  byte* flag;
  void (*subMenuFunc)(int color);
};

enum MenuEntryFlags {
  ENone = 0,
  EWrap = (1<<0),
  ECustom = (1<<1),
  EEnterHandler = (1<<2),
};

struct MenuEntrySubNew {
  enum MenuType type;
  const char* title;
  const char* subTitle;
  uint16_t* valuePtr;
  uint16_t min;
  uint16_t max;  
  uint16_t flags;
  void (*getSubTextFunc)(char*textBuffer, const char**label);
  void (*applyFunc)(void);
  bool (*onEnterFunc)(void);
};


struct MenuEntrySubRotator {
  enum MenuType type;
  const char* title;
  const char* subTitle;
  byte flagValue;
  byte* flag;
  void (*subMenuFunc)(int color);
};

struct MenuEntryStateCh {
  enum MenuType type;
  const char* title;
  byte state;
};

struct MenuPage {
  const char* title;
  byte cursor;
  byte parentPage;
  byte numEntries;
  const MenuEntry** entries;
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
