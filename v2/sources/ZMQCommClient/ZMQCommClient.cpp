#include "ZMQCommClient.h"

ZMQCommClient::ZMQCommClient() : context(1), socket(context, zmq::socket_type::req)
{
    socket.set(zmq::sockopt::linger, 0);
    socket.connect("tcp://localhost:5555");
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
    memcpy(request.data(), &bufferRequest, sizeof(SmallMsg));
    auto sendResult = socket.send(request, zmq::send_flags::dontwait);

    zmq::poller_t<> poller;
    poller.add(socket, zmq::event_flags::pollin);
    const auto poll_events = poller.wait_all(events, timeout);
    if (!poll_events)
    {
        return false;
    }

    auto recvResult = socket.recv(reply, zmq::recv_flags::none);
    auto msg = *(static_cast<SmallMsg*>(request.data()));
    if (msg > 0)
    {
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
}
