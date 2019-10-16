#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

//Dummy functions, used by macros for interrupts() / noInterrupts()
void __enable_irq() {}
void __disable_irq() {}


struct IntervalTimer
{
    public:
        IntervalTimer() {};
        bool begin(void (*funct)(), unsigned int microseconds) { };

};

#endif
