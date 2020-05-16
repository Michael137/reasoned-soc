#!/usr/local/bin/python3

import curses
import time
import math

def test_gen_data():
    return [['GPU', 'DSP', 'PPROC', 'ICE', 'Others'],
            [1, 8, 3, 4, 5]]

def live_barchart(gen):
    # Initial data
    [labels, data] = gen()
    WIN_ROW = len(labels) + 2
    # Left (1) and right (1) border and labels and label separator
    LABEL_PADDING = max([len(x) for x in labels]) + 2
    PADDING = LABEL_PADDING
    BORDER_WIDTH = 1
    WIN_COL = 64 + LABEL_PADDING + 2 * BORDER_WIDTH
    WIN_X = 0
    WIN_Y = 0

    win = curses.newwin(WIN_ROW, WIN_COL, WIN_X, WIN_Y)
    win.border(0)

    for idx, lbl in enumerate(labels):
        win.addstr(idx + BORDER_WIDTH, BORDER_WIDTH, "{}| ".format(lbl))

    while True:
        time.sleep(0.03)
        display = '#'
        for i in range(len(data)):
            cur_max = max(data)
            if data[i] > 0:
                scale = data[i] / cur_max
                num_ticks = math.floor((WIN_COL - (2 * BORDER_WIDTH + LABEL_PADDING)) * scale)
            else:
                num_ticks = 0

            for j in range(num_ticks):
                if num_ticks != 0:
                    win.addstr(i + BORDER_WIDTH, j + BORDER_WIDTH + LABEL_PADDING, "{}".format(display))
        win.refresh()
