/**
 * @file L4.cpp
 * @brief Implementation of the L4 packet class for the NIC simulation project.
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include "L4.h"

l4_packet::l4_packet(const std::string& packet_str) : packet_data(packet_str) {
    parse_packet();
}

void l4_packet::parse_packet() {
    // Format: src_port|dst_port|index|data_bytes
    size_t start = 0, end = 0;
    std::string fields[4];
    for (int i = 0; i < 3; ++i) {
        end = packet_data.find('|', start);
        if (end == std::string::npos) {
            std::cerr << "Error: Not enough fields in L4 packet: " << packet_data << std::endl;
            return;
        }
        fields[i] = packet_data.substr(start, end - start);
        start = end + 1;
    }
    fields[3] = packet_data.substr(start);
    
    // src_port
    try {
        src_port = static_cast<uint16_t>(std::stoi(fields[0]));
    } catch (...) {
        std::cerr << "Error: Invalid src_port: " << fields[0] << std::endl;
        return;
    }
    
    // dst_port
    try {
        dst_port = static_cast<uint16_t>(std::stoi(fields[1]));
    } catch (...) {
        std::cerr << "Error: Invalid dst_port: " << fields[1] << std::endl;
        return;
    }
    
    // index
    try {
        index = static_cast<uint16_t>(std::stoi(fields[2]));
    } catch (...) {
        std::cerr << "Error: Invalid index: " << fields[2] << std::endl;
        return;
    }
    
    // data
    data = fields[3];
}

bool l4_packet::validate_packet(open_port_vec open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               uint8_t mac[MAC_SIZE]) {
    // Check if there's a matching open port (communication exists)
    int port_index = find_open_port(open_ports);
    if (port_index == -1) {
        // No open channel exists - throw the packet
        return false;
    }
    
    // Validate index is within bounds
    if (index >= DATA_ARR_SIZE) {
        // Invalid index - kill the packet
        return false;
    }
    
    // Validate data format (should be hex bytes separated by spaces)
    if (data.empty()) {
        return false;
    }
    
    // Check if data contains valid hex bytes
    size_t start = 0, end = 0;
    int byte_count = 0;
    while (start < data.size() && byte_count < DATA_ARR_SIZE) {
        while (start < data.size() && data[start] == ' ') ++start;
        if (start >= data.size()) break;
        
        end = data.find(' ', start);
        std::string hex_byte = (end == std::string::npos) ? data.substr(start) : data.substr(start, end - start);
        
        if (!hex_byte.empty()) {
            try {
                std::stoi(hex_byte, nullptr, 16);
                byte_count++;
            } catch (...) {
                return false;
            }
        }
        
        if (end == std::string::npos) break;
        start = end + 1;
    }
    
    return true;
}

int l4_packet::find_open_port(const open_port_vec& open_ports) {
    for (size_t i = 0; i < open_ports.size(); i++) {
        if (open_ports[i].src_prt == src_port && open_ports[i].dst_prt == dst_port) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool l4_packet::proccess_packet(open_port_vec &open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               memory_dest &dst) {
    // Find the matching open port
    int port_index = find_open_port(open_ports);
    if (port_index == -1) {
        // No open channel exists - throw the packet
        return false;
    }
    
    // Validate index is within bounds
    if (index >= DATA_ARR_SIZE) {
        // Invalid index - kill the packet
        return false;
    }
    
    // Store content in data[] of open_port starting at the index position
    // Parse the data string and store it in the open_port struct
    size_t start = 0, end = 0;
    int data_index = index; // Start storing at the index position
    
    while (start < data.size() && data_index < DATA_ARR_SIZE) {
        while (start < data.size() && data[start] == ' ') ++start;
        if (start >= data.size()) break;
        
        end = data.find(' ', start);
        std::string hex_byte = (end == std::string::npos) ? data.substr(start) : data.substr(start, end - start);
        
        if (!hex_byte.empty()) {
            try {
                open_ports[port_index].data[data_index++] = static_cast<unsigned char>(std::stoi(hex_byte, nullptr, 16));
            } catch (...) {
                return false;
            }
        }
        
        if (end == std::string::npos) break;
        start = end + 1;
    }
    
    // Set destination to LOCAL_DRAM since we stored data in open_port
    dst = LOCAL_DRAM;
    return true;
}

bool l4_packet::as_string(std::string &packet) {
    packet = std::to_string(src_port) + "|" + std::to_string(dst_port) + "|" + std::to_string(index) + "|" + data;
    return true;
} 