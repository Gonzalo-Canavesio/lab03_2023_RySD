#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include <omnetpp.h>
#include <string.h>

#include "FeedbackPkt_m.h"

using namespace omnetpp;

class TransportRx: public cSimpleModule {
    private:
        cQueue buffer;
        cQueue feedbackBuffer;
        cMessage *endServiceEvent;
        cMessage *endFeedbackEvent;
        cOutVector bufferSizeVec;
        cOutVector pktDrop;
        int pktDropCount;

        void sendPkt();
        void sendFeedback();
        void enqueueFeedback(cMessage *msg);
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
    buffer.setName("Receptor buffer");
    feedbackBuffer.setName("Feedback buffer");    
    bufferSizeVec.setName("BufferSize");
    pktDrop.setName("Packet Drop");
    pktDropCount = 0;
    endServiceEvent = new cMessage("endService");
    endFeedbackEvent = new cMessage("endFeedback");
}

void TransportRx::finish() {
    recordScalar("PacketDrop", pktDropCount);
}

void TransportRx::sendPkt() {
    if (!buffer.isEmpty()) {
        // if packet in buffer, send next one
            cPacket *pkt = (cPacket*) buffer.pop();
            send(pkt, "toApp");
            scheduleAt(simTime() + pkt->getDuration(), endServiceEvent);
    }
}

void TransportRx::enqueueFeedback(cMessage *msg){
    if (feedbackBuffer.getLength() < par("feedbackBufferSize").intValue()) {
            feedbackBuffer.insert(msg);
            if (!endFeedbackEvent->isScheduled()) {
                scheduleAt(simTime() + 0, endFeedbackEvent);
            }
        } else {
            delete msg;
        }
}

void TransportRx::sendFeedback() {
    if (!feedbackBuffer.isEmpty()) {
        // if the feedback buffer is not empty, send next one
            FeedbackPkt *pkt = (FeedbackPkt*) feedbackBuffer.pop();
            send(pkt, "toOut$o");
            scheduleAt(simTime() + pkt->getDuration(), endFeedbackEvent);
    }
}

void TransportRx::handleMessage(cMessage *msg) {
    bufferSizeVec.record(buffer.getLength());
    if (msg == endServiceEvent) {
        // the message is endServiceEvent
        sendPkt();    
    } else if (msg == endFeedbackEvent) {
        // the message is endFeedbackEvent
        sendFeedback();    
    } else {
        // enqueue the message
        if (strcmp(msg->getName(), "packet") == 0) {
            const int umbral = 0.70 * par("bufferSize").intValue();
            if (buffer.getLength() < par("bufferSize").intValue()) {
                if (buffer.getLength() >= umbral){
                    FeedbackPkt *fPkt = new FeedbackPkt();
                    fPkt->setName("feedback");
                    fPkt->setByteLength(1);
                    fPkt->setFullBufferR(true);
                    enqueueFeedback(fPkt);                
                }
                buffer.insert(msg);
                if (!endServiceEvent->isScheduled()) {
                    scheduleAt(simTime() + 0, endServiceEvent)
                }
            } else {
                this->bubble("Packet dropped");
                pktDropCount++;
                pktDrop.record(pktDropCount);
                delete msg;
            }
        } else if (strcmp(msg->getName(), "feedback") == 0) {
            // the message is a feedback packet
            enqueueFeedback(msg);
        }
    }
}