#!/bin/sh

no_turbo=/sys/devices/system/cpu/intel_pstate/no_turbo
boost=/sys/devices/system/cpu/cpufreq/boost

get_permission() {
  group=$(groups | tr ' ' '\n' | grep -i benchmark)
  if [ -z "$group" ]; then
    group=$(groups | cut -d' ' -f1)
  fi
  tofix=""
  for i in $no_turbo $boost /sys/devices/system/cpu/cpufreq/policy[0-9]*/scaling_governor; do
    test -e $i || continue
    test -w $i && continue
    tofix="$tofix $i"
  done
  if [ -n "$tofix" ]; then
    sudo sh -c "chgrp $group $tofix; chmod g+rw $tofix;"
  fi
}

turn_on() {
  get_permission
  for i in /sys/devices/system/cpu/cpufreq/policy[0-9]*/scaling_governor; do
    echo performance > $i
  done
  if test -f $no_turbo; then
    echo 1 > $no_turbo
  elif test -f $boost; then
    echo 0 > $boost
  else
    echo "failed to disable turbo/boost"
  fi
}

turn_off() {
  get_permission
  governor=schedutil
  if test -f $no_turbo; then
    echo 0 > $no_turbo
    governor=powersave
  elif test -f $boost; then
    echo 1 > $boost
  else
    echo "failed to enable turbo/boost"
  fi
  for i in /sys/devices/system/cpu/cpufreq/policy[0-9]*/scaling_governor; do
    echo $governor > $i
  done
}


case "$1" in
  -h|--help)
    echo "Usage: $0 {on|off|-i}"
    ;;
  on|an|start)  turn_on ;;
  off|aus|stop) turn_off ;;
  -i|--interactive|*)
    while true; do
      echo -n "Press Enter to turn benchmark modus on: "
      read tmp
      turn_on
      echo -n "Press Enter to turn benchmark modus off:"
      read tmp
      turn_off
    done
    ;;
esac
