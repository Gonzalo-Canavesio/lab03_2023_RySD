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
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    bufferSizeQueue.setName("BufferSizeQueue");
    packetDropQueue.setName("PacketDropQueue");
    packetDropQueue.record(0);
    endServiceEvent = new cMessage("endService");
    feedbackSent = false;
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
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete(msg);
            this->bubble("packet-dropped");
            packetDropQueue.record(1);
        }
        else {
            const int umbral =  0.80 * par("bufferSize").intValue();
            const int umbralMin = 0.25 * par("bufferSize").intValue();    

            if (buffer.getLength() >= umbral && !feedbackSent){
                cPacket *feedbackPkt = new cPacket("packet");
                feedbackPkt->setByteLength(20);
                feedbackPkt->setKind(2);
                buffer.insertBefore(buffer.front(), feedbackPkt);
                feedbackSent = true;
            }else if (buffer.getLength() < umbralMin && feedbackSent)
            {
                cPacket *feedbackPkt = new cPacket("packet");
                feedbackPkt->setByteLength(20);
                feedbackPkt->setKind(3);
                buffer.insertBefore(buffer.front(), feedbackPkt);
                feedbackSent = false;
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
