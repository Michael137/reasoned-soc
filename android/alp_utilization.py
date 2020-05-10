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
from typing import List

accelerators = [
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

# Collect kernel log
def log_dmesg() -> str:
    cmd = ["dmesg", "-T"]
    output = run_via_adb(cmd)
    return output

def process_dmesg(log: str) -> List[str]:
    lines = log.split('\n')

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
        print("ERROR: following required binaries not found: ")
        print(not_found)
        sys.exit()

    # Is device connected?
    # NOTE: command will always return a header line followed by
    #       one line for each connected device
    devices = check_output("adb devices").split('\n')
    if len(devices) == 1:
        print("ERROR: no devices connected")
        sys.exit()
    elif len(devices) > 2:
        print("ERROR: this script expects only a single connected Android device")
        sys.exit()

    # Is adb connected as root?
    whoami = run_via_adb(["whoami"])
    if whoami != "root":
        print("WARNING: script requires adb in root...restarting as root")
        sp.run(["adb", "root"])
        # TODO: check if root restart has been succesful

    # TODO: Check if adb has write permissions

def calc_utilization(processed_log: List[str]) -> List[float]:
    #raise NotImplementedError
    print(processed_log)
    return []

# Compute ALP stats
def alp():
    calc_utilization(process_dmesg(log_dmesg))

check_reqs()

