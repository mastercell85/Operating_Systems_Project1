#ifndef QUANTUM_STRATEGY_H
#define QUANTUM_STRATEGY_H

#include "Process.h"

// Abstract interface for quantum calculation (Strategy pattern)
class QuantumStrategy
{
public:
    virtual ~QuantumStrategy() = default;
    virtual double calculate( const Process& proc ) const = 0;
};

// Dynamic quantum: q_i = base + (P_low - P_i) / A - B * T_i
class DynamicQuantumStrategy : public QuantumStrategy
{
public:
    DynamicQuantumStrategy( double baseQuantum, double scalingA, double scalingB );
    double calculate( const Process& proc ) const override;

private:
    double baseQuantum_;
    double scalingA_;
    double scalingB_;
};

#endif
