#!/usr/local/bin/python3

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
# Regular Expressions
import re
from random import seed
import random

# For type annotations
from typing import List, Dict

# from ascii_plot import *

DEBUG = True
ACCELERATORS = [
    # Qualcomm GPU (KGSL driver)
    "ardeno",
    
    # JPEG HW, PPROC, VFE, etc.
    "camera sensors",

    # VIDC
    "video codec",

    # Qualcomm IP Accelerator
    "IPA",

    # Qualcomm aDSP
    "adsp",

    # Qualcomm ICE
    "crypto",
    "others"
]

def printe(exit = True, *args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)
    if exit:
        sys.exit()

def printw(*args, **kwargs):
    printe(False, *args, **kwargs)

def printd(*args, **kwargs):
    if DEBUG:
        printw(*args, **kwargs)

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

def check_output(cmd: str) -> str:
    try:
        # subprocess.check_output returns bytes so decode it to UTF-8 string
        return sp.check_output(cmd, stderr=sp.STDOUT, shell=True).decode("utf-8").strip()
    except sp.CalledProcessError as e:
        raise RuntimeError("command '{}' return with error (code {}): {}".format(e.cmd, e.returncode, e.output))

# Execute commands through adb
def run_via_adb(user_cmd: List[str]) -> str:
    adb_cmd_prefix = ["adb", "shell"]
    cmd = ' '.join(adb_cmd_prefix + user_cmd)
    return check_output(cmd)

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
    devices = check_output("adb devices").split('\n')
    if len(devices) == 1:
        printe("ERROR: no devices connected")
    elif len(devices) > 2:
        printe("ERROR: this script expects only a single connected Android device")

    # Is adb connected as root?
    whoami = run_via_adb(["whoami"])
    if whoami != "root":
        printw("WARNING: script requires adb in root...restarting as root")
        sp.run(["adb", "root"])
        # TODO: check if root restart has been succesful

    # TODO: Check if adb has write permissions

def calc_utilization(processed_log: Dict[str,List[str]]) -> List[float]:
    printd(processed_log)
    return []

# Compute ALP stats
def alp():
    calc_utilization(process_dmesg(log_dmesg()))
    return

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
                  threshold = 20 ) -> float:
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

def calc_flash_count(processed_log: Dict[str,List[str]],
                     threshold = 20 ) -> float:
    times = processed_log["DEBUG"]
    most_recent = extract_time(times[-1])
    elig_times = [t for t in times if extract_time(t) > (most_recent - timedelta(seconds=threshold))]

    flush_count = 0
    inv_count = 0
    for e in elig_times:
        if 'flushing cache' in e:
            flush_count += 1
        if 'invalidate cache' in e:
            inv_count += 1

    printd("# of flushes: ", flush_count)
    printd("# of invalidations: ", inv_count)

    return 0

# TODO: see https://stackoverflow.com/questions/9854511/python-curses-dilemma
def utilization_stream():
    data = {"GPU": 20, "DSP": 40, "ICE": 1, "Others": 39}
    for k,v in data.items():
        data[k] = v + random.randint(0, 10)
    return data

if __name__ == "__main__":
    thr = 20
    seed(1)
    check_reqs()
    # alp()
    calc_log_time(process_dmesg(log_dmesg(), probes = ["TIME"]), threshold = thr)
    # barchart_loop(utilization_stream)
    calc_flash_count(process_dmesg(log_dmesg(), probes = ["DEBUG"]), threshold = thr)
