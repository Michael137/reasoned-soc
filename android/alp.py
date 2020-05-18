#!/usr/bin/python3 
'''
>> Htop for accelerators <<

Steps:
1. Log process
2. Get log
3. Filter based on accelerators
4. Calcualte Utilization
  - Approximated through logs
  - Detailed utilization: query perf counters where possible
    (e.g., https://www.exploit-db.com/exploits/39504)

TODO:
    - Log processing
    - Build perf library if necessary and possible
    - Pyton C bindings to call perf measurments

Probe Types:
    - Perf counters
    - Log w/ timestamp
        - Format:
            { <accelerator>:
                { <tag>:
                    <timestamp>: [ msg1, ..., msgN ]
                  ... },
                ...
            }
        - <tag> is one of:
            * ALP_DEBUG: Informational message
            * ALP_TIME: Timing message
'''

import sys
import subprocess as sp
import time as t
from datetime import *
from dateutil.parser import *
from timeit import default_timer as timer
# Regular Expressions
import re
from random import seed
import random
from pathlib import Path
import curses
import argparse

# For type annotations
from typing import List, Dict

# Custom libraries
from terminal_plot import *
from plots import *

DEBUG = False
VERBOSE = False

def printe(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)
    if exit:
        sys.exit()

def printw(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def printd(*args, **kwargs):
    if DEBUG:
        print(*args, file=sys.stderr, **kwargs)

def printv(*args, **kwargs):
    if VERBOSE:
        print(*args, **kwargs)

# Collect kernel log
def log_dmesg() -> str:
    cmd = ["dmesg", "-T"]
    output = run_via_adb(cmd)
    return output

def process_dmesg(log: str, probes: List[str] = ["DEBUG","IOCTL","TIME"]) -> Dict[str, List[float]]:
    lines = log.split('\n')

    results = {}
    for line in lines:
        probe = [x for x in probes if x in line]
        if len(probe) == 0:
            continue
        elif len(probe) > 1:
            printe("More than a single probe available for log line.\n{}\n{}".format(line,probe))
        else:
            probe = probe[0]

        if probe in line:
            if probe not in results:
                # TODO: should be dict where key is timestamp
                results[probe] = []
            results[probe].append(line)

    return results

# Collect Android log
def log_logcat() -> str:
    cmd = ["logcat", "-d"]
    output = run_via_adb(cmd)
    return output

def check_output(cmd: List[str]) -> str:
    try:
        # subprocess.check_output returns bytes so decode it to UTF-8 string
        return sp.check_output(cmd, stderr=sp.STDOUT).decode("utf-8").strip()
    except sp.CalledProcessError as e:
        raise RuntimeError("command '{}' return with error (code {}): {}".format(e.cmd, e.returncode, e.output))

# Execute commands through adb
def run_via_adb(user_cmd: List[str]) -> str:
    adb_cmd_prefix = ["adb", "shell"]
    cmd = adb_cmd_prefix + user_cmd
    return check_output(cmd)

def check_adb_root() -> bool:
    return run_via_adb(["whoami"]) == "root"

def check_reqs():
    required_bins = ["adb"]
    not_found = []
    for b in required_bins:
        try:
            sp.check_output("which " + b, stderr=sp.STDOUT, shell=True)
        except sp.CalledProcessError as e:
            not_found.append(b)
    if len(not_found) > 0:
        printe("ERROR: following required binaries not found:\n", not_found)

    # Is device connected?
    # NOTE: command will always return a header line followed by
    #       one line for each connected device
    devices = check_output(['adb', 'devices']).split('\n')
    devices = [x for x in devices if not x.isspace()]
    if len(devices) == 1:
        printe("ERROR: no devices connected")
    elif len(devices) > 2:
        printe("ERROR: this script expects only a single connected Android device")

    [device_name, adb_status] = devices[1].split()
    if adb_status == 'unauthorized':
        printe("ERROR: permission denied...please allow adb access on your device")

    # Is adb connected as root?
    if not check_adb_root():
        printw("WARNING: script requires adb in root...restarting as root")
        sp.run(["adb", "root"])
        if not check_adb_root():
            printe("ERROR: failed to restart adb in root")

    # TODO: Check if adb has write permissions

# Get utilization numbers via perf probe of driver with
# name "drv_name"
def perf(drv_name: str):
    raise NotImplementedError

# TODO: function return annotation
def extract_time(s: str):
    # Match timestamp s.a. "[Wed May 13 23:23:08 2020] ..."
    matched = re.search(r'\[(.*?)\]', s, re.M | re.I)
    # Extract timestamp
    ts = parse(matched.group(1))
    return ts

# TODO: function return annotation
def extract_pid(s: str):
    # Match pid tag s.a. "... (pid: 1234) ..."
    matched = re.search(r'\(pid:(.*?)\)', s, re.M | re.I)
    # Extract pid
    pid = matched.group(1)
    return pid

# threshold is in seconds
def calc_log_time(processed_log: Dict[str,List[str]],
                  threshold = 5 ) -> float:
    exec_times = []
    ioctl_times = []
    times = processed_log["TIME"]
    most_recent = extract_time(times[-1])
    elig_times = [t for t in times if extract_time(t) > (most_recent - timedelta(seconds=threshold))]

    pids = {}
    for e in elig_times:
        printd(e)
        pid = extract_pid(e)
        if '(execution' in e:
            if pid not in pids:
                pids[pid] = {'ioctl':[], 'exec':[]}

            time = float(e.split(' ')[-1])
            pids[pid]['exec'].append(time)
            exec_times.append(time)
        elif '(ioctl' in e:
            if pid not in pids:
                pids[pid] = {'ioctl':[], 'exec':[]}

            time = float(e.split(' ')[-1])
            pids[pid]['ioctl'].append(time)
            ioctl_times.append(time)

    # Ignore largest two execution measurement since it
    # is *always* an artifact of previous run
    # TODO: confirm above assumption
    exec_times.remove(max(exec_times))
    #exec_times.remove(max(exec_times))
    ioctl_times.remove(max(ioctl_times))
    #ioctl_times.remove(max(ioctl_times))

    exec_tot = sum(exec_times)
    ioctl_tot = sum(ioctl_times)

    for pid, v in pids.items():
        printd(pid, ',', sum(v['exec']), ',', sum(v['ioctl']))

    printd("len(exec_times): ", len(exec_times))
    printd("len(ioctl_times): ", len(ioctl_times))
    printd("Total execution time (s): ", exec_tot)
    printd("Total ioctl time (s): ", ioctl_tot)

    return 0

def calc_flush_count(processed_log: Dict[str,List[str]],
                     threshold = 20 ) -> Dict[str,int]:
    times = processed_log["DEBUG"]
    most_recent = extract_time(times[-1])
    elig_times = [t for t in times if extract_time(t) > (most_recent - timedelta(seconds=threshold))]

    flush_count = 0
    inv_count = 0
    for e in elig_times:
        if 'flushing cache' in e:
            flush_count += 1
        elif 'invalidate cache' in e:
            inv_count += 1

    printd("# of flushes: ", flush_count)
    printd("# of invalidations: ", inv_count)

    return { 'flush': flush_count, 'invalidate': inv_count }

# TODO: validate numbers?
# TODO: what is the the Adreno driver for? kgsl vs adreno?
def calc_accelerator_interaction_count(processed_log: Dict[str, List[str]], threshold = 20,
                                       check_full_log = False) -> Dict[str,int]:
    if len(processed_log) == 0:
        return {}

    most_recent = extract_time(processed_log[-1])
    if check_full_log:
        elig_times = processed_log
    else:
        assert(threshold > 0)
        elig_times = [t for t in reversed(processed_log) if extract_time(t) >= (most_recent - timedelta(seconds=threshold))]

    accelerators = {}
    for e in elig_times:
        # Match pid tag s.a. "... IOCTL aDSP ..."
        matched = re.search(r'IOCTL ([A-Za-z0-9]+):', e, re.M | re.I)

        if matched is not None:
            # Extract accelerator name
            accl = matched.group(1)
            if accl not in accelerators:
                accelerators[accl] = 0
            accelerators[accl] += 1

    return accelerators

######### Utilization Related ##############
def alp():
    return [ACCELERATORS, [0] * len(ACCELERATORS)]

# TODO: turn into generator
# TODO: test
class StreamDataNaive():
    prev_log = []
    # ref_count = 0

    def __init__(self):
        self.prev_log = process_dmesg(log_dmesg(), probes = ["IOCTL"])["IOCTL"]
        if len(self.prev_log) > 0:
            self.latest_ts = extract_time(self.prev_log[-1])
        else:
            self.latest_ts = datetime.min

    # TODO: can prev_log be init'ed in ctor?
    def __call__(self):
        log = process_dmesg(log_dmesg(), probes = ["IOCTL"])["IOCTL"]
        self.prev_log = [l for l in reversed(log) if extract_time(l) > self.latest_ts]
        if len(self.prev_log) > 0:
            self.latest_ts = extract_time(self.prev_log[-1])

        # New prev_log == newest stream
        return self.prev_log

######### Running tflite benchmarks ##############
# Config should be a list of models with absolute paths
def parse_model_cfg(cfg_path: str = str(Path.cwd() / 'models.cfg')) -> List[str]:
    models = []
    with open(cfg_path) as cfg_f:
        for line in cfg_f:
            models.append(line.strip())
            # TODO: assert that path exists

    assert(len(models) > 0)
    return models

# TODO: make configurable
def run_tflite_bench_random(model_path_pool: List[str], num_proc = 4) -> Dict[str, float]:
    assert(num_proc > 0)
    info_dict = {}
    base_cmd = ['/data/local/tmp/benchmark_model',  \
                '--num_threads=1',                  \
                '--use_hexagon=true',               \
                '--warmup_runs=1,'                  \
                '--num_runs=1',                     \
                '--hexagon_profiling=false',        \
                '--enable_op_profiling=false']

    models = [random.choice(model_path_pool) for _ in range(num_proc)]
    cmd = list.copy(base_cmd)
    for idx, model in enumerate(models):
        cmd += ['--graph=' + str(model), '&']
        if idx < len(models) - 1:
            cmd += base_cmd

    # Wait for all subprocesses to finish
    # TODO: make following work: cmd += [';', 'wait', '<', '<(jobs -p)']
    cmd += ['wait']

    printv('Running benchmark using: ', cmd)
    start_t = timer()
    run_via_adb(cmd)
    end_t = timer()

    info_dict['TIME'] = end_t - start_t
    return info_dict

# TODO: accept DEBUG as argument
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Answers all your questions about OS-Accelerator interactions')
    parser.add_argument('--debug', default=False, action='store_true', help='enable DEBUG mode; printd(...) will print to stderr')
    parser.add_argument('--gui', default=False, action='store_true', help='show htop-style GUI for accelerator utilization')
    parser.add_argument('--memory', default=False, action='store_true', help='Measure cache statistics for accelerators')
    parser.add_argument('--benchmark', default=False, action='store_true', help='run tflite benchmark')
    parser.add_argument('--verbose', default=False, action='store_true', help='verbose outputs')
    args = vars(parser.parse_args())
    DEBUG = args['debug']
    VERBOSE = args['verbose']

    random.seed(datetime.now())

    check_reqs()

    if args['gui']:
        data_streamer = StreamDataNaive()
        def barchart_data_streamer():
            data = calc_accelerator_interaction_count(data_streamer(), check_full_log = True)
            return [list(data.keys()), list(data.values())]
        try:
            curses.initscr()
            live_barchart(barchart_data_streamer)
        finally:
            curses.endwin()
    elif args['memory']:
        elapsed = round(run_tflite_bench_random(parse_model_cfg(), num_proc = 1)["TIME"])
        #calc_log_time(process_dmesg(log_dmesg(), probes = ["TIME"]), threshold = elapsed)
        print(calc_flush_count(process_dmesg(log_dmesg(), probes = ["DEBUG"]), threshold = elapsed))
    elif args['benchmark']:
        elapsed = round(run_tflite_bench_random(parse_model_cfg(), num_proc = 1)["TIME"])
        log = process_dmesg(log_dmesg(), probes = ["IOCTL"])["IOCTL"]
        printd(log)
        printd("interaction counts: ", calc_accelerator_interaction_count(
                                            log,
                                            threshold = elapsed))

    # elapsed = round(run_tflite_bench_random(parse_model_cfg(), num_proc = 1)["TIME"])
    # data_streamer = StreamDataNaive()
    # last_log = data_streamer()
    # printd("interaction counts: ", calc_accelerator_interaction_count(last_log,check_full_log = True))
    # new_log = data_streamer()
    # t.sleep(3)
    # printd("interaction counts: ", calc_accelerator_interaction_count(new_log,check_full_log = True))
