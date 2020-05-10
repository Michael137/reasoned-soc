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
'''

import subprocess as sp

import sys

# For type annotations
from typing import List, Dict

DEBUG = False
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

def process_dmesg(log: str) -> Dict[str, List[float]]:
    lines = log.split('\n')

    probes = ["DEBUG","IOCTL","TIME"]
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

if __name__ == "__main__":
    check_reqs()
    alp()
