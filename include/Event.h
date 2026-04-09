#ifndef EVENT_H
#define EVENT_H

struct Event
{
    double eventTime;
    int eventType;
    int processID;

    Event( double t, int type, int id )
        : eventTime( t ), eventType( type ), processID( id ) {}
};

struct EventNode
{
    Event event;
    EventNode* next;

    EventNode( Event e ) : event( e ), next( nullptr ) {}
};

#endif
