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
    bool feedbackSent;
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
    feedbackSent = false;
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
    bufferSizeQueue.record(buffer.getLength());
    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            pkt->setKind(1);
            send(pkt, "out");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
        feedbackSent = false;
    } else { // if msg is a data packet
        const int umbral =  0.80 * par("bufferSize").intValue();
        const int umbralMin = 0.25 * par("bufferSize").intValue();        
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete(msg);
            this->bubble("packet-dropped");
            packetDrop++;
            packetDropQueue.record(packetDrop);
        }
        else { 
            if (buffer.getLength() >= umbral && !feedbackSent)
            {
                Feedbackpkt *feedbackPkt = new Feedbackpkt();
                feedbackPkt->setByteLength(20);
                feedbackPkt->setKind(2);
                send(feedbackPkt, "out");
                feedbackSent = true;
            }else if (buffer.getLength() < umbralMin)
            {
                Feedbackpkt *feedbackPkt = new Feedbackpkt();
                feedbackPkt->setByteLength(20);
                feedbackPkt->setKind(3);
                send(feedbackPkt, "out");
                //feedbackSent = true;
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
