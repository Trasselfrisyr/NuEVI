CC=clang
CXX=clang++

CFLAGS=-Wall -Wextra -Wpedantic -Wno-gnu
CFLAGS += -DARDUINO=10808 -DTEENSYDUINO=146 -D__MK20DX256__ -DUSB_MIDI

RELEASE ?= 0
ifeq ($(RELEASE), 1)
   CFLAGS+=-O2
else
   CFLAGS+=-O0 -g
endif


CXXFLAGS= $(CFLAGS) -std=c++14

LIBS=
LDFLAGS=

SYSINC = ./include
INCS = ../NuEVI ./include ./imgui ./gl3w

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
  SYSINC+=./platform/macos
  CFLAGS+=-mmacos-version-min=10.9 -F/Library/Frameworks
  LIBS+=-framework SDL2 -framework OpenGL -F/Library/Frameworks
  LDFLAGS+=-Wl,-rpath,@executable_path/../Frameworks
  LDFLAGS+=-Wl,-rpath,/Library/Frameworks
else ifeq ($(UNAME_S), Linux)
  CFLAGS+=-D__LINUX__ $(shell pkg-config --cflags sdl2)
  LIBS+=$(shell pkg-config --libs sdl2 opengl)
endif

INCDIRS = $(addprefix -isystem ,$(SYSINC))
INCDIRS += $(addprefix -I,$(INCS))


TARGET=nuevisim

CXXFILES=		../NuEVI/menu.cpp \
				../NuEVI/adjustmenu.cpp \
				../NuEVI/midi.cpp \
				../NuEVI/settings.cpp \
				../NuEVI/led.cpp \
				src/nuevisim.cpp \
				src/simeeprom.cpp \
				src/Print.cpp \
				src/simserial.cpp \
				src/simwire.cpp \
				src/simusbmidi.cpp \
				src/filters.cpp \
				src/Adafruit_GFX_sim.cpp \
				src/Adafruit_SSD1306_sim.cpp \
				src/Adafruit_MPR121_sim.cpp \
				imgui/imgui.cpp \
				imgui/imgui_draw.cpp \
				imgui/imgui_widgets.cpp \
				imgui/examples/imgui_impl_sdl.cpp \
				imgui/examples/imgui_impl_opengl3.cpp


CFILES= 		gl3w/gl3w.c	\
				../NuEVI/name.c


OBJS=$(CXXFILES:.cpp=.o) $(CFILES:.c=.o)

all: $(TARGET)

nuevisim: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -o $@  $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c -o $@  $<

clean:
	rm -f $(TARGET) $(OBJS) *.o

.PHONY: all


# Dependecies
DEPS=make.deps

.PHONY: deps mrproper
mrproper: clean
	rm $(DEPS)
deps: $(DEPS)

H_DEPS=$(wildcard src/*.h) $(wildcard ../NuEVI/*.h)

make.deps: $(CXXFILES) $(H_DEPS)
	$(CXX) $(CXXFLAGS) -Wno-deprecated $(INCDIRS) -MM $(DEPS_HS) $^ > $@

-include .deps/*
