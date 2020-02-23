# Profiling MLPerf Models

In this directory we keep the models built for MLPerf's inference benchmarks.

## Qualcomm
### Open v0.5
- [Snapdragon 855](https://github.com/mlperf/inference_results_v0.5/blob/master/open/Qualcomm/systems/SDM855.json)
- Uses [SNPE v1.30](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk/tools)
- Installation:
  - Copy the [mlperf.patch](mlperf.patch) into the root directory for the Qualcomm run ([code availablefrom here](https://github.com/mlperf/inference_results_v0.5/tree/master/open/Qualcomm/code)
  - Run: `git apply mlperf.patch`
  - Follow their installation and run instructions

### Closed v0.5
