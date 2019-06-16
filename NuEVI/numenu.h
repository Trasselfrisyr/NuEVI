#ifndef __NUMENU_H
#define __NUMENU_H

//***********************************************************

enum MenuType {
  ESub,
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
