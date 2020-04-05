# SNPE Installation and Benchmarking
0. Make sure your gcc is **not** one of the latest versions (gcc7 is sufficient (default on Ubuntu)) since it can cause libc version complications when installing from source later
1. Install the **Python 2.7** pre-requisites for SNPE.
  - Make sure you stick to python 2.7 since 3.6 is not supported and versions between 2.7 and 3.6 are tricky to set up right
2. Install the Android NDK (via the Hexagon SDK installation)
  - Add following to your bashrc
`export ANDROID_NDK_ROOT="$HOME/Qualcomm/Hexagon_SDK/3.5.1/tools/android-ndk-r19c"`
3. Install the **latest version** of "scikit-image" and other pre-requisites (check with bin/check_python_depends.sh and bin/dependencies.sh in snpe folder). The latter is important to have on the latest version because pre-v0.14 contain a numpy breakage
4. Install protobuf from source
5. Check whether `libatomic.so.1` is in `$SNPE_ROOT/lib/aarch64-linux-gcc4.9/` already
  - Otherwise install using `apt` or equivalent
6. Install Caffe (see Tensorflow installation in section below; this can be used to run the Inception_v3 network):
  - gflags: `sudo apt-get install libgflags2 libgflags-dev`
  - BLAS: `sudo apt-get install libatlas-base-dev`
  - hdf5: `sudo apt-get install libhdf5-serial-dev`
  - [Install glog](https://github.com/google/glog/wiki/Installing-Glog-on-Ubuntu-14.04)
  - `git clone https://github.com/BVLC/caffe.git ~/caffe; cd ~/caffe; git reset --hard 18b09e807a6e146750d84e89a961ba8e678830b4`
  - `mkdir build; cd build; cmake ..`
  - `make all -j4`
  - `make test`
  - `make runtest`
  - `make install`
  - `make pycaffe`
  - Install Caffe2 using your distribution's package manager
7. Add following to your bashrc:
```bash
export PYTHONPATH=$HOME/Qualcomm/snpe-1.35.0.698/lib/python:$PYTHONPATH
export PYTHONPATH=$HOME/caffe/build/install/python:$PYTHONPATH
export LD_LIBRARY_PATH=$HOME/caffe/build/install/lib:$LD_LIBRARY_PATH
export PATH=$HOME/caffe/build/install/bin:$PATH
export SNPE_ROOT=$HOME/Qualcomm/snpe-1.35.0.698
cd $HOME/Qualcomm/snpe-1.35.0.698/bin; source envsetup.sh -c ~/caffe > /dev/null; cd ~
```
8. Make sure the **libasdprpc.so** is in `/system/vendor/lib/rfsa/adsp` on your device. Otherwise the benchmarks will not work using the DSP runtime
  - See [this Qualcomm Forum Thread](https://developer.qualcomm.com/forum/qdn-forums/software/hexagon-dsp-sdk/toolsinstallation/34446) for more information
  - If the shared object is missing copy it from the Hexagon SDK like so: `adb push $HOME/Qualcomm/Hexagon_SDK/3.5.1/libs/common/remote/ship/android_Release_aarch64/libadsprpc.so /system/vendor/lib/rfsa/adsp/libadsprpc.so`
9. Try AlexNet example
  - Copy the `/home/gardei/Qualcomm/snpe-1.35.0.698/bin/x86_64-linux-clang/snpe-caffe-to-dlc` to the AlexNet scripts directory or put it on the path
  - Run from `$HOME/Qualcomm/snpe-1.35.0.698/models/alexnet/scripts`: `python setup_alexnet.py -d -a ./temp-assets-cache`
  - Copy over the shared libraries to your device: `adb push $SNPE_ROOT/lib/dsp/lib* /dsp/snpe`
  - Benchmarks on device; run following from host:
    - `cd $SNPE_ROOT/benchmarks`
    - Configure the `alexnet_sample.json` file to include DSP measurements and increase number of runs
    - `python snpe_bench.py -c alexnet_sample.json -a --profilinglevel detailed`
    - Export results `cd $SNPE_ROOT/benchmarks/alexnet/results/latest_results`
  - **NOTE: add a `-d` flag to the `snpe_bench.py` invocation to debug any errors**

# Installation from Virtual Environment
1. Create a virtual environment from your `$SNPE_ROOT` folder
2. Activate your virtual environment
3. Copy the [requirements.txt](requirements.txt) file to `$SNPE_ROOT`
4. Run: `pip install -r requirements.txt`

# Tensorflow
- SNPE uses v1.6 (should be already included in the requirements.txt)
- v1.6 can be installed by running: `python -m pip install tensorflow==1.6`
## SNPE Tensorflow setup
1.
```bash
export TENSORFLOW_DIR=`python -m pip show tensorflow | grep "Location" | awk '{print $2}'`/tensorflow
```
2. From `$SNPE_ROOT` run:
```bash
cd $SNPE_ROOT/bin; source envsetup.sh -t $TENSORFLOW_DIR > /dev/null; cd -
```
3. Get the model:
- (with CPU runtime)
```bash
python $SNPE_ROOT/models/inception_v3/scripts/setup_inceptionv3.py -a ~/tmpdir -d -r cpu
```
- (with DSP runtime)
```bash
python $SNPE_ROOT/models/inception_v3/scripts/setup_inceptionv3.py -a ~/tmpdir -d -r dsp
```
- (with AIP runtime)
```bash
python $SNPE_ROOT/models/inception_v3/scripts/setup_inceptionv3.py -a ~/tmpdir -d -r aip
```
4. Follow example instructions from docs to run on host or Android target
- The `--use_aip`/`--use_dsp` flags to `snpe-net-run` can be used to select the AIP or DSP runtimes specifically

## Inception_v3 Benchmarking
1. Copy [inception_v3_sample.json](snpe/benchmarks/inception_v3_sample.json) to your `$SNPE/benchmarks` directory
2. Run the benchmark script

## MobileNetSSD
- This model has been tested on Tensorflow 1.11 and will throw errors with tensorflow modules above 1.14.
  - `python -m pip install tensorflow=1.11`
- Follow SNPE installation instructions for object detection model
- Push the mobilenet_ssd.dlc, imagelist.txt and images to the device
- Use the `create_file_list.py` and `create_inceptionv3_raws.py` scripts to resize the alexnet images to the appropriate input dimensions and corresponding file list
- Run the benchmark suite using [mobilenetssd.json](snpe/benchmarks/mobilenetssd.json)
  - Make sure the `imagelist.txt` contains the path and filenames for the **.raw** files on the device

## DeepLabv3
- Activate the virtual environment containing the Tensorflow and SNPE pre-requisuite installations
- Follow the steps described in SNPE documentation
- Preprocessing the input can be done by invoking [preprocess.py](snpe/benchmarks/preprocess.py) with a set of test images:
  - E.g., `python preprocess.py ~/Qualcomm/snpe-1.35.0.698/models/alexnet/data`
  - Note: the images will be resized to 512x512 as expected per the SNPE documentation for DeepLabv3
  - Note: inspired by [this Qualcomm project](https://developer.qualcomm.com/project/image-segmentation-using-deeplabv3)
- Create an `imagelist.txt`
- Run the benchmark suite using [deeplabv3.json](snpe/benchmarks/deeplabv3.json)
  - Make sure the `imagelist.txt` contains the path and filenames for the **.raw** files on the device

# MLPerf Benchmark Validation
## From result sources
Here we aim to reproduce the results of the MLPerf closed [inference results](https://github.com/mlperf/inference_results_v0.5/tree/master/closed/Qualcomm)

1. Get the [Qualcomm source code](https://github.com/mlperf/inference_results_v0.5/blob/master/closed/Qualcomm/code/mobilenet/reference/VERSION.txt)
2. Check the SNPE version used from the [systems directory](https://github.com/mlperf/inference_results_v0.5/blob/master/closed/Qualcomm/systems/SDM855.json)
3. Download the appropriate SNPE SDK version from [Qualcomm](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk/tools)
  - Follow the above SNPE SDK installation and setup instructions
4. Download the appropriate dataset and convert the images to raw format and the appropriate size
  - One can use `$SNPE_ROOT/models/inception_v3/scripts/create_inceptionv3_raws.py` for this
5. Create an `imagelist.txt` that contains the path to the **.raw** files
6. Follow the model conversion instructions from the MLPerf submission of step 1
7. Run model (i.e., the **.dlc**) file through `$SNPE_ROOT/benchmarks/snpe_bench.py`
  - An example benchmark JSON configuration is in [mobilenet_v1.json](snpe/benchmarks/mobilenet_v1.json)

## App
