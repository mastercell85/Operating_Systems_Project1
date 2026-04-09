#include "EventQueue.h"
#include <stdexcept>

EventQueue::EventQueue() : head( nullptr ) {}

EventQueue::~EventQueue()
{
    while( head != nullptr )
    {
        EventNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void EventQueue::insert( Event e )
{
    EventNode* newNode = new EventNode( e );

    if( head == nullptr || e.eventTime < head->event.eventTime )
    {
        newNode->next = head;
        head = newNode;
        return;
    }

    EventNode* current = head;
    while( current->next != nullptr && current->next->event.eventTime < e.eventTime )
    {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

Event EventQueue::getNextEvent()
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

bool EventQueue::isEmpty() const
{
    return head == nullptr;
}
