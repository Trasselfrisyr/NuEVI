#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

//Dummy functions, used by macros for interrupts() / noInterrupts()
void __enable_irq() {}
void __disable_irq() {}


struct IntervalTimer
{
    IntervalTimer() { };
    bool begin(void (* __attribute__((unused)) func)() ,unsigned int __attribute__((unused)) microseconds) { return true; }
};

#endif
