# Building repo
Run the following from this directory (i.e., `reasoned-soc/android/cpp`):
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

# Running `atop`

1. Connect your device (note: typically you want to run `adb root` and `adb remount -R` prior to connecting but the application should tell you what's wrong if you didn't)
2. From this directory run `./build/bin/atop --verbose`. If you run it inside a directory without a `imgui.ini` the application window layout may break and you will have to repostition them manually. The `imgui.ini` file provided in this repo is based on a 15" Macbook Pro so it might look different on your machine. Below is a reference picture to what it should look like. Once you reposition the windows to your desire the application will save the layout for the next run.
