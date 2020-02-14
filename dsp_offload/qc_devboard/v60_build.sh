set -e
cp -u vgg.c vgg_data.c vgg_data_32b.c vgg.h vgg_data_float.c ${HEXAGON_NN}  
make -C $HEXAGON_NN tree VERBOSE=1 V=hexagon_Release_dynamic_toolv83_v60
make -C $HEXAGON_NN tree VERBOSE=1 V=android_Release GRAPHINIT="vgg.c vgg_data.c vgg_data_32b.c vgg_data_float.c"
