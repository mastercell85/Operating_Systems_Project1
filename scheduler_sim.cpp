#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <limits>
#include <queue>
#include <map>
#include <fstream>
#include <string>


//event type constants
const int ARRIVAL = 0;
const int DEPARTURE = 1;
const int QUANTUM_EXPIRATION = 2;

//minimum quantum to prevent negative/zero values
const double MIN_QUANTUM = 0.001;

//lowest priority value (highest numerical)
const int P_LOW = 10;

//number of processes to simulate
const int TOTAL_PROCESSES = 10000;


//process struct - represents a single process in the system
struct Process
{
    int pid;
    double arrivalTime;
    double serviceTime;       //total CPU time needed (S_i)
    double remainingTime;     //CPU time still needed
    double totalExecutedTime; //T_i - accumulated CPU time used so far
    int priority;             //static priority 1-10 (1 = highest)

    Process() : pid( 0 ), arrivalTime( 0.0 ), serviceTime( 0.0 ),
                remainingTime( 0.0 ), totalExecutedTime( 0.0 ), priority( 1 ) {}

    Process( int id, double arrival, double service, int prio )
        : pid( id ), arrivalTime( arrival ), serviceTime( service ),
          remainingTime( service ), totalExecutedTime( 0.0 ), priority( prio ) {}
};


//event struct - represents a simulation event
struct Event
{
    double eventTime;
    int eventType;
    int processID;

    Event( double t, int type, int id ) : eventTime( t ), eventType( type ), processID( id ) {}
};


//event node for the linked-list event queue
struct EventNode
{
    Event event;
    EventNode* next;

    EventNode( Event e ) : event( e ), next( nullptr ) {}
};


//event queue - sorted linked-list priority queue ordered by event time
class EventQueue
{
    private:
        EventNode* head;

    public:
        EventQueue() : head( nullptr ) {}

    //insert event into the event queue in sorted order
    void insert( Event e )
    {
        EventNode* newNode = new EventNode( e );

        //if event queue is empty or new event is earliest
        if( head == nullptr || e.eventTime < head->event.eventTime )
        {
            newNode->next = head;
            head = newNode;
            return;
        }

        //find correct insert position
        EventNode* current = head;
        while( current->next != nullptr && current->next->event.eventTime < e.eventTime )
        {
            current = current->next;
        }

        newNode->next = current->next;
        current->next = newNode;
    }

    //get next event. removes it from the queue.
    Event getNextEvent()
    {
        if( head == nullptr )
        {
            throw std::runtime_error( "Event queue is empty!" );
        }

        EventNode* temp = head;
        Event nextEvent = head->event;
        head = head->next;
        delete temp;

        return nextEvent;
    }

    //check if the event queue is empty
    bool isEmpty()
    {
        return head == nullptr;
    }

    //destructor
    ~EventQueue()
    {
        while( head != nullptr )
        {
            EventNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};


//function prototypes
double expRandom( double lambda );
static inline double randomNumBetween0And1();
void runSimulation( double lambda, double avgServiceTime, double baseQuantum,
                    double scalingA, double scalingB );


int main( int argc, char* argv[] )
{
    //seed the random number generator
    std::srand( static_cast< unsigned int >( std::time( nullptr ) ) );

    double lambda, avgServiceTime, baseQuantum, scalingA, scalingB;

    if( argc == 6 )
    {
        //parse command-line arguments
        lambda         = std::atof( argv[1] );
        avgServiceTime = std::atof( argv[2] );
        baseQuantum    = std::atof( argv[3] );
        scalingA       = std::atof( argv[4] );
        scalingB       = std::atof( argv[5] );
    }
    else if( argc == 1 )
    {
        //interactive mode - prompt user for input
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

    //display simulation parameters
    std::cout << std::fixed << std::setprecision( 4 );
    std::cout << "========== Simulation Parameters ==========\n";
    std::cout << "Arrival Rate (lambda): " << lambda << "\n";
    std::cout << "Avg Service Time:      " << avgServiceTime << "\n";
    std::cout << "Base Quantum:          " << baseQuantum << "\n";
    std::cout << "Scaling Factor A:      " << scalingA << "\n";
    std::cout << "Scaling Factor B:      " << scalingB << "\n";
    std::cout << "Processes to simulate: " << TOTAL_PROCESSES << "\n";
    std::cout << "============================================\n\n";

    //run the simulation
    runSimulation( lambda, avgServiceTime, baseQuantum, scalingA, scalingB );

    return 0;
}


//uniformly distributed random number between 0 and 1
static inline double randomNumBetween0And1()
{
    return static_cast< double >( rand() ) / RAND_MAX;
}


//exponentially distributed random number with mean 1/lambda
double expRandom( double lambda )
{
    double U = randomNumBetween0And1();
    //prevent taking log of 0
    if( U <= 0.0 )
    {
        U = std::numeric_limits< double >::min();
    }

    return ( -std::log( U ) ) / lambda;
}


//calculate dynamic quantum for a process
double calculateQuantum( const Process& proc, double baseQuantum, double scalingA, double scalingB )
{
    double quantum = baseQuantum + ( P_LOW - proc.priority ) / scalingA - scalingB * proc.totalExecutedTime;

    //clamp to minimum to prevent negative/zero quantum
    if( quantum < MIN_QUANTUM )
    {
        quantum = MIN_QUANTUM;
    }

    return quantum;
}


//run the discrete-event simulation
void runSimulation( double lambda, double avgServiceTime, double baseQuantum,
                    double scalingA, double scalingB )
{
    //initialize event queue
    EventQueue eventQueue;

    //initialize ready queue (FIFO for round-robin)
    std::queue< int > readyQueue;

    //process table - look up any process by PID
    std::map< int, Process > processTable;

    //simulation state
    double clock = 0.0;
    double lastEventTime = 0.0;
    bool cpuBusy = false;
    int nextProcessID = 1;
    int completedProcesses = 0;
    int currentRunningPID = -1;       //PID of process currently on CPU
    double currentQuantumUsed = 0.0;  //tracks quantum assigned to current running process

    //metrics
    double totalTurnaroundTime = 0.0;
    int totalContextSwitches = 0;
    double areaUnderQueueCurve = 0.0;

    //per-priority turnaround tracking (for report graph)
    double priorityTurnaround[11] = { 0.0 };  //index 1-10
    int priorityCount[11] = { 0 };             //index 1-10

    //output formatting
    std::cout << std::fixed << std::setprecision( 4 );

    //schedule first arrival
    double firstArrivalTime = expRandom( lambda );
    eventQueue.insert( Event( firstArrivalTime, ARRIVAL, nextProcessID ) );
    nextProcessID++;

    //main simulation loop
    while( completedProcesses < TOTAL_PROCESSES )
    {
        //get next event
        Event currentEvent = eventQueue.getNextEvent();

        //calculate time since last event
        double deltaT = currentEvent.eventTime - lastEventTime;

        //update area under queue length curve
        areaUnderQueueCurve += readyQueue.size() * deltaT;

        //advance clock
        clock = currentEvent.eventTime;
        lastEventTime = clock;

        //handle event based on its type
        if( currentEvent.eventType == ARRIVAL )
        {
            //generate process attributes
            double serviceTime = expRandom( 1.0 / avgServiceTime );
            int priority = ( rand() % 10 ) + 1;

            //create process and store in process table
            Process newProc( currentEvent.processID, currentEvent.eventTime, serviceTime, priority );
            processTable[ currentEvent.processID ] = newProc;

            //schedule next arrival (only if we still need more processes)
            if( nextProcessID <= TOTAL_PROCESSES )
            {
                double interArrivalTime = expRandom( lambda );
                double nextArrivalTime = currentEvent.eventTime + interArrivalTime;
                eventQueue.insert( Event( nextArrivalTime, ARRIVAL, nextProcessID ) );
                nextProcessID++;
            }

            //check if cpu is idle
            if( !cpuBusy )
            {
                //dispatch this process to CPU
                cpuBusy = true;
                currentRunningPID = currentEvent.processID;
                totalContextSwitches++;

                //calculate dynamic quantum
                Process& proc = processTable[ currentEvent.processID ];
                double quantum = calculateQuantum( proc, baseQuantum, scalingA, scalingB );
                currentQuantumUsed = quantum;

                if( proc.remainingTime <= quantum )
                {
                    //process will finish within this quantum
                    double departureTime = clock + proc.remainingTime;
                    eventQueue.insert( Event( departureTime, DEPARTURE, currentEvent.processID ) );
                }
                else
                {
                    //process will be preempted
                    double preemptTime = clock + quantum;
                    eventQueue.insert( Event( preemptTime, QUANTUM_EXPIRATION, currentEvent.processID ) );
                }
            }
            else
            {
                //cpu is busy - add process to ready queue
                readyQueue.push( currentEvent.processID );
            }
        }
        else if( currentEvent.eventType == DEPARTURE )
        {
            //process has completed
            Process& proc = processTable[ currentEvent.processID ];

            //update execution time
            proc.totalExecutedTime += proc.remainingTime;
            proc.remainingTime = 0.0;

            //calculate turnaround time
            double turnaroundTime = clock - proc.arrivalTime;
            totalTurnaroundTime += turnaroundTime;

            //track per-priority turnaround
            priorityTurnaround[ proc.priority ] += turnaroundTime;
            priorityCount[ proc.priority ]++;

            //increment completed processes
            completedProcesses++;

            //remove process from table (no longer needed)
            processTable.erase( currentEvent.processID );

            //check for waiting processes in the ready queue
            if( !readyQueue.empty() )
            {
                //get the next process from the ready queue
                int nextProcID = readyQueue.front();
                readyQueue.pop();

                //dispatch next process
                currentRunningPID = nextProcID;
                totalContextSwitches++;

                Process& nextProc = processTable[ nextProcID ];
                double quantum = calculateQuantum( nextProc, baseQuantum, scalingA, scalingB );
                currentQuantumUsed = quantum;

                if( nextProc.remainingTime <= quantum )
                {
                    double departureTime = clock + nextProc.remainingTime;
                    eventQueue.insert( Event( departureTime, DEPARTURE, nextProcID ) );
                }
                else
                {
                    double preemptTime = clock + quantum;
                    eventQueue.insert( Event( preemptTime, QUANTUM_EXPIRATION, nextProcID ) );
                }
            }
            else
            {
                //no process waiting - cpu goes idle
                cpuBusy = false;
                currentRunningPID = -1;
            }
        }
        else if( currentEvent.eventType == QUANTUM_EXPIRATION )
        {
            //process was preempted - quantum expired
            Process& proc = processTable[ currentEvent.processID ];

            //update remaining time and total executed time
            double quantum = currentQuantumUsed;
            proc.remainingTime -= quantum;
            proc.totalExecutedTime += quantum;

            //push preempted process to back of ready queue
            readyQueue.push( currentEvent.processID );

            //dispatch next process from front of ready queue
            int nextProcID = readyQueue.front();
            readyQueue.pop();

            currentRunningPID = nextProcID;
            totalContextSwitches++;

            Process& nextProc = processTable[ nextProcID ];
            double quantumNext = calculateQuantum( nextProc, baseQuantum, scalingA, scalingB );
            currentQuantumUsed = quantumNext;

            if( nextProc.remainingTime <= quantumNext )
            {
                double departureTime = clock + nextProc.remainingTime;
                eventQueue.insert( Event( departureTime, DEPARTURE, nextProcID ) );
            }
            else
            {
                double preemptTime = clock + quantumNext;
                eventQueue.insert( Event( preemptTime, QUANTUM_EXPIRATION, nextProcID ) );
            }
        }
    }

    //calculate performance metrics
    double avgTurnaroundTime = totalTurnaroundTime / completedProcesses;
    double avgContextSwitches = static_cast< double >( totalContextSwitches ) / completedProcesses;
    double avgQueueLength = areaUnderQueueCurve / clock;

    //display results
    std::cout << "--- SIMULATION RESULTS ---\n";
    std::cout << "Simulation Time:           " << clock << " seconds\n";
    std::cout << "Completed Processes:       " << completedProcesses << "\n";
    std::cout << "Total Context Switches:    " << totalContextSwitches << "\n";
    std::cout << "\n--- PERFORMANCE METRICS ---\n";
    std::cout << "Avg Turnaround Time:       " << avgTurnaroundTime << " seconds\n";
    std::cout << "Avg Context Switches:      " << avgContextSwitches << " per process\n";
    std::cout << "Avg Ready Queue Length:    " << avgQueueLength << " processes\n";

    //display per-priority turnaround times
    std::cout << "\n--- TURNAROUND BY PRIORITY ---\n";
    for( int p = 1; p <= 10; p++ )
    {
        if( priorityCount[ p ] > 0 )
        {
            double avgTT = priorityTurnaround[ p ] / priorityCount[ p ];
            std::cout << "Priority " << std::setw( 2 ) << p
                      << ": Avg Turnaround = " << avgTT
                      << " seconds (" << priorityCount[ p ] << " processes)\n";
        }
    }
}
