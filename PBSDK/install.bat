copy /b /y bin\cygpb1.dll %WINDIR%\System32\cygpb1.dll
reg delete "HKLM\SOFTWARE\Cygnus Solutions\CygPB" /f

reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2" /v "cygdrive flags" /t REG_DWORD /d 34
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2" /v "cygdrive prefix" /d "/cygdrive"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/" /v "flags" /t REG_DWORD /d 10
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/" /v "native" /d "%CD%"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/usr/bin"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/usr/bin" /v "flags" /t REG_DWORD /d 10
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/usr/bin" /v "native" /d "%CD%\bin"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/usr/lib"
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/usr/lib" /v "flags" /t REG_DWORD /d 10
reg add "HKLM\SOFTWARE\Cygnus Solutions\CygPB\mounts v2\/usr/lib" /v "native" /d "%CD%\lib"

reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v POCKETBOOKSDK /f
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v POCKETBOOKSDK /d "%CD%"
