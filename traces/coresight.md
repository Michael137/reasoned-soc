# Overview
- Hardware tracing provides a low-overhead technique for gathering -level SoC
- [Examples of hardware instrumentation](http://events17.linuxfoundation.org/sites/events/files/slides/Need_to_Power_Instrument_Linux_Kernel_v4.pdf)
- [Hardware tracing tools](https://hsdm.dorsal.polymtl.ca/system/files/may2016.pdf)

# ARM CoreSight
- It is support on the Qualcomm dev board used in our experiments
- Advanced Microcontroller Bus Architecture (AMBA): protocol for ARM on-chip buses
- Advanced Trace Bus (ATB): carries trace data around a SoC
- Cross Trigger Interface (CTI): interface that connects multiple CoreSight components and feeds events from components to the cross trigger matrix
- Cross Trigger Matrix (CTM): connects three or more channel interfaces
- Embedded Cross Trigger (ECT): contains CTM and CTI
- Embedded Trace Buffer (ETB)
- Embedded Trace Macrocell (ETM): hardware macrocell that outputs *trace information* to a *trace port*
- System Trace Macrocell (STM)
- Trace Funnel: device that combines multiple traces sources onto a single bus
- AHB Trace Macrocell (HTM): trace source that makes bus information visible
- [Hardware Assisted Tracing](https://elinux.org/images/b/b3/Hardware_Assisted_Tracing_on_ARM.pdf):
  - **Each core** has a **companion** IP block: **Embedded Trace Macrocell** (ETM)
  - **OS drivers program macrocells** with tracing characteristics
  - Once triggered trace macrocells **operate independently**
  - Funnel collects ETM information from each core and captures them into trace capture buffers (ETB)
