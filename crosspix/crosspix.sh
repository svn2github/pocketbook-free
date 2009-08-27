#!/bin/sh

appname="crosspix.exe.so"
# determine the application directory
appdir=''
case "$0" in
  */*)
    # $0 contains a path, use it
    appdir=`dirname "$0"`
    ;;
  *)
    # no directory in $0, search in PATH
    saved_ifs=$IFS
    IFS=:
    for d in $PATH
    do
      IFS=$saved_ifs
      if [ -x "$d/$appname" ]; then appdir="$d"; break; fi
    done
    ;;
esac

# figure out the full app path
if [ -n "$appdir" ]; then
    apppath="$appdir/$appname"
    WINEDLLPATH="$appdir:$WINEDLLPATH"
    export WINEDLLPATH
else
    apppath="$appname"
fi

# determine the WINELOADER
if [ ! -x "$WINELOADER" ]; then WINELOADER="wine"; fi

# and try to start the app
exec "$WINELOADER" "$apppath" "$@"
