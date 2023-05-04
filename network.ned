simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile double serviceTime; // sec
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}



//Envia
simple nodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        output out;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        queue: Queue {
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> queue.in;
        queue.out --> out;    
}
//recibe
simple nodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        queue: Queue {
            parameters:
                @display("p=75,190");
        }
    connections:
        in --> sink.in;
        //case study 1:
        queue.out --> {    datarate = 0.5mbps;    } --> sink.in;
        //case study 2:
        //queue.out --> {    datarate = 1mbps;    } --> sink.in;
}

network Network
{
    @display("bgl=2");
    submodules:
        nodeTx: nodeTx {
            @display("p=30,30");
        }
        queue1: Queue {
            @display("p=130,30");
        }
        queue2: Queue {
            @display("p=130,90");
        }
        nodeRx: nodeRx {
            @display("p=230,30");
        }    
    connections:
        nodeTx.out --> {datarate = 1mbps; delay = 100us;} -->queue.in;
        //case study 1:
        queue1.out --> {datarate = 1mbps; delay = 100us;} --> nodeRx.in;
        //case study 2:
        //queue1.out --> {datarate = 0.5mbps; delay = 100us;} --> nodeRx.in;

}
