#!/bin/bash

adb shell dmesg | ag 'DEBUG|IOCTL|TIME' > fastrpc_processed.txt
