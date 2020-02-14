#!/bin/bash

set -e
adb push ${HEXAGON_NN}/android_Release/ship/graph_app /data
adb push ${HEXAGON_NN}/hexagon_Release_dynamic_toolv83_v65/ship/libhexagon_nn_skel.so /vendor/lib/rfsa/adsp
adb push ${HEXAGON_NN}/test/vgg_labels.txt /vendor/etc/

for filename in /home/gardei/vgg16_vgg_b/*
do
	echo "${filename}"
	name=${filename##*/}
	adb push "${filename}" /vendor/etc/
	adb shell /data/graph_app --iters 1 "/vendor/etc/${name}" --labels_filename "/vendor/etc/vgg_labels.txt" | grep -E 'filesize|RpcMsecs'
done
