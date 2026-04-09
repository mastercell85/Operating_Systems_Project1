# Dynamic Round-Robin CPU Scheduler Simulation

A discrete-event simulation of a dynamic Round-Robin CPU scheduler that adjusts each process's time quantum based on priority and accumulated CPU time. Written in C++ for CS 4328 (Operating Systems) at Texas State University.

## Overview

The scheduler simulates 10,000 processes with Poisson arrivals and exponentially distributed service times. Each process is assigned a static priority (1-10, where 1 is highest). Instead of a fixed time quantum, the scheduler computes a per-process quantum using:

```
q_i = base + (P_low - P_i) / A - B * T_i
```

where:
- **base** — base time quantum
- **P_low** — lowest priority value (10)
- **P_i** — priority of process *i*
- **A** — priority scaling factor
- **B** — age penalty scaling factor
- **T_i** — accumulated CPU time used by process *i*

High-priority processes receive larger quanta, and processes that have already consumed significant CPU time get smaller quanta — balancing responsiveness with fairness.

## How to Compile

```bash
g++ -std=c++11 -O2 -o scheduler_sim scheduler_sim.cpp
```

## How to Run

```bash
./scheduler_sim <lambda> <avgServiceTime> <baseQuantum> <A> <B>
```

| Argument | Description |
|---|---|
| `lambda` | Average process arrival rate (Poisson) |
| `avgServiceTime` | Average CPU service time (exponential distribution) |
| `baseQuantum` | Base time quantum |
| `A` | Priority scaling factor |
| `B` | Age penalty scaling factor |

**Example:**

```bash
./scheduler_sim 12 0.06 0.06 5 0.1
```

Running with no arguments enters interactive mode, prompting for each parameter.

## Key Results

Optimal configuration found via 360 parameter sweep runs: **base=0.06, A=5, B=0.1**.

| Metric | Dynamic RR | Vanilla RR (A=99999, B=0) |
|---|---|---|
| Avg turnaround time | 0.2018 s | 0.2034 s |
| Avg context switches/process | 1.07 | 1.58 |
| Avg ready queue length | 1.69 | 1.71 |

The dynamic scheduler reduces context switches by ~32% compared to vanilla Round-Robin at the same base quantum, with comparable turnaround times.

**Starvation note:** Priority 10 (lowest) processes wait ~40% longer than priority 1 (0.277 s vs 0.197 s), a trade-off of the priority-weighted quantum.

Setting A to a very large value and B=0 degrades the dynamic scheduler to standard Round-Robin, which was used for the comparison baseline.

## Project Context

Written for CS 4328 — Operating Systems, Texas State University. The assignment required implementing a CPU scheduling simulation with configurable parameters and analyzing performance across different configurations.
