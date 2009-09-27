//iv_timerproc:
// typedef void (*iv_timerproc)();
// void SetHardTimer(char *name, iv_timerproc tproc, int ms);
// void SetWeakTimer(char *name, iv_timerproc tproc, int ms);
//???: How to wrap this?
// void ClearTimer(iv_timerproc tproc);

callbackTypemapIn(TimerProc, iv_timerproc);

void SetHardTimer(char *name, iv_timerproc tproc, int ms);
void SetWeakTimer(char *name, iv_timerproc tproc, int ms);
void ClearTimer(iv_timerproc tproc);
%ignore ClearTimer;

%clear iv_timerproc;
