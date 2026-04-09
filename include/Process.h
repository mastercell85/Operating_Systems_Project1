#ifndef PROCESS_H
#define PROCESS_H

struct Process
{
    int pid;
    double arrivalTime;
    double serviceTime;
    double remainingTime;
    double totalExecutedTime;
    int priority;

    Process()
        : pid( 0 ), arrivalTime( 0.0 ), serviceTime( 0.0 ),
          remainingTime( 0.0 ), totalExecutedTime( 0.0 ), priority( 1 ) {}

    Process( int id, double arrival, double service, int prio )
        : pid( id ), arrivalTime( arrival ), serviceTime( service ),
          remainingTime( service ), totalExecutedTime( 0.0 ), priority( prio ) {}
};

#endif
