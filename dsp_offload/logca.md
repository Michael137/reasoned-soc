# What to measure?
1. [LogCA metrics](https://users.cs.duke.edu/~lkw34/papers/logca-isca2017.pdf):
  - (Latency) L: Cycles to move data from the host to the accelerator across the interface, including the cycles data spends in the caches or memory
  - (Overhead) o: Cycles the host spends in setting up the algorithm
  - (Granularity) g: Size of the offloaded data
  - (Computational index) C: Cycles the host spends per byte of data
  - (Acceleration) A: The peak speedup of an accelerator

# Methodology
## L
1. Time to map application into DSP memory
2. Time CPU waits for DSP (due to synchronous computation)
3. Time to signal between DSP<->CPU to inidicate when computation is done

## o
1. Measure FastRPC setup
2. Measure FastRPC teardown

## g
1. Image size
2. Model size

## c
1. " profile the CPU code on the host by varying the granularity from 16B to 32MB. At each granularity, we measure the execution time and use regression analysis to determine C"

## A
1. Derived
