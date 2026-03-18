# Change Log - CS4328 Project 1

## Format
Each entry: what was implemented/changed, why, and timestamp.

---

### 2026-03-17 — Initial Setup
- **Renamed** `PA_2_ctz18.cpp` → `scheduler_sim.cpp`
  - Why: Old name was from Computer Systems Fundamentals PA_2. New name reflects this project's purpose.
- **Created** `Project_1_planning.md`
  - Why: Step-by-step plan covering all PDF requirements before coding begins.
- **Created** `change_log.md`
  - Why: Track all implementations and changes with reasoning and timestamps.

### 2026-03-17 — Part 1: Data Structures & Command-Line Parsing
- **Added** `Process` struct with fields: `pid`, `arrivalTime`, `serviceTime`, `remainingTime`, `totalExecutedTime`, `priority`
  - Why: Each process needs to track remaining CPU time and accumulated execution for the dynamic quantum formula. Priority (1-10) is required by the spec.
- **Added** `QUANTUM_EXPIRATION` event type (constant = 2)
  - Why: PA_2 only had ARRIVAL and DEPARTURE (FCFS). Round-Robin requires preemption when a quantum expires.
- **Refactored** `main()` — replaced hardcoded lambda loop with 5 command-line arguments
  - Why: PDF Section 2 requires args in order: λ, avgServiceTime, baseQuantum, A, B.
- **Added** interactive fallback mode (prompts via `cin` when no args provided)
  - Why: OnlineGDB supports command-line args but interactive mode is easier for quick testing.
- **Added** global constants: `MIN_QUANTUM` (0.001), `P_LOW` (10), `TOTAL_PROCESSES` (10000)
  - Why: Prevents negative quantum (per spec), and centralizes magic numbers.
- **Carried over** from PA_2: `EventQueue` class, `expRandom()`, `randomNumBetween0And1()`
  - Why: These work as-is for the new simulator. No changes needed.
- **Stubbed** `runSimulation()` — accepts all 5 parameters, body not yet implemented
  - Why: Placeholder for Part 2 implementation.
