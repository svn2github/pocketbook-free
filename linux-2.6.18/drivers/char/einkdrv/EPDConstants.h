//---------------------------------------------------------------------------

#ifndef EPDConstantsH
#define EPDConstantsH


#define BYTE unsigned char
#define HWND unsigned int

//const  wm_CommandUpdate  = WM_USER;
#define  dc_NewImage          0xA0
#define  dc_StopNewImage      0xA1
#define  dc_DisplayImage      0xA2
#define  dc_PartialImage      0xB0
#define  dc_DisplayPartial    0xB1
#define  dc_Reset             0xEE
#define  dc_SetDepth          0xF3
#define  dc_EraseDisplay      0xA3
#define  dc_Rotate            0xF5
#define  dc_Positive          0xF7
#define  dc_Negative          0xF8
#define  dc_GoToNormal        0xF0
#define  dc_GoToSleep         0xF1
#define  dc_GoToStandBy       0xF2
#define  dc_WriteToFlash      0x01
#define  dc_ReadFromFlash     0x02
#define  dc_Init              0xA4
#define  dc_AutoRefreshOn     0xF9
#define  dc_AutoRefreshOff    0xFA
#define  dc_SetRefresh        0xFB
#define  dc_ForcedRefresh     0xFC
#define  dc_GetRefresh        0xFD
#define  dc_RestoreImage      0xA5
#define  dc_ControllerVersion 0xE0
#define  dc_SoftwareVersion   0xE1
#define  dc_DisplaySize       0xE2
#define  dc_GetStatus         0xAA
#define  dc_Temperature       0x21
#define  dc_WriteRegister     0x10
#define  dc_ReadRegister      0x11
#define  dc_Abort             0xA1

#define MaxBufferSize        (((800 * 600) / 2) + 50)
#define  EEPROMSize           0x400000 //{4Mb}

typedef BYTE TEEPROMData[EEPROMSize];


typedef struct
{
   HWND Owner;
   BYTE Command;
   int BytesToWrite ;
   int BytesToRead;
   BYTE Data[MaxBufferSize];
}TDisplayCommand;


int WriteDataBuf(BYTE *Data,int len);
#endif