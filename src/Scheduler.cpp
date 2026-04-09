#include "Scheduler.h"
#include "Constants.h"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <cstring>

Scheduler::Scheduler( double lambda, double avgServiceTime,
                      const QuantumStrategy& strategy )
    : lambda_( lambda ),
      avgServiceTime_( avgServiceTime ),
      strategy_( strategy ),
      clock_( 0.0 ),
      lastEventTime_( 0.0 ),
      cpuBusy_( false ),
      nextProcessID_( 1 ),
      completedProcesses_( 0 ),
      currentRunningPID_( -1 ),
      currentQuantumUsed_( 0.0 ),
      totalTurnaroundTime_( 0.0 ),
      totalContextSwitches_( 0 ),
      areaUnderQueueCurve_( 0.0 )
{
    std::memset( priorityTurnaround_, 0, sizeof( priorityTurnaround_ ) );
    std::memset( priorityCount_, 0, sizeof( priorityCount_ ) );
}


SimulationResult Scheduler::run()
{
    // Schedule first arrival
    double firstArrival = expRandom( lambda_ );
    eventQueue_.insert( Event( firstArrival, ARRIVAL, nextProcessID_++ ) );

    // Main simulation loop
    while( completedProcesses_ < TOTAL_PROCESSES )
    {
        Event e = eventQueue_.getNextEvent();

        double deltaT = e.eventTime - lastEventTime_;
        areaUnderQueueCurve_ += readyQueue_.size() * deltaT;

        clock_ = e.eventTime;
        lastEventTime_ = clock_;

        switch( e.eventType )
        {
            case ARRIVAL:            handleArrival( e );            break;
            case DEPARTURE:          handleDeparture( e );          break;
            case QUANTUM_EXPIRATION: handleQuantumExpiration( e );  break;
        }
    }

    // Build results
    SimulationResult result{};
    result.simulationTime              = clock_;
    result.completedProcesses          = completedProcesses_;
    result.totalContextSwitches        = totalContextSwitches_;
    result.avgTurnaroundTime           = totalTurnaroundTime_ / completedProcesses_;
    result.avgContextSwitchesPerProcess = static_cast< double >( totalContextSwitches_ ) / completedProcesses_;
    result.avgReadyQueueLength         = areaUnderQueueCurve_ / clock_;

    for( int p = 0; p <= 10; p++ )
    {
        result.priorityTurnaround[p] = priorityTurnaround_[p];
        result.priorityCount[p]      = priorityCount_[p];
    }

    return result;
}


void Scheduler::handleArrival( const Event& e )
{
    // Generate process attributes
    double serviceTime = expRandom( 1.0 / avgServiceTime_ );
    int priority = ( rand() % 10 ) + 1;

    // Create process and store in process table
    Process newProc( e.processID, e.eventTime, serviceTime, priority );
    processTable_[ e.processID ] = newProc;

    // Schedule next arrival if we still need more processes
    if( nextProcessID_ <= TOTAL_PROCESSES )
    {
        double interArrivalTime = expRandom( lambda_ );
        double nextArrivalTime = e.eventTime + interArrivalTime;
        eventQueue_.insert( Event( nextArrivalTime, ARRIVAL, nextProcessID_++ ) );
    }

    // Dispatch to CPU if idle, otherwise enqueue
    if( !cpuBusy_ )
    {
        dispatchProcess( e.processID );
    }
    else
    {
        readyQueue_.push( e.processID );
    }
}


void Scheduler::handleDeparture( const Event& e )
{
    Process& proc = processTable_[ e.processID ];

    // Update execution time
    proc.totalExecutedTime += proc.remainingTime;
    proc.remainingTime = 0.0;

    // Calculate turnaround time
    double turnaroundTime = clock_ - proc.arrivalTime;
    totalTurnaroundTime_ += turnaroundTime;

    // Track per-priority turnaround
    priorityTurnaround_[ proc.priority ] += turnaroundTime;
    priorityCount_[ proc.priority ]++;

    completedProcesses_++;
    processTable_.erase( e.processID );

    // Dispatch next waiting process or go idle
    if( !readyQueue_.empty() )
    {
        int nextPID = readyQueue_.front();
        readyQueue_.pop();
        dispatchProcess( nextPID );
    }
    else
    {
        cpuBusy_ = false;
        currentRunningPID_ = -1;
    }
}


void Scheduler::handleQuantumExpiration( const Event& e )
{
    Process& proc = processTable_[ e.processID ];

    // Update remaining and executed time
    proc.remainingTime -= currentQuantumUsed_;
    proc.totalExecutedTime += currentQuantumUsed_;

    // Preempted process goes to back of ready queue
    readyQueue_.push( e.processID );

    // Dispatch next process from front of ready queue
    int nextPID = readyQueue_.front();
    readyQueue_.pop();
    dispatchProcess( nextPID );
}


void Scheduler::dispatchProcess( int pid )
{
    currentRunningPID_ = pid;
    cpuBusy_ = true;
    totalContextSwitches_++;

    Process& proc = processTable_[ pid ];
    double quantum = strategy_.calculate( proc );
    currentQuantumUsed_ = quantum;

    if( proc.remainingTime <= quantum )
    {
        eventQueue_.insert( Event( clock_ + proc.remainingTime, DEPARTURE, pid ) );
    }
    else
    {
        eventQueue_.insert( Event( clock_ + quantum, QUANTUM_EXPIRATION, pid ) );
    }
}


double Scheduler::expRandom( double rate )
{
    double U = static_cast< double >( rand() ) / RAND_MAX;

    if( U <= 0.0 )
    {
        U = std::numeric_limits< double >::min();
    }

    return ( -std::log( U ) ) / rate;
}
