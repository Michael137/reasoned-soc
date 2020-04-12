#!/bin/bash

FILE=$1
OUT_FILE=$(echo "${FILE}" | cut -f 1 -d '.').out
OS=darwin
NDK_ROOT=$HOME/Library/Android/sdk/ndk/21.0.6113669
LLVM_TOOLCHAIN="$NDK_ROOT/toolchains/llvm/prebuilt/${OS}-x86_64"
INSTALL_DIR=/data/local/tmp

export LD_LIBRARY_PATH=$LLVM_TOOLCHAIN/lib64:$LLVM_TOOLCHAIN/lib:$LD_LIBRARY_PATH

$LLVM_TOOLCHAIN/bin/aarch64-linux-android21-clang --target=aarch64-none-linux-android21 \
												  --gcc-toolchain=$LLVM_TOOLCHAIN		\
												  --sysroot=$LLVM_TOOLCHAIN/sysroot $FILE -o $OUT_FILE

adb push $OUT_FILE $INSTALL_DIR
adb shell "${INSTALL_DIR}/${OUT_FILE}"
