#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

using namespace std;

extern "C"{
    #include "fake_receiver.h"
}

class CanHandler {
public:
    CanHandler(const string& filepath); // Constructor to initialize the CAN handler with the given file path
    ~CanHandler(); // Destructor to clean up resources

    bool start(); // Start the CAN handler and begin receiving messages
    void stop(); // Stop the CAN handler and clean up resources

    bool getMessage(string& out_msg); // Retrieve a message from the queue, returns true if a message was retrieved, false if the queue is empty

private:
    void receiveLoop(); // The main loop that runs in a separate thread to receive messages from the CAN bus and add them to the queue

    string m_filepath;
    thread m_receiverThread;
    
    queue<string> m_messageQueue;
    mutex m_mutex;
    condition_variable m_condVar;

    atomic<bool> m_running;
};

#endif