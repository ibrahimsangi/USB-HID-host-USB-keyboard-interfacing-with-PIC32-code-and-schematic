
/*
 * File:   LCD.h
 *
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 7, 2013, 1:06 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

 /*  *****************************************************************   */

    /*  DATA bus tristate register  */
    #define LCD_BusOutput   mPORTESetPinsDigitalOut(BIT_0 | BIT_1 | BIT_2 | BIT_3 |\
                                                        BIT_4 | BIT_5 | BIT_6 | BIT_7)
    /*  Enable and Register select as output */
    #define E_PinOutput     mPORTGSetPinsDigitalOut(BIT_12)
    #define RS_PinOutput    mPORTGSetPinsDigitalOut(BIT_13)

    /*  Enable Pin  */
    #define SetEnablePin    mPORTGSetBits(BIT_12)
    #define ClearEnablePin  mPORTGClearBits(BIT_12)

    /*  Register Select Pin  */
    #define SetRSPin        mPORTGSetBits(BIT_13)
    #define ClearRSPin      mPORTGClearBits(BIT_13)

    /*  DATA BUS FOR LCD 8bits  */
    #define LCD_BUS(_value)     (mPORTEWrite((unsigned char)(_value)))


    #define FirstLine   0x80
    #define SecondLine  0xC0
    #define ThirdLine   0x94
    #define FourthLine  0xD4


/*--------------------------------------------------------------------------------------------*/

    /*  Functions for LCD   */

    void Initialize_LCD(void);
    void ClearScreen_LCD(void);
    void Toggle_Enable_Pin_LCD(void);
    void WriteCmd_LCD(unsigned char Cmd);
    void WriteData_LCD(unsigned char Data);
    void WriteString_LCD(const char *String);
    unsigned char MoveCursorToPosition(unsigned char Address);

/*  *****************************************************************   */


#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

