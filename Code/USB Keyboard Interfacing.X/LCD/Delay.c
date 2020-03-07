/* 
 * File:   Delay.c
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 8, 2013, 5:28 AM
 */

#include "Delay.h"
#include <plib.h>

// PreProcessors
#define SYS_FREQ            (80000000L)                     // 80MHz system clock

void __Delay_ms(unsigned long Period_ms)
{
    unsigned long x = 0;
    
    x = (Period_ms * (SYS_FREQ/2/1000));

    OpenCoreTimer(x);

    while (!mCTGetIntFlag());           // wait for flag to set
    mCTClearIntFlag();                  // clear CoreTimer flag
    
    CloseCoreTimer();
}

void __Delay_us(unsigned long Period_us)
{
    unsigned long x = 0;

    x = (Period_us * (SYS_FREQ/2/1000000));

    OpenCoreTimer(x);

    while (!mCTGetIntFlag());           // wait for flag to set
    mCTClearIntFlag();                  // clear CoreTimer flag

    CloseCoreTimer();
}