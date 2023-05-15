#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    int packetDrop;
    cOutVector bufferSizeQueue;
    cOutVector packetDropQueue;
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
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
    packetDrop = 0;
    bufferSizeQueue.setName("BufferSizeQueue");
    packetDropQueue.setName("PacketDropQueue");
    packetDropQueue.record(0);
    endServiceEvent = new cMessage("endService");
}

void Queue::finish() {
}

void Queue::handleMessage(cMessage *msg) {

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "out");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else { // if msg is a data packet
        const int umbral =  0.80 * par("bufferSize").intValue();        
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete(msg);
            this->bubble("packet-dropped");
            packetDrop++;
            packetDropQueue.record(packetDrop);
        }
        else { 
            if (buffer.getLength() >= umbral)
            {
                Feedbackpkt *feedbackPkt = new Feedbackpkt();
                feedbackPkt->setByteLength(20);
                feedbackPkt->setKind(2);
                feedbackPkt->setReaminingBufferSize(par("bufferSize").longValue() - buffer.getLength());
                send(feedbackPkt, "out");
            }            
            // Enqueue the packet
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
