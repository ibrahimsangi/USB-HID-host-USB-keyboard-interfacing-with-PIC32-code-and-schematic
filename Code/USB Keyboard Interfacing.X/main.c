/* 
 * File:   main.c
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 14, 2013, 5:14 AM
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "usb_config.h"
#include "USB/usb.h"
#include "USB/usb_host_hid_parser.h"
#include "LCD/LCD.h"
#include "LCD/Delay.h"
#include "Common/GenericTypeDefs.h"
#include "USB/usb_host_hid.h"
#include "KeyBoardRoutine.h"

/*
 * 
 */
#if defined( __PIC32MX__ )

    #pragma config UPLLEN   = ON            // USB PLL Enabled
    #pragma config FPLLMUL  = MUL_15        // PLL Multiplier
    #pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider
    #pragma config FPLLIDIV = DIV_2         // PLL Input Divider
    #pragma config FPLLODIV = DIV_1         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FWDTEN   = OFF           // Watchdog Timer
    #pragma config WDTPS    = PS1           // Watchdog Timer Postscale
    #pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
    #pragma config OSCIOFNC = OFF           // CLKO Enable
    #pragma config POSCMOD  = HS            // Primary Oscillator
    #pragma config IESO     = OFF           // Internal/External Switch-over
    #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (KLO was off)
    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = OFF           // Boot Flash Write Protect
    #pragma config PWP      = OFF           // Program Flash Write Protect
    #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select

#endif

// We are taking Timer 3  to schedule input report transfers

// NOTE - The datasheet doesn't state this, but the timer does get reset to 0
// after a period register match.  So we don't have to worry about resetting
// the timer manually.

#define STOP_TIMER_IN_IDLE_MODE     0x2000
#define TIMER_SOURCE_INTERNAL       0x0000
#define TIMER_ON                    0x8000
#define GATED_TIME_DISABLED         0x0000
#define TIMER_16BIT_MODE            0x0000

#define TIMER_PRESCALER_1           0x0000
#define TIMER_PRESCALER_8           0x0010
#define TIMER_PRESCALER_64          0x0020
#define TIMER_PRESCALER_256         0x0030
#define TIMER_INTERRUPT_PRIORITY    0x0001

extern BOOL ReportBufferUpdated;
extern BOOL LED_Key_Pressed;
extern BOOL DisplayConnectOnce;
extern BOOL DisplayDeatachOnce;
extern BYTE CAPS_Lock_Pressed;
extern BYTE NUM_Lock_Pressed;
extern BYTE HeldKeyCount;
extern BYTE HeldKey;
extern BYTE LineNo;
extern BYTE currCharPos;
extern BYTE FirstKeyPressed ;

extern BYTE ErrorCounter;
extern BYTE ErrorDriver;
extern BYTE NumOfBytesRcvd;

extern HID_DATA_DETAILS Appl_LED_Indicator;

extern HID_DATA_DETAILS Appl_ModifierKeysDetails;
extern HID_DATA_DETAILS Appl_NormalKeysDetails;

extern HID_USER_DATA_SIZE Appl_BufferModifierKeys[8];
extern HID_USER_DATA_SIZE Appl_BufferNormalKeys[6];
extern HID_USER_DATA_SIZE Appl_ShadowBuffer1[6];

extern HID_REPORT_BUFFER     Appl_raw_report_buffer;
extern HID_LED_REPORT_BUFFER Appl_led_report_buffer;

//******************************************************************************
//******************************************************************************
// Main
//******************************************************************************
//******************************************************************************

extern APP_STATE App_State_Keyboard;

int main (void)
{
    BYTE i;
    int  value;

    value = SYSTEMConfigWaitStatesAndPB( GetSystemClock() );

    // Enable the cache for the best performance
    CheKseg0CacheOn();

    INTEnableSystemMultiVectoredInt();

    value = OSCCON;
    while (!(value & 0x00000020))
        value = OSCCON;    // Wait for PLL lock to stabilize
    
    USBInitialize(0);           // Initialize USB layers

    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;

    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;

    Initialize_LCD();
    WriteString_LCD("     Welcome To");
    MoveCursorToPosition(SecondLine);
    WriteString_LCD("     IbrahimLabs");
    MoveCursorToPosition(ThirdLine);
    WriteString_LCD("    Mini Computer");
    MoveCursorToPosition(FourthLine);
    WriteString_LCD("using PIC32 USB host");
    __Delay_ms(3000);

    
    
    while(1)
    {
        LATBbits.LATB1 = 1;
        LATBbits.LATB0 = 0;
        USBTasks();
        App_Detect_Device();
        switch(App_State_Keyboard)
        {
            case DEVICE_NOT_CONNECTED:
                USBTasks();
                if(DisplayDeatachOnce == FALSE)
                {
                    WriteCmd_LCD(0x0C);  /*  Display ON; Cursor OFF; Blink ON   */
                    ClearScreen_LCD();
                    MoveCursorToPosition(FirstLine);
                    WriteString_LCD("     KeyBoard Not");
                    MoveCursorToPosition(SecondLine);
                    WriteString_LCD("       found");
                    MoveCursorToPosition(ThirdLine);
                    WriteString_LCD("   Please connect");
                    MoveCursorToPosition(FourthLine);
                    WriteString_LCD("   a USB Keyboard ");
                    DisplayDeatachOnce = TRUE;
                }
                if(USBHostHID_ApiDeviceDetect()) /* True if report descriptor is parsed with no error */
                {
                    App_State_Keyboard = DEVICE_CONNECTED;
                    DisplayConnectOnce = FALSE;
                }
                break;
            case DEVICE_CONNECTED:
                App_State_Keyboard = READY_TO_TX_RX_REPORT;
                if(DisplayConnectOnce == FALSE)
                {
                    ClearScreen_LCD();
                    MoveCursorToPosition(FirstLine);
                    WriteString_LCD("  KeyBoard Found");
                    MoveCursorToPosition(SecondLine);
                    WriteString_LCD("  USB HID Host App");
                    MoveCursorToPosition(ThirdLine);
                    WriteString_LCD("Please wait 1 second");
                    __Delay_ms(1000);
                    ClearScreen_LCD();
                    LineNo = 1;
                    WriteCmd_LCD(0x0D);  /*  Display ON; Cursor OFF; Blink ON   */
                    DisplayConnectOnce = TRUE;
                    DisplayDeatachOnce = FALSE;
                }
                InitializeTimer();              // start 10ms timer to schedule input reports

                break;
            case READY_TO_TX_RX_REPORT:
                if(!USBHostHID_ApiDeviceDetect())
                {
                    App_State_Keyboard = DEVICE_NOT_CONNECTED;
                    //                                DisplayOnce = FALSE;
                }
                break;
            case GET_INPUT_REPORT:
                if(USBHostHID_ApiGetReport(Appl_raw_report_buffer.Report_ID,Appl_ModifierKeysDetails.interfaceNum,
                            Appl_raw_report_buffer.ReportSize, Appl_raw_report_buffer.ReportData))
                {
                    ;
                }
                else
                {
                    App_State_Keyboard = INPUT_REPORT_PENDING;
                }
                USBTasks();
                break;
            case INPUT_REPORT_PENDING:
                if(USBHostHID_ApiTransferIsComplete(&ErrorDriver,&NumOfBytesRcvd))
                {
                    if(ErrorDriver ||(NumOfBytesRcvd !=     Appl_raw_report_buffer.ReportSize ))
                    {
                        ErrorCounter++ ;
                        if(MAX_ERROR_COUNTER <= ErrorDriver)
                            App_State_Keyboard = ERROR_REPORTED;
                        else
                            App_State_Keyboard = READY_TO_TX_RX_REPORT;
                    }
                    else
                    {
                        ErrorCounter = 0;
                        ReportBufferUpdated = TRUE;
                        App_State_Keyboard = READY_TO_TX_RX_REPORT;

                        if(DisplayConnectOnce == TRUE)
                        {
                            for(i=0;i<Appl_raw_report_buffer.ReportSize;i++)
                            {
                                if(Appl_raw_report_buffer.ReportData[i] != 0)
                                {
                                    DisplayConnectOnce = FALSE;
                                }
                            }
                        }

                        App_ProcessInputReport();
                        App_PrepareOutputReport();
                    }
                }
                break;

            case SEND_OUTPUT_REPORT: /* Will be done while implementing Keyboard */
                if(USBHostHID_ApiSendReport(Appl_LED_Indicator.reportID,\
                        Appl_LED_Indicator.interfaceNum, Appl_LED_Indicator.reportLength,\
                        (BYTE*)&Appl_led_report_buffer))
                {
                    /* Host may be busy/error -- keep trying */
                }
                else
                {
                    App_State_Keyboard = OUTPUT_REPORT_PENDING;
                }
                USBTasks();

                break;
            case OUTPUT_REPORT_PENDING:
                if(USBHostHID_ApiTransferIsComplete(&ErrorDriver,&NumOfBytesRcvd))
                {
                    if(ErrorDriver)
                    {
                        ErrorCounter++ ;
                        if(MAX_ERROR_COUNTER <= ErrorDriver)
                            App_State_Keyboard = ERROR_REPORTED;

                        //                                App_State_Keyboard = READY_TO_TX_RX_REPORT;
                    }
                    else
                    {
                        ErrorCounter = 0;
                        App_State_Keyboard = READY_TO_TX_RX_REPORT;
                    }
                }
                break;

            case ERROR_REPORTED:
                break;
            default:
                break;

        }
    }
}

#pragma interrupt _T4Interrupt ipl4 vector 16
void _T4Interrupt( void )
{
    if (IFS0bits.T4IF)
    {
        IFS0bits.T4IF   = 0;
        if(READY_TO_TX_RX_REPORT == App_State_Keyboard)
        {
            App_State_Keyboard = GET_INPUT_REPORT; // If no report is pending schedule new request
        }
    }
}
