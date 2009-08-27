#!/bin/sh
ff() {
  [ -d "$1" ] || return 1
  for i in "$1"/*; do
    [ -f "$i" ] && echo "$i"
    [ -d "$i" ] && ff "$i"
  done
}

fd() {
  [ -d "$1" ] || return 1
  for i in "$1"/*; do
    if [ -d "$i" ]; then
      fd "$i"
      echo "$i"
    fi
  done
}

ff /mnt/ext1/system/state | while read i; do
  f="${i#/mnt/ext1/system/state/}"
  f="${f%.af?}"
  [ -f /mnt/ext1/"$f" ] ||
    [ -f /mnt/ext2/"$f" ] ||
      rm -f "$i"
done
fd /mnt/ext1/system/state | while read i; do
  [ -z "`ls "$i"`" ] && rmdir "$i"
done
