#!/bin/bash

OS=$(uname)

if [ "${OS}" == "Darwin" ]; then
	./compile_and_run_macos.sh $1
else
	./compile_and_run_linux.sh $1
fi
