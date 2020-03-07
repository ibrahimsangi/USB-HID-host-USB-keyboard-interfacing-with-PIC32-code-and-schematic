/* 
 * File:   KeyBoardRoutines.c
 * Author: IBRAHIM LABS
 *
 * Website: http://ibrahimlabs.blogspot.com/
 *
 * Created on September 22, 2013, 7:35 AM
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "usb_config.h"
#include "USB/usb.h"
#include "USB/usb_host_hid_parser.h"
#include "USB/usb_host_hid.h"
#include "LCD/LCD.h"
#include "LCD/Delay.h"
#include "Common/GenericTypeDefs.h"
#include "KeyBoardRoutine.h"


APP_STATE App_State_Keyboard = DEVICE_NOT_CONNECTED;

HID_DATA_DETAILS Appl_LED_Indicator;

HID_DATA_DETAILS Appl_ModifierKeysDetails;
HID_DATA_DETAILS Appl_NormalKeysDetails;

HID_USER_DATA_SIZE Appl_BufferModifierKeys[8];
HID_USER_DATA_SIZE Appl_BufferNormalKeys[6];
HID_USER_DATA_SIZE Appl_ShadowBuffer1[6];

HID_REPORT_BUFFER     Appl_raw_report_buffer;
HID_LED_REPORT_BUFFER Appl_led_report_buffer;

BOOL ReportBufferUpdated;
BOOL LED_Key_Pressed = FALSE;
BOOL DisplayConnectOnce = FALSE;
BOOL DisplayDeatachOnce = FALSE;
BYTE CAPS_Lock_Pressed = 0;
BYTE NUM_Lock_Pressed = 0;
BYTE HeldKeyCount = 0;
BYTE HeldKey;
BYTE LineNo = 0;
BYTE currCharPos;
BYTE FirstKeyPressed ;

BYTE ErrorCounter;
BYTE ErrorDriver;
BYTE NumOfBytesRcvd;


//******************************************************************************
//******************************************************************************
// USB Support Functions
//******************************************************************************
//******************************************************************************

BOOL USB_ApplicationEventHandler( BYTE address, USB_EVENT event, void *data, DWORD size )
{
    switch( (INT)event )
    {
        case EVENT_VBUS_REQUEST_POWER:
            // The data pointer points to a byte that represents the amount of power
            // requested in mA, divided by two.  If the device wants too much power,
            // we reject it.
            if (((USB_VBUS_POWER_EVENT_DATA*)data)->current <= (MAX_ALLOWED_CURRENT / 2))
            {
                return TRUE;
            }
            else
            {
                UART2PrintString( "\r\n***** USB Error - device requires too much current *****\r\n" );
            }
            break;

        case EVENT_VBUS_RELEASE_POWER:
            // Turn off Vbus power.
            // The PIC24F with the Explorer 16 cannot turn off Vbus through software.
            return TRUE;
            break;

        case EVENT_HUB_ATTACH:
            UART2PrintString( "\r\n***** USB Error - hubs are not supported *****\r\n" );
            return TRUE;
            break;

        case EVENT_UNSUPPORTED_DEVICE:
            UART2PrintString( "\r\n***** USB Error - device is not supported *****\r\n" );
            return TRUE;
            break;

        case EVENT_CANNOT_ENUMERATE:
            UART2PrintString( "\r\n***** USB Error - cannot enumerate device *****\r\n" );
            return TRUE;
            break;

        case EVENT_CLIENT_INIT_ERROR:
            UART2PrintString( "\r\n***** USB Error - client driver initialization error *****\r\n" );
            return TRUE;
            break;

        case EVENT_OUT_OF_MEMORY:
            UART2PrintString( "\r\n***** USB Error - out of heap memory *****\r\n" );
            return TRUE;
            break;

        case EVENT_UNSPECIFIED_ERROR:   // This should never be generated.
            UART2PrintString( "\r\n***** USB Error - unspecified *****\r\n" );
            return TRUE;
            break;

		case EVENT_HID_RPT_DESC_PARSED:
			 #ifdef APPL_COLLECT_PARSED_DATA
			     return(APPL_COLLECT_PARSED_DATA());
		     #else
				 return TRUE;
			 #endif
			break;

        default:
            break;
    }
    return FALSE;
}

/****************************************************************************
  Function:
    void App_PrepareOutputReport(void)

  Description:
    This function schedules output report if any LED indicator key is pressed.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
void App_PrepareOutputReport(void)
{
//    if((READY_TO_TX_RX_REPORT == App_State_Keyboard) && (ReportBufferUpdated == TRUE))
    if(ReportBufferUpdated == TRUE)
    {
        ReportBufferUpdated = FALSE;
        if(LED_Key_Pressed)
        {
            App_State_Keyboard = SEND_OUTPUT_REPORT;
            LED_Key_Pressed = FALSE;
        }
     }
}

/****************************************************************************
  Function:
    void App_ProcessInputReport(void)

  Description:
    This function processes input report received from HID device.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
void App_ProcessInputReport(void)
{
    BYTE  i;
    BYTE  data;
   /* process input report received from device */
    USBHostHID_ApiImportData(Appl_raw_report_buffer.ReportData, Appl_raw_report_buffer.ReportSize
                          ,Appl_BufferModifierKeys, &Appl_ModifierKeysDetails);
    USBHostHID_ApiImportData(Appl_raw_report_buffer.ReportData, Appl_raw_report_buffer.ReportSize
                          ,Appl_BufferNormalKeys, &Appl_NormalKeysDetails);

    for(i=0;i<(sizeof(Appl_BufferNormalKeys)/sizeof(Appl_BufferNormalKeys[0]));i++)
    {
        if(Appl_BufferNormalKeys[i] != 0)
        {
            if(Appl_BufferNormalKeys[i] == HID_CAPS_LOCK_VAL)
            {
               CAPS_Lock_Pressed = !CAPS_Lock_Pressed;
               LED_Key_Pressed = TRUE;
               Appl_led_report_buffer.CAPS_LOCK = CAPS_Lock_Pressed;
            }else if(Appl_BufferNormalKeys[i] == HID_NUM_LOCK_VAL)
            {
                NUM_Lock_Pressed = !NUM_Lock_Pressed;
                LED_Key_Pressed = TRUE;
                Appl_led_report_buffer.NUM_LOCK = NUM_Lock_Pressed;
            }else
            {
            /* check if key press was present in previous report */
            // if key press was pressed in previous report neglect it ????
               if(!App_CompareKeyPressedPrevBuf(Appl_BufferNormalKeys[i]))
                {
                    data = App_HID2ASCII(Appl_BufferNormalKeys[i]);  // convert data to ascii
                    LCD_Display_Routine(data,Appl_BufferNormalKeys[i] );
                }
            }
        }
        else
        {
            if(i==0)
            {
                HeldKeyCount = 0;
            }
            else
            {
                if(Appl_BufferNormalKeys[i-1] == HeldKey)
                {
                    if(HeldKeyCount < 3)
                    {
                        HeldKeyCount++;
                    }
                    else
                    {
                        data = App_HID2ASCII(HeldKey);  // convert data to ascii
                        LCD_Display_Routine(data,HeldKey );
                    }
                }
                else
                {
                    HeldKeyCount = 0;
                    HeldKey = Appl_BufferNormalKeys[i-1];
                }
            }
            break;
        }
      }
      App_CopyToShadowBuffer();
      App_Clear_Data_Buffer();
}

/****************************************************************************
  Function:
    void App_CopyToShadowBuffer(void)

  Description:
    This function updates the shadow buffers with previous reports.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
void App_CopyToShadowBuffer(void)
{
    BYTE i;

    for(i=0;i<(sizeof(Appl_BufferNormalKeys)/sizeof(Appl_BufferNormalKeys[0]));i++)
    {
        Appl_ShadowBuffer1[i] = Appl_BufferNormalKeys[i];
    }
}

/****************************************************************************
  Function:
    BOOL App_CompareKeyPressedPrevBuf(BYTE data)

  Description:
    This function compares if the data byte received in report was sent in
    previous report. This is to avoid duplication incase user key in strokes
    at fast rate.

  Precondition:
    None

  Parameters:
    BYTE data         -   data byte that needs to be compared with previous
                          report

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
BOOL App_CompareKeyPressedPrevBuf(BYTE data)
{
    BYTE i;

    for(i=0;i<(sizeof(Appl_BufferNormalKeys)/sizeof(Appl_BufferNormalKeys[0]));i++)
    {
        if(data == Appl_ShadowBuffer1[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}


/****************************************************************************
  Function:
    void App_Detect_Device(void)

  Description:
    This function monitors the status of device connected/disconnected

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
void App_Detect_Device(void)
{
  if(!USBHostHID_ApiDeviceDetect())
  {
     App_State_Keyboard = DEVICE_NOT_CONNECTED;
  }
}

/****************************************************************************
  Function:
    void App_Clear_Data_Buffer(void)

  Description:
    This function clears the content of report buffer after reading

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
void App_Clear_Data_Buffer(void)
{
    BYTE i;

    for(i=0;i<(sizeof(Appl_BufferNormalKeys)/sizeof(Appl_BufferNormalKeys[0]));i++)
    {
        Appl_BufferNormalKeys[i] = 0;
    }

    for(i=0;i<Appl_raw_report_buffer.ReportSize;i++)
    {
        Appl_raw_report_buffer.ReportData[i] = 0;
    }
}

/****************************************************************************
  Function:
    BYTE App_HID2ASCII(BYTE a)
  Description:
    This function converts the HID code of the key pressed to coressponding
    ASCII value. For Key strokes like Esc, Enter, Tab etc it returns 0.

  Precondition:
    None

  Parameters:
    BYTE a          -   HID code for the key pressed

  Return Values:
    BYTE            -   ASCII code for the key pressed

  Remarks:
    None
***************************************************************************/
BYTE App_HID2ASCII(BYTE a) //convert USB HID code (buffer[2 to 7]) to ASCII code
{
   BYTE AsciiVal;
   BYTE ShiftkeyStatus = 0;
   if((Appl_BufferModifierKeys[MODIFIER_LEFT_SHIFT] == 1)||(Appl_BufferModifierKeys[MODIFIER_RIGHT_SHIFT] == 1))
    {
        ShiftkeyStatus = 1;
    }

   if(a>=0x1E && a<=0x27)
    {
       if(ShiftkeyStatus)
        {
            switch(a)
            {
                case Symbol_Exclamation: AsciiVal = 0x21;
                    break;
                case Symbol_AT: AsciiVal = 0x40;
                    break;
                case Symbol_Pound: AsciiVal = 0x23;
                    break;
                case Symbol_Dollar: AsciiVal = 0x24;
                    break;
                case Symbol_Percentage: AsciiVal = 0x25;
                    break;
                case Symbol_Cap: AsciiVal = 0x5E;
                    break;
                case Symbol_AND: AsciiVal = 0x26;
                    break;
                case Symbol_Star: AsciiVal = 0x2A;
                    break;
                case Symbol_NormalBracketOpen: AsciiVal = 0x28;
                    break;
                case Symbol_NormalBracketClose: AsciiVal = 0x29;
                    break;
                default:
                    break;
            }

            return(AsciiVal);
        }
       else
       {
            if(a==0x27)
            {
                return(0x30);
            }
            else
            {
                return(a+0x13);
            }
       }
    }

   if((a>=0x59 && a<=0x61)&&(NUM_Lock_Pressed == 1))
    {
       return(a-0x28);
    }

   if((a==0x62) &&(NUM_Lock_Pressed == 1)) return(0x30);


   if(a>=0x04 && a<=0x1D)
    {
      if(((CAPS_Lock_Pressed == 1)&&((Appl_BufferModifierKeys[MODIFIER_LEFT_SHIFT] == 0)&&
                                (Appl_BufferModifierKeys[MODIFIER_RIGHT_SHIFT] == 0)))
          ||((CAPS_Lock_Pressed == 0)&&((Appl_BufferModifierKeys[MODIFIER_LEFT_SHIFT] == 1) ||
                                        (Appl_BufferModifierKeys[MODIFIER_RIGHT_SHIFT] == 1))))
           return(a+0x3d); /* return capital */
      else
           return(a+0x5d); /* return small case */
    }


   if(a>=0x2D && a<=0x38)
    {
        switch(a)
        {
            case Symbol_HyphenUnderscore:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2D;
                else
                    AsciiVal = 0x5F;
                break;
            case Symbol_EqualAdd:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x3D;
                else
                    AsciiVal = 0x2B;
                break;
            case Symbol_BracketOpen:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x5B;
                else
                    AsciiVal = 0x7B;
                break;
            case Symbol_BracketClose:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x5D;
                else
                    AsciiVal = 0x7D;
                break;
            case Symbol_BackslashOR:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x5C;
                else
                    AsciiVal = 0x7C;
                break;
            case Symbol_SemiColon:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x3B;
                else
                    AsciiVal = 0x3A;
                break;
            case Symbol_InvertedComma:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x27;
                else
                    AsciiVal = 0x22;
                break;
            case Symbol_Tilde:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x60;
                else
                    AsciiVal = 0x7E;
                break;
            case Symbol_CommaLessThan:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2C;
                else
                    AsciiVal = 0x3C;
                break;
            case Symbol_PeriodGreaterThan:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2E;
                else
                    AsciiVal = 0x3E;
                break;
            case Symbol_FrontSlashQuestion:
                if(!ShiftkeyStatus)
                    AsciiVal = 0x2F;
                else
                    AsciiVal = 0x3F;
                break;
            default:
                break;
        }
        return(AsciiVal);
    }

   return(0);
}


/****************************************************************************
  Function:
    void LCD_Display_Routine(BYTE data , BYTE HIDData)
  Description:
    This function displays the key strokes on the LCD  mounted on Explorer16
    demo board.

  Precondition:
    None

  Parameters:
    BYTE data       -   ASCII code for the key pressed
    BYTE HIDData    -   HID code for the key pressed, this is needed to take
                        action for keys like Esc, Enter, Tab etc.

  Return Values:
    None

  Remarks:
***************************************************************************/

void LCD_Display_Routine(BYTE data , BYTE HIDData)
{
    LATBbits.LATB1 = 0;
    LATBbits.LATB0 = 1;
    if((HIDData>=0x1E && HIDData<=0x27) || (HIDData>=0x04 && HIDData<=0x1D) ||
            (HIDData>=0x2D && HIDData<=0x38) || ((HIDData>=0x59 && HIDData<=0x62)&&(NUM_Lock_Pressed == 1)))
    {
        WriteData_LCD(data);
        currCharPos++;
    }
    else if(HIDData == Symbol_Escape)                                // escape key pressed
    {
        ClearScreen_LCD();
        MoveCursorToPosition(FirstLine);
        LineNo = 1;
        currCharPos = 0;
    }
    else if (HIDData == Symbol_Space)                               // space pressed
    {
        WriteData_LCD(' ');                                // space ascii value 0x20
        currCharPos++;
    }
    else if (HIDData == Symbol_Return)
    {
        currCharPos = 20;
        ChangeLine();
    }
    else if (HIDData == Symbol_Backspace)                   // back space pressed
    {
        if(currCharPos != 0)
        {
            currCharPos--;
            if (LineNo == 1)
                MoveCursorToPosition(FirstLine + currCharPos);
            else if (LineNo == 2)
                MoveCursorToPosition(SecondLine + currCharPos);
           else if (LineNo == 3)
                MoveCursorToPosition(ThirdLine + currCharPos);
            else if (LineNo == 4)
                MoveCursorToPosition(FourthLine + currCharPos);
            WriteData_LCD(' ');                                // clearing that character

            if (LineNo == 1)
                MoveCursorToPosition(FirstLine + currCharPos);
            else if (LineNo == 2)
                MoveCursorToPosition(SecondLine + currCharPos);
            else if (LineNo == 3)
                MoveCursorToPosition(ThirdLine + currCharPos);
            else if (LineNo == 4)
                MoveCursorToPosition(FourthLine + currCharPos);
        }
    }

    ChangeLine();

}

/****************************************************************************
  Function:
    BOOL USB_HID_DataCollectionHandler(void)
  Description:
    This function is invoked by HID client , purpose is to collect the
    details extracted from the report descriptor. HID client will store
    information extracted from the report descriptor in data structures.
    Application needs to create object for each report type it needs to
    extract.
    For ex: HID_DATA_DETAILS Appl_ModifierKeysDetails;
    HID_DATA_DETAILS is defined in file usb_host_hid_appl_interface.h
    Each member of the structure must be initialized inside this function.
    Application interface layer provides functions :
    USBHostHID_ApiFindBit()
    USBHostHID_ApiFindValue()
    These functions can be used to fill in the details as shown in the demo
    code.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    TRUE    - If the report details are collected successfully.
    FALSE   - If the application does not find the the supported format.

  Remarks:
    This Function name should be entered in the USB configuration tool
    in the field "Parsed Data Collection handler".
    If the application does not define this function , then HID cient
    assumes that Application is aware of report format of the attached
    device.
***************************************************************************/
BOOL USB_HID_DataCollectionHandler(void)
{
  BYTE NumOfReportItem = 0;
  BYTE i;
  USB_HID_ITEM_LIST* pitemListPtrs;
  USB_HID_DEVICE_RPT_INFO* pDeviceRptinfo;
  HID_REPORTITEM *reportItem;
  HID_USAGEITEM *hidUsageItem;
  BYTE usageIndex;
  BYTE reportIndex;
  BOOL foundLEDIndicator = FALSE;
  BOOL foundModifierKey = FALSE;
  BOOL foundNormalKey = FALSE;



  pDeviceRptinfo = USBHostHID_GetCurrentReportInfo(); // Get current Report Info pointer
  pitemListPtrs = USBHostHID_GetItemListPointers();   // Get pointer to list of item pointers

  BOOL status = FALSE;
   /* Find Report Item Index for Modifier Keys */
   /* Once report Item is located , extract information from data structures provided by the parser */
   NumOfReportItem = pDeviceRptinfo->reportItems;
   for(i=0;i<NumOfReportItem;i++)
    {
       reportItem = &pitemListPtrs->reportItemList[i];
       if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Variable)&&
           (reportItem->globals.usagePage==USAGE_PAGE_KEY_CODES))
        {
           /* We now know report item points to modifier keys */
           /* Now make sure usage Min & Max are as per application */
            usageIndex = reportItem->firstUsageItem;
            hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];
            if((hidUsageItem->usageMinimum == USAGE_MIN_MODIFIER_KEY)
                &&(hidUsageItem->usageMaximum == USAGE_MAX_MODIFIER_KEY)) //else application cannot suuport
            {
               reportIndex = reportItem->globals.reportIndex;
               Appl_ModifierKeysDetails.reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
               Appl_ModifierKeysDetails.reportID = (BYTE)reportItem->globals.reportID;
               Appl_ModifierKeysDetails.bitOffset = (BYTE)reportItem->startBit;
               Appl_ModifierKeysDetails.bitLength = (BYTE)reportItem->globals.reportsize;
               Appl_ModifierKeysDetails.count=(BYTE)reportItem->globals.reportCount;
               Appl_ModifierKeysDetails.interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
               foundModifierKey = TRUE;
            }

        }
        else if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Array)&&
           (reportItem->globals.usagePage==USAGE_PAGE_KEY_CODES))
        {
           /* We now know report item points to modifier keys */
           /* Now make sure usage Min & Max are as per application */
            usageIndex = reportItem->firstUsageItem;
            hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];
            if((hidUsageItem->usageMinimum == USAGE_MIN_NORMAL_KEY)
                &&(hidUsageItem->usageMaximum <= USAGE_MAX_NORMAL_KEY)) //else application cannot suuport
            {
               reportIndex = reportItem->globals.reportIndex;
               Appl_NormalKeysDetails.reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
               Appl_NormalKeysDetails.reportID = (BYTE)reportItem->globals.reportID;
               Appl_NormalKeysDetails.bitOffset = (BYTE)reportItem->startBit;
               Appl_NormalKeysDetails.bitLength = (BYTE)reportItem->globals.reportsize;
               Appl_NormalKeysDetails.count=(BYTE)reportItem->globals.reportCount;
               Appl_NormalKeysDetails.interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
               foundNormalKey = TRUE;
            }

        }
        else if((reportItem->reportType==hidReportOutput) &&
                (reportItem->globals.usagePage==USAGE_PAGE_LEDS))
        {
            usageIndex = reportItem->firstUsageItem;
            hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

            reportIndex = reportItem->globals.reportIndex;
            Appl_LED_Indicator.reportLength = (pitemListPtrs->reportList[reportIndex].outputBits + 7)/8;
            Appl_LED_Indicator.reportID = (BYTE)reportItem->globals.reportID;
            Appl_LED_Indicator.bitOffset = (BYTE)reportItem->startBit;
            Appl_LED_Indicator.bitLength = (BYTE)reportItem->globals.reportsize;
            Appl_LED_Indicator.count=(BYTE)reportItem->globals.reportCount;
            Appl_LED_Indicator.interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
            foundLEDIndicator = TRUE;
        }

    }


   if(pDeviceRptinfo->reports == 1)
    {
        Appl_raw_report_buffer.Report_ID = 0;
        Appl_raw_report_buffer.ReportSize = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
        Appl_raw_report_buffer.ReportData = (BYTE*)malloc(Appl_raw_report_buffer.ReportSize);
        Appl_raw_report_buffer.ReportPollRate = pDeviceRptinfo->reportPollingRate;
        if((foundNormalKey == TRUE)&&(foundModifierKey == TRUE))
        status = TRUE;
    }

    return(status);
}

void ChangeLine(void)
{
    if(currCharPos == 20)
    {
       if (LineNo == 1)
       {
           MoveCursorToPosition(SecondLine);
           LineNo = 2;
       }
       else if (LineNo == 2)
       {
           MoveCursorToPosition(ThirdLine);
           LineNo = 3;
       }
       else if (LineNo == 3)
       {
           MoveCursorToPosition(FourthLine);
           LineNo = 4;
       }
       else
       {
           ClearScreen_LCD();
           LineNo = 1;
       }
       currCharPos = 0;
    }
}

