
/*
 * File:   Delay.h
 *
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 8, 2013, 5:28 AM
 */

#ifndef DELAY_H
#define	DELAY_H

#ifdef	__cplusplus
extern "C" {
#endif


#if !defined(SYS_FREQ)
extern void __Delay_ms(unsigned long);
extern void __Delay_us(unsigned long);
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* DELAY_H */

