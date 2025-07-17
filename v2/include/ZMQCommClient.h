#ifndef ZMQCOMM_H
#define ZMQCOMM_H

#include <string>
#include <chrono>
#include <vector>

#include <zmq.hpp>

typedef short unsigned int SmallMsg;

class ZMQCommClient {
private:
    zmq::context_t context;
    zmq::socket_t socket;
    std::vector<zmq::poller_event<>> events{1};
    zmq::message_t request{sizeof(SmallMsg)};
    zmq::message_t reply{sizeof(SmallMsg)};


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
