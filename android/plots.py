import numpy as np
import matplotlib.pyplot as plt

# Input:
#   data[0]: ioctl data
#   data[1]: execution data
#   labels
def barchart(data, labels, title = ''):
    n_groups = len(data[0])

    ioctl_times = data[0]
    exec_times = data[1]

    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.35
    opacity = 0.8

    rects1 = plt.bar(index, ioctl_times, bar_width,
                     alpha=opacity,
                     label='Execution')

    rects2 = plt.bar(index + bar_width, exec_times, bar_width,
                     alpha=opacity,
                     label='IOCTl')

    plt.xlabel('PIDs')
    plt.ylabel('Latency (s)')
    plt.title(title)
    plt.xticks(index + bar_width, labels)
    plt.legend()

    plt.tight_layout()
    plt.show()

def barchart_stacked():
    raise NotImplementedError
