#!/usr/local/bin/python3

import curses
import time

# For type annotations
from typing import List, Dict

stdscr = curses.initscr()

def barchart(data: Dict[str,int]):
    max_value = max(count for _, count in data.items())
    increment = max_value / 25
    
    longest_label_length = max(len(label) for label, _ in data.items())

    i = 0
    for label, count in data.items():
    
        bar_chunks, remainder = divmod(int(count * 8 / increment), 8)
        # First draw the full width chunks
        bar = '#' * bar_chunks
    
        # If the bar is empty, add a left one-eighth block
        bar = bar or  '| '

        stdscr.addstr(i, 0, "{} {} {}".format(label.rjust(longest_label_length), count, bar))
        stdscr.refresh()
        i += 1

def barchart_loop(generator_fn):
    curses.noecho()
    curses.cbreak()

    try:
        while True:
            barchart(generator_fn())
            time.sleep(1)
    finally:
        curses.echo()
        curses.nocbreak()
        curses.endwin()
