#!/usr/local/bin/python3

import curses
import time
import math

ACCELERATORS = [
    # Qualcomm GPU (KGSL driver)
    "ardeno",
    "kgsl",
    
    # JPEG HW, PPROC, VFE, VIDC (video codec) etc.
    "Camera Sensors",
    "vidioc",

    # Qualcomm IP Accelerator
    "IPA",

    # Qualcomm aDSP
    "aDSP",

    # Qualcomm ICE
    "ICE",
    "Others"
]

def gen_test_data():
    return [['GPU', 'DSP', 'PPROC', 'ICE', 'Others'],
            [1, 8, 3, 4, 5]]

def gen_init_data():
    return [ACCELERATORS,
            [0] * len(ACCELERATORS)]

def live_barchart(gen):
    # Initial data
    [init_labels, _] = gen_init_data()
    WIN_ROW = len(init_labels) + 2
    # Left (1) and right (1) border and labels and label separator
    LABEL_PADDING = max([len(x) for x in init_labels]) + 2
    PADDING = LABEL_PADDING
    BORDER_WIDTH = 1
    WIN_COL = 64 + LABEL_PADDING + 2 * BORDER_WIDTH
    WIN_X = 0
    WIN_Y = 0

    win = curses.newwin(WIN_ROW, WIN_COL, WIN_X, WIN_Y)
    win.border(0)

    for idx, lbl in enumerate(init_labels):
        win.addstr(idx + BORDER_WIDTH, BORDER_WIDTH, "{}| ".format(lbl))
    win.refresh()

    tmp = range(0, len(ACCELERATORS))
    label_indices = {ACCELERATORS[i]: tmp[i] for i in range(0, len(ACCELERATORS))}

    while True:
        #for idx,val in enumerate(data):
        #    win.addstr(idx, 0, "{}".format(data[idx]))
        #    win.refresh()
        #    time.sleep(0.03)

        time.sleep(0.5)
        [labels, data] = gen()
        assert(len(labels) == len(data))
        for a in ACCELERATORS:
            if a not in labels:
                data.append(0)
                labels.append(a)

        display = '#'
        for i in range(len(data)):
            cur_max = max(data)
            if data[i] > 0:
                scale = data[i] / cur_max
                scaled = (WIN_COL - (2 * BORDER_WIDTH + LABEL_PADDING)) * scale
                if scaled > 0:
                    scaled = max(1, math.floor(scaled))

                num_ticks = scaled
            else:
                num_ticks = 0

            row_idx = label_indices[labels[i]] + BORDER_WIDTH
            # Clear bars
            for j in range(BORDER_WIDTH + LABEL_PADDING, (WIN_COL - BORDER_WIDTH)):
                win.addstr(row_idx, j, ' ')

            for j in range(num_ticks):
                if num_ticks != 0:
                    win.addstr(row_idx, j + BORDER_WIDTH + LABEL_PADDING, "{}".format(display))
        win.refresh()
