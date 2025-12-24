#!/bin/bash

toolchain_path="/home/stu/T113-S3/tool/toolchain-sunxi-glibc-gcc-830/toolchain/bin"

function cue() {
    echo
    echo "Usage:"
    echo "  ./build.sh -linux"
    echo "  ./build.sh -t113 "
    echo "  ./build.sh -clean  "
    echo
}

while test $# -gt 0
do
  case "$1" in
  -linux)
    platform="linux"
    ;; 
  -t113)
    platform="t113"
    ;; 
  -clean)
    rm ./build/ -rf
    echo "clean project success"
    exit 0
    ;;
  -h)
    cue
    exit 0
    ;;
  *)
  esac
  shift
done

if [ ! -d ./build ]; then
    mkdir build
fi

if [ -z ${platform} ]; 
then
    cue
    exit 0
fi

if [ ${platform} == "linux" ];
then
    echo "build linux app"
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=platform/x86linux/linux.cmake -DSIMULATOR_LINUX=${platform}
    make -j16
    exit 0
fi

if [ ${platform} == "t113" ];
then
    export STAGING_DIR="${toolchain_path}":$STAGING_DIR
    echo "build t113 app"
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=platform/t113/t113.cmake -DTOOLCHAIN_PATH="${toolchain_path}"
    make -j16
    exit 0
fi