#!/usr/local/bin/python3

import curses
import time
import math
import os
import sys
import termios
import atexit
from select import select
from enum import Enum 


ACCELERATORS = [
    # Qualcomm GPU (KGSL driver)
    "ardeno",
    "kgsl",
    
    # Camera sensors: JPEG HW, PPROC, VFE, VIDC (video codec) etc.
    "vidioc",
    "cam_sensor",
    "v4l2",

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

class Modes(Enum):
    null = 0
    interaction = 1
    timing = 2

############## Non-blocking keyboard input ##############
# From: https://stackoverflow.com/a/22085679
class KBHit:
    def __init__(self):
        # Save the terminal settings
        self.fd = sys.stdin.fileno()
        self.new_term = termios.tcgetattr(self.fd)
        self.old_term = termios.tcgetattr(self.fd)

        # New terminal setting unbuffered
        self.new_term[3] = (self.new_term[3] & ~termios.ICANON & ~termios.ECHO)
        termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.new_term)

        # Support normal-terminal reset at exit
        atexit.register(self.set_normal_term)


    def set_normal_term(self):
        termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.old_term)


    def getch(self):
        ''' Returns a keyboard character after kbhit() has been called.
            Should not be called in the same program as getarrow().
        '''
        s = ''
        return sys.stdin.read(1)


    def getarrow(self):
        ''' Returns an arrow-key code after kbhit() has been called. Codes are
        0 : up
        1 : right
        2 : down
        3 : left
        Should not be called in the same program as getch().
        '''
        c = sys.stdin.read(3)[2]
        vals = [65, 67, 66, 68]

        return vals.index(ord(c.decode('utf-8')))

    def kbhit(self):
        ''' Was keyboard hit? '''
        dr,dw,de = select([sys.stdin], [], [], 0)
        return dr != []

#################################################################################
SEPARATOR = "|"
LABELS_D = { Modes.null : "Exit (q/ESC)",
             Modes.interaction : "Interaction Counts (i)",
             Modes.timing : "Timing Counts (t)" }

def draw_menu_bar(win, startx, starty, menu_height, menu_width):
    win.attron(curses.color_pair(3))
    for i in range(menu_height):
        win.addstr(startx + i, starty, " " * menu_width)

    len_drawn = starty
    for _,l in LABELS_D.items():
        win.addstr(startx, len_drawn, l)
        len_drawn += len(l)
        win.addstr(startx, len_drawn, SEPARATOR)
        len_drawn += len(SEPARATOR)
    win.attroff(curses.color_pair(3))

def highlight_label(win, startx, starty, label, color_pair):
    win.attron(color_pair)
    len_drawn = starty
    for k,l in LABELS_D.items():
        if k == label:
            break
        len_drawn += len(l) + len(SEPARATOR)
    win.addstr(startx, len_drawn, l)
    win.attroff(color_pair)

def reset_labels_exclude(win, startx, starty, label):
    for k,l in LABELS_D.items():
        if k != label:
            highlight_label(win, startx, starty, k, curses.color_pair(3))

# TODO: rewrite using panels?
def live_barchart(gen, maxY, maxX):
    # Initial data
    [init_labels, _] = gen_init_data()
    # WIN_ROW = len(init_labels) + 2
    WIN_ROW = maxY
    # Left (1) and right (1) border and labels and label separator
    LABEL_PADDING = max([len(x) for x in init_labels]) + 2
    BORDER_WIDTH = 1
    PADDING = LABEL_PADDING + 2 * BORDER_WIDTH
    WIN_COL = maxX
    WIN_X = 0
    WIN_Y = 0

    win = curses.newwin(maxY, WIN_COL, WIN_X, WIN_Y)
    win.border(0)

    for idx, lbl in enumerate(init_labels):
        win.addstr(idx + BORDER_WIDTH, BORDER_WIDTH, "{}| ".format(lbl))

    tmp = range(0, len(ACCELERATORS))
    label_indices = {ACCELERATORS[i]: tmp[i] for i in range(0, len(ACCELERATORS))}

    kb = KBHit()

    curses.start_color()
    curses.init_pair(1, curses.COLOR_CYAN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_BLACK, curses.COLOR_YELLOW)
    curses.init_pair(3, curses.COLOR_BLACK, curses.COLOR_WHITE)

    MENU_HEIGHT = 1
    MENU_WIDTH = WIN_COL - 2 * BORDER_WIDTH
    draw_menu_bar(win, WIN_ROW - MENU_HEIGHT - BORDER_WIDTH, BORDER_WIDTH, MENU_HEIGHT, MENU_WIDTH)
    win.refresh()

    mode = Modes.interaction

    while True:
        #for idx,val in enumerate(data):
        #    win.addstr(idx, 0, "{}".format(data[idx]))
        #    win.refresh()
        #    time.sleep(0.03)

        time.sleep(0.1)

        if kb.kbhit():
            c = kb.getch()
            if ord(c) == 27: # ESC
                break
            elif ord(c) == 113: # q
                break
            elif ord(c) == 105: # i
                mode = Modes.interaction
                highlight_label(win, WIN_ROW - MENU_HEIGHT - BORDER_WIDTH, BORDER_WIDTH,
                                mode, curses.color_pair(2))
                reset_labels_exclude(win, WIN_ROW - MENU_HEIGHT - BORDER_WIDTH, BORDER_WIDTH, mode)
            elif ord(c) == 116: # t
                mode = Modes.timing
                highlight_label(win, WIN_ROW - MENU_HEIGHT - BORDER_WIDTH, BORDER_WIDTH,
                                mode, curses.color_pair(2))
                reset_labels_exclude(win, WIN_ROW - MENU_HEIGHT - BORDER_WIDTH, BORDER_WIDTH, mode)

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

            # Draw bars
            for j in range(num_ticks):
                if num_ticks != 0:
                    win.addstr(row_idx, j + BORDER_WIDTH + LABEL_PADDING, "{}".format(display))
        win.refresh()

    kb.set_normal_term()
