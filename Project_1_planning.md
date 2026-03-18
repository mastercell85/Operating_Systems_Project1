# CS4328 Project 1 - Dynamic Round-Robin Scheduler Planning

## Requirements Checklist (from PDF)

### Section 1.1 - Workload Generation
- [ ] 10,000 processes per simulation run
- [ ] Each process has:
  - [ ] Process ID (PID): unique integer
  - [ ] Arrival Time (A_i): Poisson arrival process with rate λ
  - [ ] Service Time (S_i): exponentially distributed total CPU time
  - [ ] Static Priority (P_i): uniform random, 1 (Highest) to 10 (Lowest)

### Section 1.2 - The Scheduler
- [ ] Dynamic Round-Robin (not fixed quantum)
- [ ] Quantum formula: `q_i = base + (P_Low - P_i) / A - B * T_i`
  - [ ] `base`: starting quantum value (command-line arg)
  - [ ] `P_Low`: lowest priority = 10 (highest numerical value)
  - [ ] `P_i`: process's static priority
  - [ ] `A`: global scaling factor (command-line arg)
  - [ ] `T_i`: process's accumulated total execution time
  - [ ] `B`: global scaling factor (command-line arg)
- [ ] Quantum computed fresh each time a process is about to run
- [ ] Ensure no negative quantum (clamp to minimum > 0)

### Section 1.3 - Performance Metrics
- [ ] Average turnaround time
- [ ] Average number of context switches
- [ ] Average number of processes in the ready queue

### Section 2 - The Simulator
- [ ] Discrete-time event simulation
- [ ] Event types: process arrival, process completion, quantum expiration
- [ ] Event Queue: priority queue sorted by event time
- [ ] Clock variable: starts at first event time, advances event-to-event
- [ ] 5 command-line arguments (in this exact order):
  1. [ ] Average arrival rate λ
  2. [ ] Average CPU service time
  3. [ ] Base quantum
  4. [ ] Scaling Factor A
  5. [ ] Scaling Factor B
- [ ] Ready Queue for processes waiting for CPU

### Section 3 - Scheduler Design & Analysis
- [ ] Default test scenario: λ = 12, avg service time = 0.06
- [ ] Vary base quantum and scaling factors A, B
- [ ] Study impact on all 3 performance metrics
- [ ] Verify: A = very large number, B = 0 → vanilla RR with fixed base quantum

### Section 4 - Deliverables
- [ ] Analysis report covering:
  1. [ ] Best values of base, A, B for good metric tradeoffs (with plots)
  2. [ ] Starvation concerns analysis + how to tackle them
  3. [ ] Graph: priority vs. turnaround time
  4. [ ] Compare recommended dynamic RR vs. vanilla RR (base quantum only)
- [ ] Code submission
- [ ] Compile and run instructions (we will use OnlineGDB)
- [ ] Report with results and interpretation
- [ ] Must run on CS Linux servers via command line (no GUI)

### Grading Breakdown
- 30% correct design and data structures
- 30% correct results
- 30% proper analysis report (explanation, compile/run instructions, etc.)
- (10% unspecified — likely code quality/style)

---

## Constraints
- Single `.cpp` file (for OnlineGDB compatibility)
- Standard C++ only (C++11/14 — no platform-specific headers)
- Command-line arguments entered via OnlineGDB's "Command line arguments" field
- Must also compile with `g++` on CS Linux servers

---

## Part 1: Data Structures

### 1A - Process Struct
```cpp
struct Process {
    int pid;                  // unique identifier
    double arrivalTime;       // when process arrived
    double serviceTime;       // total CPU time needed (S_i)
    double remainingTime;     // CPU time still needed
    double totalExecutedTime; // T_i — accumulated CPU time used so far
    int priority;             // static priority 1-10 (1 = highest)
};
```

### 1B - Event Struct (refactored from PA_2)
```cpp
const int ARRIVAL = 0;
const int DEPARTURE = 1;
const int QUANTUM_EXPIRATION = 2;

struct Event {
    double eventTime;
    int eventType;   // ARRIVAL, DEPARTURE, or QUANTUM_EXPIRATION
    int processID;
};
```

### 1C - EventQueue (reuse from PA_2)
- Sorted linked-list priority queue ordered by eventTime
- Methods: `insert()`, `getNextEvent()`, `isEmpty()`
- Carries over directly from PA_2 with no changes needed

### 1D - Process Storage
- `std::map<int, Process> processTable` — look up any process by PID
- Needed because processes go back and forth between ready queue and CPU

### 1E - Ready Queue
- `std::queue<int>` holding PIDs (FIFO order for Round-Robin)

---

## Part 2: Workload Generation

Generated on-the-fly as arrival events fire (same approach as PA_2):

- **Inter-arrival time**: `expRandom(lambda)` — exponential with rate λ (Poisson process)
- **Service time**: `expRandom(1.0 / avgServiceTime)` — exponential with given mean
- **Priority**: `(rand() % 10) + 1` — uniform integer 1 to 10
- **Initialize**: `remainingTime = serviceTime`, `totalExecutedTime = 0.0`

Reuse `expRandom()` and `randomNumBetween0And1()` from PA_2.

---

## Part 3: Dynamic Quantum Calculation

```
q_i = base + (P_Low - P_i) / A - B * T_i
```

- `P_Low` = 10 (constant)
- Computed each time a process is dispatched to CPU
- **Clamp**: if `q_i < MIN_QUANTUM` (e.g., 0.001), set `q_i = MIN_QUANTUM`
  - Prevents negative or zero quantum from age penalty

### Examples:
- Priority 1, T_i=0: `q = base + 9/A` (longest quantum — highest priority, fresh process)
- Priority 10, T_i=0: `q = base + 0/A` (just base — lowest priority)
- Priority 1, large T_i: `q = base + 9/A - B*T_i` (shrinks over time)

---

## Part 4: Simulation Event Loop

### Initialization
1. Parse 5 command-line args
2. Seed random number generator
3. Schedule first arrival event at `t = expRandom(lambda)`
4. Set `clock = 0`, `cpuBusy = false`

### Event Handling

**ARRIVAL:**
1. Create Process with PID, arrivalTime=eventTime, serviceTime, remainingTime=serviceTime, totalExecutedTime=0, priority
2. Store in processTable
3. Schedule next arrival: `eventTime + expRandom(lambda)`
4. If CPU idle → dispatch this process
5. If CPU busy → push PID to ready queue

**DISPATCH (helper — called whenever CPU needs to start running a process):**
1. Compute quantum: `q_i = base + (10 - P_i) / A - B * T_i`
2. Clamp quantum to minimum
3. If `remainingTime <= q_i`:
   - Process will finish → schedule DEPARTURE at `clock + remainingTime`
4. If `remainingTime > q_i`:
   - Process will be preempted → schedule QUANTUM_EXPIRATION at `clock + q_i`
5. Increment total context switch counter
6. Set `cpuBusy = true`
7. Record which process is currently on CPU

**DEPARTURE:**
1. Get the time slice just used: `timeUsed = process.remainingTime` (it finished)
2. Update `process.totalExecutedTime += timeUsed`
3. Update `process.remainingTime = 0`
4. Calculate turnaround: `clock - process.arrivalTime`
5. Add to `totalTurnaroundTime`
6. Increment `completedProcesses`
7. If ready queue not empty → dispatch next process (front of queue)
8. If ready queue empty → `cpuBusy = false`

**QUANTUM_EXPIRATION:**
1. `timeUsed = quantum that was assigned` (the q_i computed at dispatch)
2. Update `process.remainingTime -= timeUsed`
3. Update `process.totalExecutedTime += timeUsed`
4. Push this process PID to back of ready queue
5. Dispatch next process from front of ready queue
   - (The preempted process is now at the back, so a different process runs next — unless it's the only one, then it gets re-dispatched)

### Metrics Tracking (at EVERY event)
Before advancing clock:
```
deltaT = currentEvent.eventTime - lastEventTime
areaUnderQueueCurve += readyQueue.size() * deltaT
```
Then advance clock and lastEventTime.

Context switches: incremented inside DISPATCH helper.

### Loop Termination
- Continue until `completedProcesses >= 10000`

---

## Part 5: Metrics Calculation & Output

After loop ends:
```
avgTurnaroundTime   = totalTurnaroundTime / completedProcesses
avgContextSwitches  = totalContextSwitches / completedProcesses
avgReadyQueueLength = areaUnderQueueCurve / clock
```

Print to console in clear format.

**For report graphs**, also optionally output:
- Per-priority turnaround times (for priority vs. turnaround graph)
- CSV output for easy plotting

---

## Part 6: Command-Line Argument Parsing

```
Usage: ./simulator <lambda> <avgServiceTime> <baseQuantum> <A> <B>

Example: ./simulator 12 0.06 0.03 5 0.01
```

- Parse with `atof(argv[i])` or `std::stod(argv[i])`
- Print usage and exit if `argc != 6`
- Print the input parameters back to confirm

---

## Part 7: Testing & Validation

### Vanilla RR Validation
- Run with A = 1000000, B = 0 → every process gets quantum ≈ base
- Compare behavior to PA_2's FCFS (won't match exactly since RR != FCFS, but sanity check)

### Sanity Checks
- CPU utilization ≈ λ * avgServiceTime (when < 1.0, system is stable)
  - λ=12, service=0.06 → utilization ≈ 0.72 (72%)
- All 10,000 processes should complete
- No process should have negative remaining time
- Turnaround time should always be ≥ service time

### Parameter Sweep for Report
- Fix λ=12, avgService=0.06
- Vary base: e.g., 0.01, 0.02, 0.03, 0.05, 0.1
- Vary A: e.g., 1, 2, 5, 10, 100, 1000000
- Vary B: e.g., 0, 0.001, 0.01, 0.05, 0.1
- Record all 3 metrics for each combination

---

## Part 8: Report (after code works)

### Required Sections
1. **Tradeoff Analysis**: best base/A/B values with supporting plots
2. **Starvation Analysis**:
   - Do low-priority processes starve? (Check turnaround for priority 10 vs 1)
   - Mitigation ideas: aging mechanism, minimum quantum floor, etc.
3. **Priority vs. Turnaround Graph**: scatter or bar chart
4. **Dynamic RR vs. Vanilla RR**: side-by-side metric comparison
5. **Compile/run instructions** (OnlineGDB + g++ command)

---

## Implementation Order

1. [ ] Set up `main()` with command-line argument parsing and validation
2. [ ] Define `Process` struct and `std::map<int, Process>` storage
3. [ ] Refactor `Event` struct — add `QUANTUM_EXPIRATION` type
4. [ ] Carry over `EventQueue` class from PA_2 (no changes needed)
5. [ ] Carry over `expRandom()` and `randomNumBetween0And1()` from PA_2
6. [ ] Implement arrival handling: create process with priority, schedule next arrival
7. [ ] Implement dispatch helper: compute dynamic quantum, schedule correct event type
8. [ ] Implement departure handling: turnaround calc, dispatch next or go idle
9. [ ] Implement quantum expiration handling: update remaining time, re-queue, dispatch next
10. [ ] Add metrics tracking: turnaround, context switches, ready queue area
11. [ ] Calculate and print final metrics
12. [ ] Add per-priority turnaround tracking (for report graph)
13. [ ] Test: vanilla RR (A=large, B=0) — verify behavior
14. [ ] Test: default scenario (λ=12, service=0.06) with various base/A/B
15. [ ] Add CSV output for plotting
16. [ ] Parameter sweep runs for report data
17. [ ] Write analysis report
18. [ ] Final test on OnlineGDB to confirm compatibility
