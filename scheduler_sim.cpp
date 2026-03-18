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


//run the discrete-event simulation
void runSimulation( double lambda, double avgServiceTime, double baseQuantum,
                    double scalingA, double scalingB )
{
    //TODO: implement simulation loop (Part 4 of planning doc)
    std::cout << "Simulation not yet implemented.\n";
}
