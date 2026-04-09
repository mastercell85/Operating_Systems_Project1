#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "Event.h"

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();

    // Non-copyable (owns raw pointers)
    EventQueue( const EventQueue& ) = delete;
    EventQueue& operator=( const EventQueue& ) = delete;

    void insert( Event e );
    Event getNextEvent();
    bool isEmpty() const;

private:
    EventNode* head;
};

#endif
