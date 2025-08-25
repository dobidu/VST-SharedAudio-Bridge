//
// Created by gferraz on 7/17/25.
//

#ifndef ZMQCOMMSERVER_H
#define ZMQCOMMSERVER_H

#include <string>
#include <vector>
#include <zmq.hpp>

typedef short unsigned int SmallMsg;

class ZMQCommServer {
private:
    zmq::context_t context;
    zmq::socket_t socket;
    zmq::message_t request{sizeof(SmallMsg)};
    zmq::message_t reply{sizeof(SmallMsg)};

    const std::string port{"5555"};
    const SmallMsg replySucessValue{1};

public:
    ZMQCommServer();
    int receiveBufferRequest();
    void sendBufferReadyResponse();
    void startServer();
    void closeServer();
};

#endif //ZMQCOMMSERVER_H
