#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    cOutVector bufferSizeQueue;
    cOutVector packetDropQueue;
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    void send();
public:
    Queue();
    virtual ~Queue();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = NULL;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    bufferSizeQueue.setName("bufferSizeQueue");
    packetDropQueue.setName("packetDropQueue");
    packetDropQueue.record(0);
    endServiceEvent = new cMessage("endService");
}

void Queue::send(){
    if (!buffer.isEmpty()) {
            // dequeue packet
            cMessage *pkt = (cMessage*) buffer.pop();
            // send packet
            send(pkt, "out");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTimed() + serviceTime, endServiceEvent);
    }
}
void Queue::finish() {
}

void Queue::handleMessage(cMessage *msg) {

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        send();
    } else { // if msg is a data packet
        if (buffer.getLength() >= par("bufferSize").longValue()){
            //drop the packet
            delete msg;
            this->bubble("Packet dropped");
            packetDropQueue.record(1);
        } else{ 
            // enqueue the packet
            buffer.insert(msg);
            bufferSizeQueue.record(buffer.getLength());
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime() + 0, endServiceEvent);
            }
        }
    }
}

#endif /* QUEUE */
