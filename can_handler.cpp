#include "can_handler.h"
#include <iostream>

using namespace std;

CanHandler::CanHandler(const string& filepath) { // Constructor: initialize the CAN handler with the given file path
    m_filepath = filepath;
    m_running = false;
}

CanHandler::~CanHandler() { // Destructor: clean up resources
    stop();
}

bool CanHandler::start() {
    if (m_running) { // If the handler is already running, return true
        return true;
    }

    if (open_can(m_filepath.c_str()) != 0) { // opens the can file and If there was an error opening the CAN file, print an error message and return false
        cerr << "Error while opening the CAN fiel: " << m_filepath << endl;
        return false;
    }

    m_running = true;
    m_receiverThread = thread(&CanHandler::receiveLoop, this); // Start the receiver thread to run the receiveLoop function

    return true;

}

void CanHandler::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    m_condVar.notify_all(); // Notify the receiver thread to wake up and exit

    if (m_receiverThread.joinable()) {
        m_receiverThread.join();
    }

    close_can();
}

void CanHandler::receiveLoop() { 
    char buffer[MAX_CAN_MESSAGE_SIZE]; // Buffer to hold incoming CAN messages

    while (m_running) {
        int bytes_read = can_receive(buffer);

        if (bytes_read > 0){
            string msg(buffer, bytes_read);
            // cout << "Received CAN message: " << msg << endl;
            lock_guard<mutex> lock(m_mutex);   // Lock the mutex to safely access the message queue
            m_messageQueue.push(msg); // Add the received message to the queue
            m_condVar.notify_one(); // Notify one waiting thread that a new message is available
        }
    }
}

bool CanHandler::getMessage(string& out_msg) {
    unique_lock<mutex> lock(m_mutex); // Lock the mutex to safely access the message queue

    m_condVar.wait(lock, [this]() { return !m_messageQueue.empty() || !m_running; }); // Wait until there is a message in the queue or the handler is stopped

    if (!m_running && m_messageQueue.empty()) { 
        return false;
    }
    
    out_msg = m_messageQueue.front(); // Get the front message from the queue
    m_messageQueue.pop(); // Remove the message from the queue
    
    return true;
}