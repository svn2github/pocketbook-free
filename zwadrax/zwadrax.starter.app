#!/bin/sh
APPNAME=`basename $0 .starter.app`
DIRNAME=`dirname $0`
exec $DIRNAME/$APPNAME.app "$@" 1>$DIRNAME/$APPNAME.stdout.txt 2>$DIRNAME/$APPNAME.stderr.txt
