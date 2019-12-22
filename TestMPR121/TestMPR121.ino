#include <Wire.h>
#include <Adafruit_MPR121.h>

/*
    Tool to test an MPR121 unit. Wires each touch input to a teensy pin to simulate touch sensor signals.
    Used to detect faulty MPR121 units before assembly.
*/

Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor

//LED output pins. Red is the onboard one, other two are just wired via resistors to +3.3V
#define RED_LED 13
#define GREEN_LED 20
#define YELLOW_LED 21

void setup(void) {
    Serial.begin(115200);

    //Set all "test ports" to high-impedance for now
    for(int i=0; i<=11; i++) {
        pinMode(i, INPUT);
    }

    pinMode(RED_LED, OUTPUT); led(RED_LED, true);
    pinMode(GREEN_LED, OUTPUT); led(GREEN_LED, true);
    pinMode(YELLOW_LED, OUTPUT); led(YELLOW_LED, true);

    delay(100);
    int serialwait = 100; //Max time to wait for serial port initialization
    while(!Serial && serialwait)
    {
        delay(1);
        --serialwait;
    }

    led(RED_LED, false);
    led(GREEN_LED, false);
    led(YELLOW_LED, false);

    Serial.println("Running MPR121 test");

    if (!touchSensor.begin(0x5A)) {
        //Bail out if MPR121 cannot be initialized (chip broken or not present).
        Serial.println("MPR121 initialization failed!");
        failure();
        return;
    } else {
        Serial.println("MPR121 init ok.");
    }

    //Attempt some kind of random seeding
    srandom(analogRead(0) ^ millis());
    delay(200);

    run_all();
}

void led(uint8_t pin, bool state) {
    //Onboard red LED is wired a bit differently
    if(pin != RED_LED) {
        digitalWrite(pin, state?LOW:HIGH);
    } else {
        digitalWrite(pin, state?HIGH:LOW);
    }
}

void set_touch_pins(uint16_t value)
{
    for(int pin=0; pin<12; ++pin)
    {
        uint16_t touched = (value >> pin) & 0x0001; //Mask out bit value of pin
        int teensyPin = 11-pin; //Row of teensy pins is 0-11 but mirrored vs MPR121

        if(touched) {
            pinMode(teensyPin, INPUT_PULLDOWN);
        } else {
            pinMode(teensyPin, INPUT_PULLUP);
        }
    }
}

bool run_test(const char* name, uint16_t value) {
    Serial.print(name);
    set_touch_pins(value);
    led(YELLOW_LED, true);
    delay(50);
    led(YELLOW_LED, false);
    delay(50);
    uint16_t t = touchSensor.touched();
    Serial.print(", expected: ");
    Serial.print(value, HEX);
    Serial.print(" got: ");
    Serial.print(t, HEX);

    if(value == t) {
        Serial.println(" PASS");
        return true;
    } else {
        Serial.println(" FAIL");
        return false;
    }
}

void failure() {
    led(RED_LED, true);
    led(GREEN_LED, false);
    Serial.println("Test failed!");
}

void success() {
    led(RED_LED, false);
    led(GREEN_LED, true);
    Serial.println("All tests succeeded!");
}

//Macro to bail out on any failure
#define TEST(x) if(!x) { failure(); return; }

void run_all(void) {

    TEST(run_test("ALL_OFF", 0x000));
    TEST(run_test("ALL_ON", 0xFFF));

    //Pins one at a time
    TEST(run_test("P0", 0x001 << 0));
    TEST(run_test("P1", 0x001 << 1));
    TEST(run_test("P2", 0x001 << 2));
    TEST(run_test("P3", 0x001 << 3));
    TEST(run_test("P4", 0x001 << 4));
    TEST(run_test("P5", 0x001 << 5));
    TEST(run_test("P6", 0x001 << 6));
    TEST(run_test("P7", 0x001 << 7));
    TEST(run_test("P8", 0x001 << 8));
    TEST(run_test("P9", 0x001 << 9));
    TEST(run_test("P10", 0x001 << 10));
    TEST(run_test("P11", 0x001 << 11));

    //Alternating pattern
    TEST(run_test("ODD", 0x0555));
    TEST(run_test("EVEN", 0x0AAA));

    //A few random ones for good measure
    for(int i=0; i<8; i++) {
        TEST(run_test("RANDOM", random() % 0x1000));
    }

    success();
}

//Dummy loop does nothing
void loop() {
    delay(100);
}