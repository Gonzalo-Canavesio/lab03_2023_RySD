#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include <omnetpp.h>
#include <string.h>

using namespace omnetpp;

class TransportRx: public cSimpleModule {
    private:
        cOutVector bufferSizeQueue;
        cOutVector packetDropQueue;
        cQueue buffer;
        cQueue feedbackBuffer;
        cMessage *endServiceEvent;
        cMessage *endFeedbackEvent;
        simtime_t serviceTime;
        bool feedbackSent;

    public:
        TransportRx();
        virtual ~TransportRx();
    protected:
        virtual void initialize();
        virtual void finish();
        virtual void handleMessage(cMessage *msg);

};

Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = NULL;
    endFeedbackEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
    cancelAndDelete(endFeedbackEvent);
}

void TransportRx::initialize() {
    buffer.setName("buffer");
    bufferSizeQueue.setName("BufferSizeQueue");
    packetDropQueue.setName("PacketDropQueue");
    feedbackBuffer.setName("bufferFeedback");
    packetDropQueue.record(0);
    endServiceEvent = new cMessage("endService");
    endFeedbackEvent = new cMessage("endFeedback");
    feedbackSent = false;
}

void TransportRx::finish() {
}


void TransportRx::handleMessage(cMessage *msg) {

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toApp");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else  if(msg == endFeedbackEvent){
        // the message is endFeedbackEvent
        if (!feedbackBuffer.isEmpty()) {
            // if the feedback buffer is not empty, send next one
            cPacket *pkt = (cPacket*) feedbackBuffer.pop();
                send(pkt, "toOut$o");
                scheduleAt(simTime() + pkt->getDuration(), endFeedbackEvent);
        }
    } else { // if msg is a data packet
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // drop the packet
            delete(msg);
            this->bubble("packet-dropped");
            packetDropQueue.record(1);
        } else {
            // enqueue the message
            if (msg->getKind() == 2 || msg->getKind() == 3){
                feedbackBuffer.insert(msg);

                if (!endFeedbackEvent->isScheduled()) {
                    // If there are no messages being sent, send this one now
                    scheduleAt(simTime() + 0, endFeedbackEvent);
                }
            } else {
                float umbral = 0.80 * par("bufferSize").intValue();
                float umbralMin = 0.25 * par("bufferSize").intValue();

                if (buffer.getLength() >= umbral && !feedbackSent){
                    cPacket *feedbackPkt = new cPacket("packet");
                    feedbackPkt->setByteLength(20);
                    feedbackPkt->setKind(2);
                    send(feedbackPkt, "toOut$o");
                    feedbackSent = true;
                }else if (buffer.getLength() < umbralMin && feedbackSent){
                    cPacket *feedbackPkt = new cPacket("packet");
                    feedbackPkt->setByteLength(20);
                    feedbackPkt->setKind(3);
                    send(feedbackPkt, "toOut$o");
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
}

#endif
