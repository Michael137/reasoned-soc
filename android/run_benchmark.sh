#!/bin/bash

adb shell /data/local/tmp/benchmark_model   --graph=/data/local/tmp/inception_v4_299_quant.tflite   --num_threads=4 --use_hexagon=true   --warmup_runs=1 --num_runs=50
