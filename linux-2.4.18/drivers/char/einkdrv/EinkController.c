#include "EPDConstants.h"

#define true 1
#define false 0
#define bool unsigned int


bool SilentMode;

//---------------------------------------------------------------------------
#if 0

void TEInkControl(int APortAddress):TObject()
{
   TDateTime SaveTime;
   PortAddress = APortAddress;
   HardwareOK  = LoadLibrary();
   WakeUpTimer = new TTimer(NULL);
   WakeUpTimer->Enabled=false;
   WakeUpTimer->Interval = 1000;
   WakeUpTimer->OnTimer = ResetWakeUpTimer;
   WakeUpFinished = True;
   if (HardwareOK)
   {
      TimeOutValue = 0;
      SaveTime = Now();
      while (MilliSecondsBetween (Now(), SaveTime) < 1000)
      {
          Acknowledge();
          TimeOutValue++;
      }
   }

  if (HardwareOK)
  {
      TimeOutValue = (TimeOutValue * 5) ;//{ 5 seconds }
  }else
  {
    TimeOutValue = 2000000;
  }
  Reset();
}

#endif

#if 0

void Reset()
{
  Status        = 0x00;
  Position      = 0x00;
  PositiveMode  = true;
  SetBit   (ei_DS);
  ClearBit (ei_RW);
  SetBit   (ei_OE);
  ClearBit (ei_CD);
  ClearBit (ei_WUP);
}

#endif


#if 0

void ResetWakeUpTimer(TObject *Sender)
{
  WakeUpTimer->Enabled = false;
  WakeUpFinished      = true;
}

void WakeUp()
{
  ClearBit (ei_DS);
  SetBit (ei_WUP);

  WakeUpFinished      = false;
  WakeUpTimer->Enabled = true;

  while (!WakeUpFinished)
  {
    Application->ProcessMessages();
  }
  ClearBit (ei_WUP);
  SetBit (ei_DS);
}

#endif




bool SendCommand (TDisplayCommand *dCommand)
{
   bool Check;
   int ByteCounter;
 // CommandInProgress = true;
   
   if (dCommand->Owner != 0)
   {
      //Screen->Cursor = crHourGlass;
      //SendMessage (dCommand->Owner, wm_CommandUpdate, 0, 0);
   }
   Check = true;
   
   if (dCommand->Command > 0)
      Check = WriteCommand(dCommand->Command);


#if 0
   ByteCounter = 0;
   while ((Check) && (ByteCounter < dCommand->BytesToWrite))
   {
      
      Check = WriteData (dCommand->Data [ByteCounter]);

//      Check = WriteData (0xaa);
      ByteCounter++;
      
   }
#else
	if(dCommand->BytesToWrite)
		WriteDataBuf(dCommand->Data,dCommand->BytesToWrite);

#endif   
   
   
   ByteCounter = 0;

   while ((Check) &&(ByteCounter < dCommand->BytesToRead))
   {
      Check = ReadData(&(dCommand->Data [ByteCounter]));
      ByteCounter++;
      
   }

//   CommandInProgress = false;
   if (dCommand->Owner != 0)
   {
      //Screen->Cursor = crDefault;
      //SendMessage (dCommand->Owner, wm_CommandUpdate, 0, 0);
   }
   if ((!SilentMode) && (!Check))
      //MessageDlg("Display controller is not responding.", mtError, TMsgDlgButtons() << mbOK , 0);
      printk("Display controller is not responding.");
      
   return  Check;
}
