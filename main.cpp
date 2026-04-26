#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <map>

#include "can_handler.h"
#include "parser.h"

using namespace std;

namespace fs = filesystem;

enum class State { // State machine states
    IDLE,
    RUN
};

struct MessageStats { // Structure to hold statistics for each message ID
    int count = 0;
};

void update_stat(map<uint16_t, MessageStats>& stats, uint16_t id); // Function prototype for updating message statistics

void save_csv(const map<uint16_t, MessageStats>& stats, int session, double duration); // Function prototype for saving statistics to a CSV file

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

    if (!fs::exists(log_dir)) { // Create logs directory if it doesn't exist
        fs::create_directory(log_dir);
        cout << "Created log directory: " << log_dir << endl;
    }

    State current_state = State::IDLE;
    string raw_msg;
    ofstream log_file; 
    int session_counter = 0;

    map<uint16_t, MessageStats> statistics_map; // To track statistics for each message ID during a session
    auto session_start_time = chrono::system_clock::now();  // To track the start time of each session for statistics calculation

    cout << "[IDLE] Waitng for start..." << endl;

    while (handler.getMessage(raw_msg)) {
        
        ParsedMessage parsed_msg;
        
        if (!parseCanMessage(raw_msg, parsed_msg)) {
            continue; 
        }

        if (current_state == State::IDLE) { // Waiting for start condition

            if (start_condition_met(parsed_msg)) { 
                current_state = State::RUN;
                session_counter++;
                statistics_map.clear();
                session_start_time = chrono::system_clock::now();
                string filename = "../" + log_dir + "/log_session_" + to_string(session_counter) + ".txt";
                log_file.open(filename);
                
                if (log_file.is_open()) {
                    cout << "[START] session stared. Saving on: " << filename << endl;
                } else {
                    cerr << "Error opening log file: " << filename << endl;
                    exit(2);
                }

            }

        } else if (current_state == State::RUN) {

            if (stop_condition_met(parsed_msg)) { // Stop condition met: close file and return to IDLE
                current_state = State::IDLE;
                auto now = chrono::system_clock::now();
                chrono::duration<double> elapsed = now - session_start_time;
                log_file.close();
                save_csv(statistics_map, session_counter, elapsed.count());
                cout << "[STOP] session finished. Returning in wait condition..." << endl;
            } else if (log_file.is_open()) { 
                auto timestamp = chrono::system_clock::now().time_since_epoch().count(); // Get current timestamp for logging
                if (log_file.is_open()) {
                    log_file << "(" << timestamp << ") " << raw_msg << endl;
                }
                update_stat(statistics_map, parsed_msg.id);   
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

void update_stat(map<uint16_t, MessageStats>& stats, uint16_t id) {
    stats[id].count++;
}

void save_csv(const map<uint16_t, MessageStats>& stats, int session, double duration_sec) {
    string filename = "../logs/stats_session_" + to_string(session) + ".csv";
    ofstream csv(filename);
    
    csv << "ID,number_of_messages,mean_time" << endl;
    
    for (auto const& [id, data] : stats) {

        double mean_time = (data.count > 0) ? ((duration_sec * 1000.0) / data.count) : 0; // Mean time in milliseconds per message
        
        char hex_id[10]; // Buffer to hold the hexadecimal representation of the ID
        sprintf(hex_id, "%03X", id); 
        
        csv << hex_id << "," << data.count << "," << mean_time << endl; // Write ID in hexadecimal format, count, and mean time to CSV
    }
    
    csv.close();
    cout << "[CSV] saved stats in: " << filename << endl;
}