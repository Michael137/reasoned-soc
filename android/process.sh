#!/bin/bash

adb shell dmesg | ag 'IOCTL|TIME' > fastrpc_processed.txt
