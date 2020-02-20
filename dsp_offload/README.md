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

8. Try AlexNet example
  - Copy the `/home/gardei/Qualcomm/snpe-1.35.0.698/bin/x86_64-linux-clang/snpe-caffe-to-dlc` to the AlexNet scripts directory or put it on the path
  - Run from `$HOME/Qualcomm/snpe-1.35.0.698/models/alexnet/scripts`: `python setup_alexnet.py -d -a ./temp-assets-cache`
  - Copy over the shared libraries to your device: `adb push $SNPE_ROOT/lib/dsp/lib* /dsp/snpe`
  - Benchmarks on device; run following from host:
    - `cd $SNPE_ROOT/benchmarks`
    - Configure the `alexnet_sample.json` file to include DSP measurements and increase number of runs
    - `python snpe_bench.py -c alexnet_sample.json -a --profilinglevel detailed`
    - Export results `cd $SNPE_ROOT/benchmarks/alexnet/results/latest_results`

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

## Benchmarking Inception_v3

