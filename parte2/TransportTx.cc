#ifndef TRANSPORT_TX
#define TRANSPORT_TX

#include <omnetpp.h>
#include <string.h>

using namespace omnetpp;

class TransportTx : public cSimpleModule {
    private:
        cOutVector bufferSizeQueue;
        cOutVector packetDropQueue;
        cQueue buffer;
        cMessage *endServiceEvent;
        simtime_t serviceTime;
        float packetRate;
    public:
        TransportTx();
        virtual ~TransportTx();
    protected:
        virtual void initialize();
        virtual void finish();
        virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTx);

TransportTx::TransportTx() {
    endServiceEvent = NULL;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize() {
    buffer.setName("buffer");
    bufferSizeQueue.setName("BufferSizeQueue");
    packetDropQueue.setName("PacketDropQueue");
    packetDropQueue.record(0);
    endServiceEvent = new cMessage("endService");
    packetRate = 1.0;
}

void TransportTx::finish() {
}

void TransportTx::handleMessage(cMessage *msg) {
    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toOut$o");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime*packetRate, endServiceEvent);
        }
    } else { // if msg is a data packet
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete(msg);
            this->bubble("packet-dropped");
            packetDropQueue.record(1);
        } else {
            // enqueue the message
            if (msg->getKind() == 2) {
                packetRate = packetRate*2;
            } else if (msg->getKind() == 3){
                packetRate = packetRate/2;
            } else {
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
}

#endif
