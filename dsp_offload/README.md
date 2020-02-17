# SNPE Installation and Benchmarking
0. Make sure your gcc is **not** one of the latest versions (gcc7 is sufficient (default on Ubuntu)) since it can cause libc version complications when installing from source later
1. Install the **Python 2.7** pre-requisites for SNPE.
  - Make sure you stick to python 2.7 since 3.6 is not supported and versions between 2.7 and 3.6 are tricky to set up right
2. Install Caffe instead of Tensorflow since the latter since SNPE is not compatible with the latest TF versions
3. Install the Android NDK (via the Hexagon SDK installation)
  - Add following to your bashrc
`export ANDROID_NDK_ROOT="$HOME/Qualcomm/Hexagon_SDK/3.5.1/tools/android-ndk-r19c"`
4. Install the latest version of "skimage" and "scikit-image". The latter is important to have on the latest version because pre-v0.14 contain a numpy breakage
5. Install protobuf from source
6. Check whether `libatomic.so.1` is in `$SNPE_ROOT/lib/aarch64-linux-gcc4.9/ already`
  - Otherwise install using `apt` or equivalent
7. Install Caffe:
  - `git clone https://github.com/BVLC/caffe.git ~/caffe; cd ~/caffe; git reset --hard 18b09e807a6e146750d84e89a961ba8e678830b4`
  - `mkdir build; cd build; cmake ..`
  - `make all -j4`
  - `make test`
  - `make runtest`
  - `make install`
  - `make pycaffe`
  - Install Caffe2 using your distribution's package manager
8. Add following to your bashrc:
`export PYTHONPATH=$HOME/Qualcomm/snpe-1.35.0.698/lib/python:$PYTHONPATH
export PYTHONPATH=$HOME/caffe/build/install/python:$PYTHONPATH
export LD_LIBRARY_PATH=$HOME/caffe/build/install/lib:$LD_LIBRARY_PATH
export PATH=$HOME/caffe/build/install/bin:$PATH
export SNPE_ROOT=$HOME/Qualcomm/snpe-1.35.0.698

cd $HOME/Qualcomm/snpe-1.35.0.698/bin; source envsetup.sh -c ~/caffe > /dev/null; cd ~`
9. Try AlexNet example
  - Run from `$HOME/Qualcomm/snpe-1.35.0.698/models/alexnet/scripts`: `python setup_alexnet.py -d -a ./temp-assets-cache`
