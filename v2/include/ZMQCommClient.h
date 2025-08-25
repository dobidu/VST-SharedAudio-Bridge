#ifndef ZMQCOMM_H
#define ZMQCOMM_H

#include <string>
#include <chrono>
#include <array>

#include <zmq.hpp>

typedef short unsigned int SmallMsg;

class ZMQCommClient {
private:
    zmq::context_t context;
    zmq::socket_t socket;
    std::array<zmq::pollitem_t, 1> pollItems;

    // zmq::message_t request{sizeof(SmallMsg)};
    zmq::message_t reply{sizeof(SmallMsg)};

    zmq::const_buffer requestBuffer{};


    const std::chrono::milliseconds timeout{2};
    bool isReady;
    const std::string port = "5555";

public:
    ZMQCommClient();
    ~ZMQCommClient();
    bool requestAudioBlock(int bufferSize);
    void refreshConnection();
};

#endif //ZMQCOMM_H
