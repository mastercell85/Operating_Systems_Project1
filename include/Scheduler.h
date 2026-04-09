#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "EventQueue.h"
#include "Process.h"
#include "QuantumStrategy.h"
#include <queue>
#include <map>

struct SimulationResult
{
    double simulationTime;
    int completedProcesses;
    int totalContextSwitches;
    double avgTurnaroundTime;
    double avgContextSwitchesPerProcess;
    double avgReadyQueueLength;
    double priorityTurnaround[11];  // index 1-10
    int priorityCount[11];          // index 1-10
};

class Scheduler
{
public:
    Scheduler( double lambda, double avgServiceTime,
               const QuantumStrategy& strategy );

    SimulationResult run();

private:
    // Simulation parameters
    double lambda_;
    double avgServiceTime_;
    const QuantumStrategy& strategy_;

    // Simulation state
    EventQueue eventQueue_;
    std::queue< int > readyQueue_;
    std::map< int, Process > processTable_;

    double clock_;
    double lastEventTime_;
    bool cpuBusy_;
    int nextProcessID_;
    int completedProcesses_;
    int currentRunningPID_;
    double currentQuantumUsed_;

    // Metrics
    double totalTurnaroundTime_;
    int totalContextSwitches_;
    double areaUnderQueueCurve_;
    double priorityTurnaround_[11];
    int priorityCount_[11];

    // Event handlers
    void handleArrival( const Event& e );
    void handleDeparture( const Event& e );
    void handleQuantumExpiration( const Event& e );

    // Dispatches a process from the ready queue onto the CPU
    void dispatchProcess( int pid );

    // Exponentially distributed random variate
    double expRandom( double rate );
};

#endif
