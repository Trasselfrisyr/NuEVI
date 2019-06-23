CC=clang
CXX=clang++

CFLAGS=-Wall -Wextra -Wpedantic -Wno-gnu -mmacosx-version-min=10.9 -F/Library/Frameworks
CFLAGS += -DARDUINO=10808 -D__MK20DX256__

RELEASE ?= 0
ifeq ($(RELEASE), 1)
   CFLAGS +=-O3
else
   CFLAGS += -O0 -g 
endif


CXXFLAGS= $(CFLAGS) -std=c++14

LIBS=-framework SDL2 -lc++ -lc -framework OpenGL
LDFLAGS=-macosx_version_min 10.9 -rpath @executable_path/../Frameworks

SYSINC = ~/Documents/Arduino/libraries/Filters ./include
INCS = ../NuEVI ./include ./imgui ./gl3w

INCDIRS = $(addprefix -isystem ,$(SYSINC))
INCDIRS += $(addprefix -I,$(INCS))


TARGET=nuevisim

CXXFILES=		../NuEVI/menu.cpp \
				src/nuevisim.cpp \
				src/simeeprom.cpp \
				src/Print.cpp \
				src/simserial.cpp \
				src/simwire.cpp \
				src/simmidi.cpp \
				src/filters.cpp \
				src/Adafruit_GFX_sim.cpp \
				src/Adafruit_SSD1306_sim.cpp \
				src/Adafruit_MPR121_sim.cpp \
				imgui/imgui.cpp \
				imgui/imgui_draw.cpp \
				imgui/imgui_widgets.cpp \
				imgui/examples/imgui_impl_sdl.cpp \
				imgui/examples/imgui_impl_opengl3.cpp

CFILES= 		gl3w/gl3w.c

OBJS=$(CXXFILES:.cpp=.o) $(CFILES:.c=.o)

all: $(TARGET)

nuevisim: $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(LIBS) $^

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
