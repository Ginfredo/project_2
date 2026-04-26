#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>

#include "can_handler.h"
#include "parser.h"

using namespace std;

namespace fs = filesystem;

enum class State { // State machine states
    IDLE,
    RUN
};

bool start_condition_met(const ParsedMessage& msg);

bool stop_condition_met(const ParsedMessage& msg);

int main() {
    cout << "Start..." << endl;

    CanHandler handler("../candump.log"); 
    
    if (!handler.start()) {
        cerr << "Error while starting CanHandler!" << endl;
        exit(1);
    }


    string log_dir = "logs";

    if (!fs::exists(log_dir)) {
        fs::create_directory(log_dir);
        cout << "Created log directory: " << log_dir << endl;
    }

    State current_state = State::IDLE;
    string raw_msg;
    ofstream log_file; 
    int session_counter = 1;

    cout << "[IDLE] Waitng for start..." << endl;

    while (handler.getMessage(raw_msg)) {
        
        ParsedMessage parsed_msg;
        
        if (!parseCanMessage(raw_msg, parsed_msg)) {
            continue; 
        }

        if (current_state == State::IDLE) { // Waiting for start condition

            if (start_condition_met(parsed_msg)) { 
                current_state = State::RUN;
                string filename = "../" + log_dir + "/log_session_" + to_string(session_counter) + ".txt";
                log_file.open(filename);
                
                if (log_file.is_open()) {
                    cout << "[START] session stared. Saving on: " << filename << endl;
                } else {
                    cerr << "Error opening log file: " << filename << endl;
                    exit(2);
                }

            }

        } 
        else if (current_state == State::RUN) {

            if (stop_condition_met(parsed_msg)) { // Stop condition met: close file and return to IDLE
                  current_state = State::IDLE;
                  log_file.close();
                  cout << "[STOP] session finished. Returning in wait condition..." << endl;
            } else if (log_file.is_open()) { 
                  auto now = chrono::system_clock::now();
                  auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
                  log_file << timestamp << " " << raw_msg << endl;
                
            }
        
        }
    }

    if (log_file.is_open()) {
        log_file.close();
    }
    
    handler.stop();
    cout << "system ended correctly." << endl;
    return 0;
}

bool start_condition_met(const ParsedMessage& msg) {
    return (msg.id == 0x0A0 && msg.payload.size() == 2 && 
               (msg.payload[0] == 0x66 || msg.payload[0] == 0xFF) && 
                msg.payload[1] == 0x01);
}

bool stop_condition_met(const ParsedMessage& msg) {
    return (msg.id == 0x0A0 && msg.payload.size() == 2 && 
                msg.payload[0] == 0x66 && msg.payload[1] == 0xFF);
}