#!/bin/bash

adb shell /data/local/tmp/benchmark_model --graph=/data/local/tmp/mobilenet_v1_1.0_224.tflite --num_threads=4 --use_hexagon=true --warmup_runs=1 --num_runs=50
