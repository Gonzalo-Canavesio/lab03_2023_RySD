#ifndef GENERATOR
#define GENERATOR

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Generator : public cSimpleModule {
private:
    cMessage *sendMsgEvent;
    cStdDev transmissionStats;
    cOutVector packetGenVector;
    int packetSent;
public:
    Generator();
    virtual ~Generator();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};
Define_Module(Generator);

Generator::Generator() {
    sendMsgEvent = NULL;

}

Generator::~Generator() {
    cancelAndDelete(sendMsgEvent);
}

void Generator::initialize() {
    packetGenVector.setName("PacketGen");
    transmissionStats.setName("TotalTransmissions");
    packetSent = 0;
    // create the send packet
    sendMsgEvent = new cMessage("sendEvent");
    // schedule the first event at random time
    scheduleAt(par("generationInterval"), sendMsgEvent);
}

void Generator::finish() {
    recordScalar("TotalTransmissions", transmissionStats.getSum());
    recordScalar("sentPackets", packetSent);
}

void Generator::handleMessage(cMessage *msg) {

    // create new packet
    cPacket *pkt = new cPacket("packet");
    // record the packet generation
    packetSent++;
    packetGenVector.record(packetSent);
    pkt->setByteLength(par("packetByteSize"));
    // record the packet transmission
    transmissionStats.collect(pkt->getByteLength());
    // send to the output
    send(pkt, "out");

    // compute the new departure time
    simtime_t departureTime = simTime() + par("generationInterval");
    // schedule the new packet generation
    scheduleAt(departureTime, sendMsgEvent);
}

#endif /* GENERATOR */
