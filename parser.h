#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <cstdint> 

using namespace std;

struct ParsedMessage {
    uint16_t id; // CAN message ID
    vector<uint8_t> payload; // Payload data as a vector of bytes
};

bool parseCanMessage(const string& raw_msg, ParsedMessage& parsed_msg); // Parses a raw CAN message string and fills the ParsedMessage structure. Returns true if parsing is successful, false otherwise.

#endif