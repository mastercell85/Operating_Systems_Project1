#include "Constants.h"
#include "QuantumStrategy.h"
#include "Scheduler.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

void printResults( const SimulationResult& r );

int main( int argc, char* argv[] )
{
    std::srand( static_cast< unsigned int >( std::time( nullptr ) ) );

    double lambda, avgServiceTime, baseQuantum, scalingA, scalingB;

    if( argc == 6 )
    {
        lambda         = std::atof( argv[1] );
        avgServiceTime = std::atof( argv[2] );
        baseQuantum    = std::atof( argv[3] );
        scalingA       = std::atof( argv[4] );
        scalingB       = std::atof( argv[5] );
    }
    else if( argc == 1 )
    {
        std::cout << "No command-line arguments detected. Entering interactive mode.\n\n";
        std::cout << "Enter average arrival rate (lambda): ";
        std::cin >> lambda;
        std::cout << "Enter average CPU service time: ";
        std::cin >> avgServiceTime;
        std::cout << "Enter base quantum: ";
        std::cin >> baseQuantum;
        std::cout << "Enter scaling factor A: ";
        std::cin >> scalingA;
        std::cout << "Enter scaling factor B: ";
        std::cin >> scalingB;
        std::cout << std::endl;
    }
    else
    {
        std::cerr << "Usage: ./scheduler_sim <lambda> <avgServiceTime> <baseQuantum> <A> <B>\n";
        std::cerr << "   or: ./scheduler_sim   (interactive mode)\n";
        return 1;
    }

    // Display simulation parameters
    std::cout << std::fixed << std::setprecision( 4 );
    std::cout << "========== Simulation Parameters ==========\n";
    std::cout << "Arrival Rate (lambda): " << lambda << "\n";
    std::cout << "Avg Service Time:      " << avgServiceTime << "\n";
    std::cout << "Base Quantum:          " << baseQuantum << "\n";
    std::cout << "Scaling Factor A:      " << scalingA << "\n";
    std::cout << "Scaling Factor B:      " << scalingB << "\n";
    std::cout << "Processes to simulate: " << TOTAL_PROCESSES << "\n";
    std::cout << "============================================\n\n";

    // Wire up and run
    DynamicQuantumStrategy strategy( baseQuantum, scalingA, scalingB );
    Scheduler scheduler( lambda, avgServiceTime, strategy );
    SimulationResult result = scheduler.run();

    printResults( result );
    return 0;
}


void printResults( const SimulationResult& r )
{
    std::cout << std::fixed << std::setprecision( 4 );
    std::cout << "--- SIMULATION RESULTS ---\n";
    std::cout << "Simulation Time:           " << r.simulationTime << " seconds\n";
    std::cout << "Completed Processes:       " << r.completedProcesses << "\n";
    std::cout << "Total Context Switches:    " << r.totalContextSwitches << "\n";
    std::cout << "\n--- PERFORMANCE METRICS ---\n";
    std::cout << "Avg Turnaround Time:       " << r.avgTurnaroundTime << " seconds\n";
    std::cout << "Avg Context Switches:      " << r.avgContextSwitchesPerProcess << " per process\n";
    std::cout << "Avg Ready Queue Length:    " << r.avgReadyQueueLength << " processes\n";

    std::cout << "\n--- TURNAROUND BY PRIORITY ---\n";
    for( int p = 1; p <= 10; p++ )
    {
        if( r.priorityCount[p] > 0 )
        {
            double avgTT = r.priorityTurnaround[p] / r.priorityCount[p];
            std::cout << "Priority " << std::setw( 2 ) << p
                      << ": Avg Turnaround = " << avgTT
                      << " seconds (" << r.priorityCount[p] << " processes)\n";
        }
    }
}
