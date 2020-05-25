#!/usr/local/bin/python3

import matplotlib.pyplot as plt
import csv

x = []
y = []

plot_d = {}
procs = [0,1,2,3,4,8,16,64] # 0th element is for initial noise
procs_found = 0
latest_ts = 0

with open(f"tflite_nnapi_64p.csv",'r') as csvfile:
    rows = csv.reader(csvfile, delimiter=',')
    next(rows) # skip headers
    for row in rows:
        metric = row[1]
        if metric not in plot_d:
            plot_d[metric] = {}

        timestamp = int(row[2])
        if ((timestamp - latest_ts) > 1000000 or (timestamp - latest_ts) < 0) and (latest_ts > 0):
            procs_found += 1
            print(timestamp - latest_ts)
            if procs_found == len(procs):
                procs_found = 0
        latest_ts = timestamp

        proc = procs[procs_found]
        if proc not in plot_d[metric]:
            plot_d[metric][proc] = []

        plot_d[metric][proc].append(row)

metrics = plot_d.keys()

for proc in procs:
    for i,metric in enumerate(metrics):
        print(metric,proc,len(plot_d[metric][proc]))

for proc in procs[1:]:
    fig,ax = plt.subplots(14,1,figsize = (8,35))
    ax[0].set_xlabel('Timestamp')
    ax[0].set_title(f"{proc} Concurrently Scheduled Models")
    for i,metric in enumerate(metrics):
        x = [] # timestamps
        y = [] # values
        for row in plot_d[metric][proc]:
            x.append(int(row[2]))
            y.append(float(row[4]))
        if "AXI" in metric:
            ax[i].set_ylabel(f"{metric} bytes/s")
        else:
            ax[i].set_ylabel(metric)
        ax[i].plot(x, y)
        ax[i].grid(True)
    plt.tight_layout()
    plt.savefig(f"{proc}_tflite_nnapi.pdf")
    plt.close()
