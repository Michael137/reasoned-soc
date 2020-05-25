#!/usr/local/bin/python3

import matplotlib.pyplot as plt
import csv
import sys

x = []
y = []

plot_d = {'hexagon' : {},
          'nnapi': {}}
procs = [0,1,2,3,4,8,16,64] # 0th element is for initial noise
procs_found = 0
latest_ts = 0

for framework in plot_d.keys():
    with open(f"tflite_{framework}_64p.csv",'r') as csvfile:
        rows = csv.reader(csvfile, delimiter=',')
        next(rows) # skip headers
        for row in rows:
            metric = row[1]
            if metric not in plot_d[framework]:
                plot_d[framework][metric] = {}

            timestamp = int(row[2])
            if ((timestamp - latest_ts) > 100000 or (timestamp - latest_ts) < 0) and (latest_ts > 0):
                procs_found += 1
                if procs_found == len(procs):
                    procs_found = 0
            latest_ts = timestamp

            proc = procs[procs_found]
            if proc not in plot_d[framework][metric]:
                plot_d[framework][metric][proc] = []

            plot_d[framework][metric][proc].append(row)

#for framework in plot_d.keys():

fw1 = 'hexagon'
fw2 = 'nnapi'
metrics = plot_d[fw1].keys()

for proc in procs[1:]:
    fig,ax = plt.subplots(len(metrics), 2, figsize = (10,40))
    ax[0][0].set_title(f"{fw1} {proc} Concurrently Scheduled Models")
    ax[0][1].set_title(f"{fw2} {proc} Concurrently Scheduled Models")
    for i,metric in enumerate(metrics):
        ax[i][0].set_xlabel('Timestamp')
        ax[i][1].set_xlabel('Timestamp')

        x = [] # timestamps
        y = [] # values
        for row in plot_d[fw1][metric][proc]:
            x.append(int(row[2]))
            y.append(float(row[4]))
        if "AXI" in metric:
            ax[i][0].set_ylabel(f"{metric} bytes/s")
        else:
            ax[i][0].set_ylabel(metric)
        ax[i][0].plot(x, y)
        ax[i][0].grid(True)

        x = [] # timestamps
        y = [] # values
        if metric not in plot_d[fw2]:
            continue

        for row in plot_d[fw2][metric][proc]:
            x.append(int(row[2]))
            y.append(float(row[4]))
        if "AXI" in metric:
            ax[i][1].set_ylabel(f"{metric} bytes/s")
        else:
            ax[i][1].set_ylabel(metric)
        ax[i][1].plot(x, y)
        ax[i][1].grid(True)
    plt.tight_layout()
    plt.savefig(f"{proc}_tflite.pdf")
    plt.close()
