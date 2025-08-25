#include "ZMQCommClient.h"

#include <iostream>

ZMQCommClient::ZMQCommClient() : context(1), socket(context, zmq::socket_type::req)
{
    socket.set(zmq::sockopt::linger, 0);
    socket.connect("tcp://localhost:5555");
    pollItems[0] = zmq::pollitem_t{socket, 0, ZMQ_POLLIN, 0};
}

ZMQCommClient::~ZMQCommClient()
{
    socket.disconnect("tcp://localhost:" + port);
    socket.close();
    context.close();
}

bool ZMQCommClient::requestAudioBlock(int bufferSize)
{
    auto bufferRequest = static_cast<SmallMsg>(bufferSize);
    requestBuffer = zmq::buffer(&bufferRequest, sizeof(SmallMsg));
    auto sendResult = socket.send(requestBuffer, zmq::send_flags::dontwait);

    zmq::poll(&pollItems[0], 1, timeout);
    if (pollItems[0].revents & ZMQ_POLLIN)
    {
        auto recvResult = socket.recv(reply, zmq::recv_flags::none);
        return true;
    }
    return false;
}

void ZMQCommClient::refreshConnection()
{
    socket.close();
    socket = zmq::socket_t(context, zmq::socket_type::req);
    socket.set(zmq::sockopt::linger, 0);
    socket.connect("tcp://localhost:5555");
    pollItems[0] = zmq::pollitem_t{socket, 0, ZMQ_POLLIN, 0};
}
