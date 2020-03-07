
/*
 * File:   KeyBoardRoutine.h
 *
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 22, 2013, 7:38 AM
 */

#ifndef KEYBOARDROUTINE_H
#define	KEYBOARDROUTINE_H

#ifdef	__cplusplus
extern "C" {
#endif


typedef enum _APP_STATE
{
    DEVICE_NOT_CONNECTED,
    DEVICE_CONNECTED, /* Device Enumerated  - Report Descriptor Parsed */
    READY_TO_TX_RX_REPORT,
    GET_INPUT_REPORT, /* perform operation on received report */
    INPUT_REPORT_PENDING,
    SEND_OUTPUT_REPORT, /* Not needed in case of mouse */
    OUTPUT_REPORT_PENDING,
    ERROR_REPORTED
} APP_STATE;

typedef struct _HID_REPORT_BUFFER
{
    WORD  Report_ID;
    WORD  ReportSize;
    BYTE* ReportData;
    WORD  ReportPollRate;
}   HID_REPORT_BUFFER;

typedef struct _HID_LED_REPORT_BUFFER
{
    BYTE  NUM_LOCK      : 1;
    BYTE  CAPS_LOCK     : 1;
    BYTE  SCROLL_LOCK   : 1;
    BYTE  UNUSED        : 5;
}   HID_LED_REPORT_BUFFER;




#define MAX_ALLOWED_CURRENT             (500)         // Maximum power we can supply in mA
#define MINIMUM_POLL_INTERVAL           (0x0A)        // Minimum Polling rate for HID reports is 10ms

#define USAGE_PAGE_LEDS                 (0x08)

#define USAGE_PAGE_KEY_CODES            (0x07)
#define USAGE_MIN_MODIFIER_KEY          (0xE0)
#define USAGE_MAX_MODIFIER_KEY          (0xE7)

#define USAGE_MIN_NORMAL_KEY            (0x00)
#define USAGE_MAX_NORMAL_KEY            (0xFF)

/* Array index for modifier keys */
#define MODIFIER_LEFT_CONTROL           (0)
#define MODIFIER_LEFT_SHIFT             (1)
#define MODIFIER_LEFT_ALT               (2)
#define MODIFIER_LEFT_GUI               (3)
#define MODIFIER_RIGHT_CONTROL          (4)
#define MODIFIER_RIGHT_SHIFT            (5)
#define MODIFIER_RIGHT_ALT              (6)
#define MODIFIER_RIGHT_GUI              (7)

#define HID_CAPS_LOCK_VAL               (0x39)
#define HID_NUM_LOCK_VAL                (0x53)

#define MAX_ERROR_COUNTER               (10)

//******************************************************************************
//  macros to identify special charaters(other than Digits and Alphabets)
//******************************************************************************
#define Symbol_Exclamation              (0x1E)
#define Symbol_AT                       (0x1F)
#define Symbol_Pound                    (0x20)
#define Symbol_Dollar                   (0x21)
#define Symbol_Percentage               (0x22)
#define Symbol_Cap                      (0x23)
#define Symbol_AND                      (0x24)
#define Symbol_Star                     (0x25)
#define Symbol_NormalBracketOpen        (0x26)
#define Symbol_NormalBracketClose       (0x27)

#define Symbol_Return                   (0x28)
#define Symbol_Escape                   (0x29)
#define Symbol_Backspace                (0x2A)
#define Symbol_Tab                      (0x2B)
#define Symbol_Space                    (0x2C)
#define Symbol_HyphenUnderscore         (0x2D)
#define Symbol_EqualAdd                 (0x2E)
#define Symbol_BracketOpen              (0x2F)
#define Symbol_BracketClose             (0x30)
#define Symbol_BackslashOR              (0x31)
#define Symbol_SemiColon                (0x33)
#define Symbol_InvertedComma            (0x34)
#define Symbol_Tilde                    (0x35)
#define Symbol_CommaLessThan            (0x36)
#define Symbol_PeriodGreaterThan        (0x37)
#define Symbol_FrontSlashQuestion       (0x38)


BYTE App_HID2ASCII(BYTE a); //convert USB HID code (buffer[2 to 7]) to ASCII code
void AppInitialize(void);
BOOL AppGetParsedReportDetails(void);
void App_Detect_Device(void);
void App_ProcessInputReport(void);
void App_PrepareOutputReport(void);
void InitializeTimer(void);
void App_Clear_Data_Buffer(void);
BOOL App_CompareKeyPressedPrevBuf(BYTE data);
void App_CopyToShadowBuffer(void);
BOOL USB_HID_DataCollectionHandler(void);
void LCDDisplayString(BYTE* data, BYTE lineNum);
void LCD_Display_Routine(BYTE data, BYTE HIDData);
void ChangeLine(void);




#ifdef	__cplusplus
}
#endif

#endif	/* KEYBOARDROUTINE_H */

