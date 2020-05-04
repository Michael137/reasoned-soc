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
'''

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


