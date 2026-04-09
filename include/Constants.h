#ifndef CONSTANTS_H
#define CONSTANTS_H

// Event type constants
constexpr int ARRIVAL            = 0;
constexpr int DEPARTURE          = 1;
constexpr int QUANTUM_EXPIRATION = 2;

// Minimum quantum to prevent negative/zero values
constexpr double MIN_QUANTUM = 0.001;

// Lowest priority value (highest numerical)
constexpr int P_LOW = 10;

// Number of processes to simulate
constexpr int TOTAL_PROCESSES = 10000;

#endif
