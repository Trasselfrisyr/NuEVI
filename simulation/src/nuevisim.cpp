#include <functional>

#include <SDL2/SDL.h>

#include <Adafruit_MPR121.h>
#include <Adafruit_SSD1306.h>
#include "globals.h"
#include "hardware.h"

#include <Arduino.h>

// Forward declarations
static void SimQuit(void);
static int SimInit(void);
static int SimRun(void);
static void SimLoop(std::function<bool()>, std::function<void()>);



extern Adafruit_SSD1306 display;
extern Adafruit_MPR121 touchSensor;
SimWire Wire;
SimSerial Serial;


static const int scale = 2;

static SDL_Window *window;
static SDL_Surface *surface;

void _reboot_Teensyduino_()
{
    // TODO: reboot
    // Ignore that this is marked as noreturn
    printf("Some kind of panic, rebooting teensy...\n");
    setup();
}

extern void menu(void);
extern void initDisplay(void);
extern void breath();
extern int noteValueCheck(int);
extern unsigned int breathCurve(unsigned int);
extern void pitch_bend(void);
extern void portamento_(void);
extern void extraController(void);
extern void statusLEDs(void);
extern void doorKnobCheck(void);
extern void portOn(void);
extern void portOff(void);
extern void port(void);
extern void readSwitches(void);
extern int patchLimit(int value);

static uint8_t digitalInputs[256]; // random number of inputs..
static uint8_t digitalOutputs[256]; // random number of inputs..
static uint16_t analogInputs[256]; // random number of inputs..
static uint16_t analogOutputs[256]; // random number of inputs..
static int _analogRes = 12;

void digitalWrite(uint8_t pin, uint8_t val)
{
    printf("digital write %d = %d\n", pin, val);
    digitalOutputs[pin] = val;
}

uint8_t digitalRead(uint8_t pin) {
    return digitalInputs[pin];
}

void delay(unsigned int ms)
{
    uint32_t endTick = SDL_GetTicks() + ms;
    auto checktime = [endTick]() -> bool { return endTick > SDL_GetTicks(); };
    SimLoop(checktime,NULL);
}

void pinMode(uint8_t __attribute((unused)) pin, uint8_t __attribute((unused)) mode)
{

}

int analogRead(uint8_t pin)
{
    return analogInputs[pin];
}


void analogReadRes(unsigned int __attribute__((unused)) bits)
{
    // ??
}

uint32_t analogWriteRes(uint32_t res)
{
    _analogRes = res; // ??
    return _analogRes; // ??
}


void analogWriteFrequency(uint8_t __unused pin, float __unused frequency)
{
}

void analogWrite(uint8_t pin, int value)
{
    analogOutputs[pin] = value;
}


uint16_t micros()
{
    return SDL_GetTicks()*1000;
}

uint16_t millis()
{
    return SDL_GetTicks();
}

int touchRead(uint8_t reg){ return touchSensor.readRegister16(reg); }


int main()
{
    return SimRun();
}

//***********************************************************

static void GetDisplay(SDL_Surface* dest)
{
    int w = display.width();
    int h = display.height();

    SDL_LockSurface(dest);
    uint8_t* buffer = (uint8_t*)surface->pixels;
    int pitch = surface->pitch;

    if(!display.enabled_) {
        SDL_memset( buffer, 0, (w*h)*3);
    } else {
        int fg = 255;
        int bg = 0;

        if( display.dimmed_) fg = 127;
        if( display.inverted_ ) { int tmp = fg; fg = bg; bg = tmp; }

        for(int y = 0 ; y < h; ++y) {
            for(int x = 0 ; x < w; ++x) {
                int color = display.getPixel(x,y) ? fg : bg;
                SDL_memset( buffer + pitch*y + x*3, color, 3);
            }
        }
    }
    SDL_UnlockSurface(dest);
}


static int doQuit = 0;

static void SimLoop(std::function<bool()> continue_predicate, std::function<void()> loopFunc)
{
    uint32_t time;
    while( continue_predicate() ) {
        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            if( event.type == SDL_QUIT ) {
                doQuit = 1;
                break;
            }
            if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_LEFT:     digitalInputs[mPin] = 0; break;
                    case SDLK_RIGHT:    digitalInputs[ePin] = 0; break;
                    case SDLK_UP:       digitalInputs[uPin] = 0; break;
                    case SDLK_DOWN:     digitalInputs[dPin] = 0; break;
                }
            }
            else if(event.type == SDL_KEYUP )
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_LEFT:     digitalInputs[mPin] = 1; break;
                    case SDLK_RIGHT:    digitalInputs[ePin] = 1; break;
                    case SDLK_UP:       digitalInputs[uPin] = 1; break;
                    case SDLK_DOWN:     digitalInputs[dPin] = 1; break;
                }
                fflush(stdout);
            }
        }

        if(doQuit)
            break;

        if(loopFunc) loopFunc();

        time = SDL_GetTicks();

        // TODO: Get buffer from SSD1306 and copy to surface...

        GetDisplay(surface);

        SDL_Surface *dstSurface = SDL_GetWindowSurface(window);
        SDL_BlitScaled( surface, NULL, dstSurface, NULL );
        SDL_UpdateWindowSurface(window);

        uint32_t timePassed = SDL_GetTicks() - time;
        if( timePassed < 16 ) {
            SDL_Delay( 16-timePassed );
        }
    }
}

static int SimRun( )
{
	if( 0 != SimInit() ) { return 1; }
    setup();
    SimLoop( []() -> bool { return true; }, loop );
    SimQuit();
	return 0;
}


static int SimInit()
{
	int result = result = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );
	if( 0 != result ) {
		fprintf(stderr, "Could not initialize SDL");
		return 1;
	}

	window = SDL_CreateWindow( "TinySim"
		, SDL_WINDOWPOS_UNDEFINED
		, SDL_WINDOWPOS_UNDEFINED
		, 128*scale
		, 64*scale
		, SDL_WINDOW_SHOWN );

	if( window == NULL ) {
		fprintf(stderr, "Could not create SDL window");
		SimQuit();
		return 2;
	}

	SDL_SetWindowTitle( window, "Tiny Sim" );

    memset(digitalInputs, 1, sizeof(digitalInputs));

    int16_t w = display.width();
    int16_t h = display.height();

    surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, SDL_PIXELFORMAT_RGB24);
    if(!surface)
    {
        printf("Could not create surface with size %d %d\n", w,h);
        SimQuit();
    }

    printf("create surface with size %d %d\n", w,h);

    return result;
}

static void SimQuit()
{
    printf("Leaving Sim, see you later!\n");

    if( window != NULL ) {
        SDL_DestroyWindow( window );
        SDL_FreeSurface( surface );
    }
    SDL_Quit();
}

#include "NuEVI.ino"
