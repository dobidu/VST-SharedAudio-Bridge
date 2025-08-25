#include "ZMQCommServer.h"

#include <iostream>

ZMQCommServer::ZMQCommServer()
{}

int ZMQCommServer::receiveBufferRequest()
{
    auto recvResult = socket.recv(request, zmq::recv_flags::none);

    auto msg = *(static_cast<SmallMsg *>(request.data()));
    return msg;
}

void ZMQCommServer::sendBufferReadyResponse()
{
    memcpy(reply.data(), &replySucessValue, sizeof(SmallMsg));
    auto sendResult = socket.send(reply, zmq::send_flags::none);
}

void ZMQCommServer::closeServer()
{
    socket.close();
    context.close();
}

void ZMQCommServer::startServer()
{
    context = zmq::context_t(1);
    socket = zmq::socket_t(context, zmq::socket_type::rep);
    socket.set(zmq::sockopt::linger, 0);
    socket.bind("tcp://*:" + port);
}
