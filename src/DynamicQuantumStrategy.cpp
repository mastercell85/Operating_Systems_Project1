#include "QuantumStrategy.h"
#include "Constants.h"

DynamicQuantumStrategy::DynamicQuantumStrategy(
    double baseQuantum, double scalingA, double scalingB )
    : baseQuantum_( baseQuantum ), scalingA_( scalingA ), scalingB_( scalingB )
{}

double DynamicQuantumStrategy::calculate( const Process& proc ) const
{
    double quantum = baseQuantum_
                   + ( P_LOW - proc.priority ) / scalingA_
                   - scalingB_ * proc.totalExecutedTime;

    return ( quantum < MIN_QUANTUM ) ? MIN_QUANTUM : quantum;
}
