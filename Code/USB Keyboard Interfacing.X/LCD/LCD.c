/* 
 * File:   LCD.c
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 7, 2013, 1:06 PM
 */
#define SYS_FREQ            (80000000L)                     // 80MHz system clock

#include <stddef.h>
#include <xc.h>
#include <plib.h>
#include "LCD.h"
#include "Delay.h"

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void Initialize_LCD(void)
{
    /*  making Pins as output   */
    E_PinOutput;
    RS_PinOutput;
    LCD_BusOutput;

    /*  Writing zero to pins and port   */
    ClearEnablePin;
    ClearRSPin;
    LCD_BUS (0);

   __Delay_ms(10);          /*  ten miliseconds delay   */

   /*   writing commonds for initialization of LCD  */
   WriteCmd_LCD(0x38);  /*  Functions Set as Given in Datasheet */
   WriteCmd_LCD(0x0C);  /*  Display ON; Cursor OFF; Blink OFF   */
   WriteCmd_LCD(0x06);  /*  Display Shifting OFF                */
   WriteCmd_LCD(0x01);  /*  Clear Display                       */
   __Delay_ms(10);
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

void ClearScreen_LCD(void)
{
    WriteCmd_LCD(0x01);         /*  Clear Screen command    */
    __Delay_ms(10);              /*  Delay for cursor to return home must be
                                 *  atleast 3ms menstioned in datasheet
                                 */
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

void Toggle_Enable_Pin_LCD(void)
{
    /*  toggling Enable PIN is must for data to be displayed on screen
        After every Command and data for more details see datasheet
    */
    SetEnablePin;
    __Delay_us(100);
    ClearEnablePin;
    __Delay_us(100);

}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

void WriteCmd_LCD(unsigned char Cmd)
{
    ClearRSPin;         /*  For command RS must be low (0)      */
    __Delay_us(5);
    LCD_BUS(Cmd);       /*  write Command to data bus of LCD    */
    __Delay_us(5);

    Toggle_Enable_Pin_LCD();
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

void WriteData_LCD(unsigned char Data)
{
    SetRSPin;           /*  For data RS must be high (1)    */
    __Delay_us(5);
    LCD_BUS(Data);    /*  write data to data bus of LCD   */
    __Delay_us(5);
    Toggle_Enable_Pin_LCD();
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

void WriteString_LCD(const char *String)
{
    while(*String != '\0')
    {
        WriteData_LCD(*String++);   /*  Display data untill string ends */
    }
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

unsigned char MoveCursorToPosition(unsigned char Address)
{
    /*  valid addresses are for line one 0x80 and line two are 0xC0   */
    if ((Address >= 0x80 && Address <= 0xA8) || (Address >= 0xC0 && Address <= 0xE8))
    {
        WriteCmd_LCD(Address);
        __Delay_ms(15);
        return 1;
    }
    else
        return 0;
}
