exec_times = []
ioctl_times = []

def str2sec(s):
    return float(s) * 10E-9

with open('fastrpc_processed.txt') as f:
    for line in f:
        if 'TIME' in line:
            if '(execution' in line:
                exec_times.append(float(line.split(' ')[-1]))
            elif '(ioctl' in line:
                ioctl_times.append(float(line.split(' ')[-1]))

#exclude = [x for x in exec_times if x > 1]
#print(len(exclude))

#exec_times = [x for x in exec_times if x < 5]
#ioctl_times = [x for x in ioctl_times if x < 5]

exec_tot = sum(exec_times)
ioctl_tot = sum(ioctl_times) - exec_tot

print(exec_tot)
print(ioctl_tot)
