#include <string>
#include <vector>
#include <iostream>
#include "parser.h"

using namespace std;

bool parseCanMessage(const string& raw_msg, ParsedMessage& parsed_msg) {
    
    string id_str, payload_str;
    string delimiter = "#";

    size_t delimiter_pos = raw_msg.find(delimiter);

    if (delimiter_pos == string::npos) { // delimiter_pos == string::npos(no position)  if delimeter not found and it prevents something like !0 from happpening by using !raw_msg.find(delimiter)
        cerr << "Invalid message format: missing delimiter '#'" << endl;
        return false;
    } 

    id_str = raw_msg.substr(0, delimiter_pos);
    payload_str = raw_msg.substr(delimiter_pos + 1);

    parsed_msg.id = stoi(id_str, nullptr, 16); // Convert string to uint16_t

    if (payload_str.length() % 2 != 0 || payload_str.length() > 16) {
        cerr << "Invalid payload format: payload length should be even" << endl;
        return false;
    }

    for (size_t i = 0; i < payload_str.length(); i += 2) {
        string byte_str = payload_str.substr(i, 2);
        uint8_t byte = stoi(byte_str, nullptr, 16); // Convert hex string to uint8_t
        parsed_msg.payload.push_back(byte);
    }

    return true;
}