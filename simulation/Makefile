CC=clang
CXX=clang++

CFLAGS= -O0 -g -Wall -Wextra -Wpedantic -Wno-gnu -mmacosx-version-min=10.9 -F/Library/Frameworks
CFLAGS += -DARDUINO=10808 -D__MK20DX256__

CXXFLAGS= $(CFLAGS) -std=c++14

LIBS=-framework SDL2 -lc++ -lc -framework OpenGL
LDFLAGS=-macosx_version_min 10.9

SYSINC = ~/Documents/Arduino/libraries/Filters ./include
INCS = ../NuEVI ./include

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
				src/Adafruit_MPR121_sim.cpp

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
