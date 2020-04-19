scp -i ~/.ssh/chips mbuch@acc40.int.seas.harvard.edu:/group/brooks/mbuch/kernels/msm/out/android-msm-pixel-4.9/dist/Image.lz4

HASH=sha256sum Image.lz4
KERNEL_DIRNAME="msm-android10-${HASH}"

mkdir ${KERNEL_DIRNAME}

mv Image.lz4 ${KERNEL_DIRNAME}

scp -i ~/.ssh/chips mbuch@acc40.int.seas.harvard.edu:/group/brooks/mbuch/kernels/msm/out/android-msm-pixel-4.9/dist/*.ko ${KERNEL_DIRNAME}/

echo "Copied to: ${KERNEL_DIRNAME}"
