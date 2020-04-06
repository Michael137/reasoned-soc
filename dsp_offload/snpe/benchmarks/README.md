# FastRPC Benchmarking

Master benchmark object is created in `snpe_bm.py` in `BenchmarkFactory`'s `make_benchmarks`. It uses the `SnapDnnCppDroidBenchmark` class to create a script that executes benchmarking commands.
The core of the benchmark command is: `snpe-net-run <configuration options>` where the flags are taken from the benchmark JSON. E.g., it adds `--debug`, `--profiling_level`, `--perf_profile` to the command.
The source to [snpe-net-run](https://developer.qualcomm.com/docs/snpe/benchmarking.html) that's used for `snpe_bench.py` is not public but a similar application is available in the examples (see [this thread](https://developer.qualcomm.com/comment/16248)): [$SNPE_ROOT/examples/NativeCpp/SampleCode](https://developer.qualcomm.com/docs/snpe/cplus_plus_tutorial.html)

Profiling capabilities are exposed through the SNPEBuilder C++ class. Setting profiling can be done as described in [these docs](https://developer.qualcomm.com/docs/snpe/group__c__plus__plus__apis.html).
