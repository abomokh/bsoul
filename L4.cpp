#include "L4.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdint>

l4_packet::l4_packet(const std::string& packet) : packet_data(packet) {
    // Parse port numbers, address, and data from the packet string
    // Format: src_port|dst_port|address|data
    size_t pipe1 = packet.find('|');
    size_t pipe2 = packet.find('|', pipe1 + 1);
    size_t pipe3 = packet.find('|', pipe2 + 1);
    
    if (pipe1 != std::string::npos && pipe2 != std::string::npos && pipe3 != std::string::npos) {
        std::string src_port_str = packet.substr(0, pipe1);
        std::string dst_port_str = packet.substr(pipe1 + 1, pipe2 - pipe1 - 1);
        std::string address_str = packet.substr(pipe2 + 1, pipe3 - pipe2 - 1);
        data = packet.substr(pipe3 + 1);
        
        src_port = std::stoi(src_port_str);
        dst_port = std::stoi(dst_port_str);
        address = std::stoi(address_str);
    }
}

bool l4_packet::validate_packet(open_port_vec open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               uint8_t mac[MAC_SIZE]) {
    // L4 validation: check if the port combination exists in open_ports
    for (const auto& port : open_ports) {
        if (port.src_prt == src_port && port.dst_prt == dst_port) {
            return true;
        }
    }
    return false;
}

bool l4_packet::proccess_packet(open_port_vec &open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               memory_dest &dst) {
    // First validate the packet (check if communication exists)
    if (!validate_packet(open_ports, ip, mask, nullptr)) {
        return false;
    }
    
    // Find the matching open port and store data
    for (auto& port : open_ports) {
        if (port.src_prt == src_port && port.dst_prt == dst_port) {
            // Check if address is valid (within bounds)
            if (address >= DATA_ARR_SIZE) {
                return false; // Invalid address
            }
            
            // Parse hex data and store starting from address
            size_t data_pos = 0;
            size_t start = 0;
            while (data_pos < PACKET_DATA_SIZE && start < data.size()) {
                // Skip spaces
                while (start < data.size() && data[start] == ' ') start++;
                if (start >= data.size()) break;
                
                // Find next space or end
                size_t end = data.find(' ', start);
                if (end == std::string::npos) end = data.size();
                
                // Parse hex byte
                std::string byte_str = data.substr(start, end - start);
                if (address + data_pos < DATA_ARR_SIZE) {
                    port.data[address + data_pos] = std::stoi(byte_str, nullptr, 16);
                }
                data_pos++;
                start = end + 1;
            }
            dst = common::LOCAL_DRAM;
            return true;
        }
    }
    return false;
}

bool l4_packet::as_string(std::string &packet) {
    // Convert L4 packet to string format
    packet = std::to_string(src_port) + "|" + std::to_string(dst_port) + "|" + std::to_string(address) + "|" + data;
    return true;
}

bool l4_packet::communication_exists(const open_port_vec& open_ports, uint16_t src_port, uint16_t dst_port) {
    for (const auto& port : open_ports) {
        if (port.src_prt == src_port && port.dst_prt == dst_port) {
            return true;
        }
    }
    return false;
} 