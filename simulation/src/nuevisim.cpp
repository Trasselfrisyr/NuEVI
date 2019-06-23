#include <functional>

#include <SDL2/SDL.h>

#include <Adafruit_MPR121.h>
#include <Adafruit_SSD1306.h>
#include <cmath>
#include "globals.h"
#include "hardware.h"
#include "imgui.h"
#include "GL/gl3w.h"
#include "examples/imgui_impl_sdl.h"
#include "examples/imgui_impl_opengl3.h"

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


static const int scale = 3;

static SDL_Window *window;

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

static GLuint displayTexture;

void digitalWrite(uint8_t pin, uint8_t val)
{
    // printf("digital write %d = %d\n", pin, val);
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
    _analogRes = bits;
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


bool animateAnalogs = false;
void analogUpdate(uint32_t time) {

    const uint16_t touchMaxValue = 1023;
    const uint16_t analogMax = (1<<_analogRes)-1;

    if(animateAnalogs) {
        for( int i = 0 ; i < 32; ++i) {
            analogInputs[i] = (sin(time*0.001f + i)*0.5f + 0.5f) * analogMax;
        }

        uint8_t *regs = touchSensor._registers;

        for(int r = 4; r < (4+24); r += 2) {
            uint16_t value = (sin(time * 0.005f + r)*0.5f + 0.5f) * touchMaxValue;
            regs[r] = value & 0xffu;
            regs[r+1] = (value >> 8) & 0x03u;
        }
    }
}


uint32_t micros()
{
    return SDL_GetTicks()*1000;
}

uint32_t millis()
{
    return SDL_GetTicks();
}


// There are 9 touch sensors pins on the teensy 3.2
int touchValues[12];

static int touchPinMapping[12] = {
    specialKeyPin, // 0       // SK or S2
    halfPitchBendKeyPin, // 1 // PD or S1
    vibratoPin, // 15
    extraPin, // 16
    bitePin, // 17
    pbDnPin, // 22
    pbUpPin, // 23
};


int touchRead(uint8_t pin)
{
    // find mapped sensors
    int i = 0;
    for(; i< 9 && touchPinMapping[i] != pin; ++i)
    if( i < 9)
        return touchValues[i];
    return 0;
}

static void touchWrite(uint8_t pin, uint16_t value)
{
    // find mapped sensors
    int i = 0;
    for(; (i < 9) && (touchPinMapping[i] != pin); ++i)
    if( i < 9) touchValues[i] = value;
}

static void doGlobalsWindow()
{
    if( ImGui::Begin("Globals" ) ) {


        if(ImGui::TreeNode("Sensor limits") )
        {
            ImGui::LabelText("Breath Thr", "%d", breathThrVal);
            ImGui::LabelText("Breath Max", "%d", breathMaxVal);
            ImGui::LabelText("Portam Thr", "%d", portamThrVal);
            ImGui::LabelText("Portam Max", "%d", portamMaxVal);
            ImGui::LabelText("Pitchb Thr", "%d", pitchbThrVal);
            ImGui::LabelText("Pitchb Max", "%d", pitchbMaxVal);
            ImGui::LabelText("Extrac Thr", "%d", extracThrVal);
            ImGui::LabelText("Extrac Max", "%d", extracMaxVal);
            ImGui::LabelText("Ctouch Thr", "%d", ctouchThrVal);
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Buttons") )
        {
            ImGui::LabelText("Valve 1", "%d", K1);
            ImGui::LabelText("Valve 2", "%d", K2);
            ImGui::LabelText("Valve 3", "%d", K3);
            ImGui::LabelText("Left index", "%d", K4);
            ImGui::LabelText("Trill 1", "%d", K5);
            ImGui::LabelText("Trill 2", "%d", K6);
            ImGui::LabelText("Trill 3", "%d", K7);
            ImGui::LabelText("half PB", "%d", halfPitchBendKey);
            ImGui::LabelText("Special", "%d", specialKey);
            ImGui::LabelText("Pinky", "%d", pinkyKey);
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Config") )
        {
            ImGui::LabelText("Transpose", "%d", transpose);
            ImGui::LabelText("MIDI channel", "%d", MIDIchannel);
            ImGui::LabelText("Breath curve", "%d", curve);
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Vibrato config") )
        {
            ImGui::LabelText("Sensitivity", "%d", vibSens);
            ImGui::LabelText("Return speed", "%d", vibRetn);
            ImGui::LabelText("signal squelch", "%d", vibSquelch);
            ImGui::LabelText("Direction", "%d", vibDirection);
            ImGui::TreePop();
        }


// unsigned short breathCC;  // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+:CF
// unsigned short breathAT;
// unsigned short velocity;
// unsigned short portamento;// switching on cc65? just cc5 enabled? SW:ON:OFF
// unsigned short PBdepth;   // OFF:1-12 divider
// unsigned short extraCT;   // OFF:MW:FP:CF:SP
// unsigned short vibrato;   // OFF:1-9
// unsigned short deglitch;  // 0-70 ms in steps of 5
// unsigned short patch;     // 1-128
// unsigned short octave;

// unsigned short velSmpDl;  // 0-30 ms
// unsigned short velBias;   // 0-9
// unsigned short pinkySetting; // 0 - 11 (QuickTranspose -12 to -1), 12 (pb/2), 13 - 24 (QuickTranspose +1 to +12)
// unsigned short dipSwBits; // virtual dip switch settings for special modes (work in progress)
// unsigned short priority; // mono priority for rotator chords


// unsigned short fastPatch[7];
// byte rotatorOn;
// byte currentRotation;
// int rotations[4];
// int parallel; // semitones

// int touch_Thr;

// unsigned long cursorBlinkTime;          // the last time the cursor was toggled

// byte activePatch;
// byte doPatchUpdate;

// byte legacy;
// byte legacyBrAct;

// byte slowMidi;

// int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
// int lastPressure;

// int biteSensor;    // capacitance data from bite sensor, for midi cc and threshold checks
// int lastBite;
// byte biteJumper;

// int exSensor;
// int lastEx;

// int pitchBend;

// int pbUp;
// int pbDn;

// byte vibLedOff;
// byte oldpkey;

// int vibThr;          // this gets auto calibrated in setup
// int vibThrLo;
// int vibZero;

    }

    ImGui::End();
}


//***********************************************************

static void doInputWindow()
{
    if( ImGui::Begin( "Inputs" ) ) {
        int val = analogInputs[breathSensorPin];
        if( ImGui::SliderInt("Breath input", &val, 0, 4095 ) && !animateAnalogs ) {
            analogInputs[breathSensorPin] = val;
        }

        val = analogInputs[vMeterPin];
        if( ImGui::SliderInt("Voltage", &val, 0, 4095 ) && !animateAnalogs ) {
            analogInputs[vMeterPin] = val;
        }

        // val = analogInputs[0];
        // if( ImGui::SliderInt("Unknown", &val, 0, 4095 ) && !animateAnalogs ) {
        //     analogInputs[0] = val;
        // }

        val = touchRead(halfPitchBendKeyPin);
        if( ImGui::SliderInt("Pinky key", &val, 0, 4095 ) && !animateAnalogs ) {
            touchWrite(halfPitchBendKeyPin, val);
        }

        val = analogInputs[A7];
        if( ImGui::SliderInt("Alt Bite", &val, 0, 4095 ) && !animateAnalogs ) {
            analogInputs[A7] = val;
        }

        ImGui::Separator();
        bool k1 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K1Pin*2) < ctouchThrVal;
        bool k2 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K2Pin*2) < ctouchThrVal;
        bool k3 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K3Pin*2) < ctouchThrVal;
        bool k4 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K4Pin*2) < ctouchThrVal;
        bool k5 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K5Pin*2) < ctouchThrVal;
        bool k6 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K6Pin*2) < ctouchThrVal;
        bool k7 = touchSensor.readRegister16(MPR121_FILTDATA_0L + K7Pin*2) < ctouchThrVal;

        if( ImGui::Checkbox("K1", &k1) )
            touchSensor.mockFilteredData(K1Pin, ctouchThrVal + (!k1 ? 100 : -100));

        ImGui::SameLine();
        if( ImGui::Checkbox("K2", &k2) )
            touchSensor.mockFilteredData(K2Pin, ctouchThrVal + (!k2 ? 100 : -100));

        ImGui::SameLine();
        if( ImGui::Checkbox("K3", &k3) )
            touchSensor.mockFilteredData(K3Pin, ctouchThrVal + (!k3 ? 100 : -100));

        ImGui::SameLine();
        if( ImGui::Checkbox("K4", &k4) )
            touchSensor.mockFilteredData(K4Pin, ctouchThrVal + (!k4 ? 100 : -100));

        ImGui::SameLine();
        if( ImGui::Checkbox("K5", &k5) )
            touchSensor.mockFilteredData(K5Pin, ctouchThrVal + (!k5 ? 100 : -100));

        ImGui::SameLine();
        if( ImGui::Checkbox("K6", &k6) )
            touchSensor.mockFilteredData(K6Pin, ctouchThrVal + (!k6 ? 100 : -100));

        ImGui::SameLine();
        if( ImGui::Checkbox("K7", &k7) )
            touchSensor.mockFilteredData(K7Pin, ctouchThrVal + (!k7 ? 100 : -100));
    }

    ImGui::End();
}


static uint8_t displayBuffer[128*128];

static void GetDisplay()
{
    SDL_memset( displayBuffer, 0, (128*128));

    if(display.enabled_) {
        uint8_t fg = 255;
        uint8_t bg = 0;

        if( display.dimmed_) fg = 127;
        if( display.inverted_ ) { uint8_t tmp = fg; fg = bg; bg = tmp; }

        for(int y = 0 ; y < 64; ++y) {
            for(int x = 0 ; x < 128; ++x) {
                uint8_t color = display.getPixel(x,y) ? fg : bg;
                displayBuffer[x+y*128] = color;
            }
        }
    }

    glBindTexture( GL_TEXTURE_2D, displayTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, displayBuffer);

    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        printf("glerror %d\n", error);
    }
    glBindTexture( GL_TEXTURE_2D, 0);
}

static void toggleAnalogAnimation() {
    animateAnalogs = !animateAnalogs;
    printf("Analog input variations: %s\n", animateAnalogs ? "ON": "OFF");
}


static void SimLoop(std::function<bool()> continue_predicate, std::function<void()> loopFunc)
{
    int doQuit = 0;
    uint32_t time;
    while( continue_predicate() ) {

        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            ImGui_ImplSDL2_ProcessEvent(&event);

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

                    case SDLK_1:        touchSensor.mockFilteredData(K1Pin, ctouchThrVal -100); break;
                    case SDLK_2:        touchSensor.mockFilteredData(K2Pin, ctouchThrVal -100); break;
                    case SDLK_3:        touchSensor.mockFilteredData(K3Pin, ctouchThrVal -100); break;
                    case SDLK_4:        touchSensor.mockFilteredData(K4Pin, ctouchThrVal -100); break;
                    case SDLK_5:        touchSensor.mockFilteredData(K5Pin, ctouchThrVal -100); break;
                    case SDLK_6:        touchSensor.mockFilteredData(K6Pin, ctouchThrVal -100); break;
                    case SDLK_7:        touchSensor.mockFilteredData(K7Pin, ctouchThrVal -100); break;
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
                    case SDLK_w:        toggleAnalogAnimation(); break;

                    case SDLK_1:        touchSensor.mockFilteredData(K1Pin, ctouchThrVal +100); break;
                    case SDLK_2:        touchSensor.mockFilteredData(K2Pin, ctouchThrVal +100); break;
                    case SDLK_3:        touchSensor.mockFilteredData(K3Pin, ctouchThrVal +100); break;
                    case SDLK_4:        touchSensor.mockFilteredData(K4Pin, ctouchThrVal +100); break;
                    case SDLK_5:        touchSensor.mockFilteredData(K5Pin, ctouchThrVal +100); break;
                    case SDLK_6:        touchSensor.mockFilteredData(K6Pin, ctouchThrVal +100); break;
                    case SDLK_7:        touchSensor.mockFilteredData(K7Pin, ctouchThrVal +100); break;

                }
                fflush(stdout);
            }
        }

        if(doQuit)
            break;

        time = SDL_GetTicks();

        analogUpdate(SDL_GetTicks());

        if(loopFunc) loopFunc();

        // TODO: Get buffer from SSD1306 and copy to surface...

        GetDisplay();

        glClearColor(0.0f, 1.0f, 0.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Render UI here..
        doInputWindow();
        doGlobalsWindow();


        ImGui::Begin("NuEVI display");
        ImVec2 size( 128*scale, 64*scale );
        ImVec2 uv0(0,0);
        ImVec2 uv1(1, 0.5f);
        uint64_t tmp = displayTexture;  // Avoid warning as void* is larger than GLuint
        ImGui::Image( (void*)tmp, size, uv0, uv1 );

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        uint32_t timePassed = SDL_GetTicks() - time;
        if( timePassed < 16 ) {
            SDL_Delay( 16-timePassed );
        }
    }
}

static int SimRun( )
{
	if( 0 != SimInit() ) { return 1; }

    // Dummy to always force full reset of EEPROM, to circumvent bug in NuEVI.ino 
    digitalInputs[mPin] = 0;
    digitalInputs[ePin] = 0;

    setup();

    digitalInputs[mPin] = 1;
    digitalInputs[ePin] = 1;

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

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);


	window = SDL_CreateWindow( "NuEVI Simulator"
		, SDL_WINDOWPOS_UNDEFINED
		, SDL_WINDOWPOS_UNDEFINED
		, 1024
		, 768
		, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

	if( window == NULL ) {
		fprintf(stderr, "Could not create SDL window");
		SimQuit();
		return 2;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
    gl3wInit();

    ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 150");

    glGenTextures(1, &displayTexture);
    glBindTexture( GL_TEXTURE_2D, displayTexture);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    memset(digitalInputs, 1, sizeof(digitalInputs));

    analogInputs[vMeterPin] = 3025;

    // Initialize touch sensors to not be poked 
    for(int i = 0; i < 12; ++i) {
        touchSensor.mockFilteredData(i, 4095);
    }

    return result;
}

static void SimQuit()
{
    printf("Leaving Sim, see you later!\n");

    if( window != NULL ) {
        SDL_DestroyWindow( window );
        // SDL_FreeSurface( surface );
    }
    SDL_Quit();
}

#include "NuEVI.ino"


int main()
{
    return SimRun();
}
