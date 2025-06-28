/**
 * @file NIC_sim.cpp
 * @brief Implementation of the NIC simulator class for the NIC simulation project.
 */

#include "NIC_sim.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

nic_sim::nic_sim(std::string param_file) {
    // Read NIC parameters from file
    std::ifstream file(param_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open parameter file: " << param_file << std::endl;
        return;
    }
    
    std::string line;
    
    // Read MAC address
    if (std::getline(file, line)) {
        std::istringstream mac_iss(line);
        std::string mac_part;
        int i = 0;
        size_t start = 0, end = 0;
        for (i = 0; i < MAC_SIZE; ++i) {
            end = line.find(':', start);
            mac_part = (end == std::string::npos) ? line.substr(start) : line.substr(start, end - start);
            try {
                mac[i] = static_cast<uint8_t>(std::stoi(mac_part, nullptr, 16));
            } catch (...) {
                std::cerr << "Error: Invalid MAC part: " << mac_part << std::endl;
                return;
            }
            if (end == std::string::npos) break;
            start = end + 1;
        }
        if (i != MAC_SIZE - 1) {
            std::cerr << "Error: MAC address does not have 6 parts: " << line << std::endl;
            return;
        }
    }
    
    // Read IP and mask
    if (std::getline(file, line)) {
        size_t slash_pos = line.find('/');
        if (slash_pos != std::string::npos) {
            std::string ip_str = line.substr(0, slash_pos);
            std::string mask_str = line.substr(slash_pos + 1);
            // Parse IP
            size_t start = 0, end = 0;
            for (int i = 0; i < IP_V4_SIZE; ++i) {
                end = ip_str.find('.', start);
                std::string ip_part = (end == std::string::npos) ? ip_str.substr(start) : ip_str.substr(start, end - start);
                try {
                    nic_ip[i] = static_cast<uint8_t>(std::stoi(ip_part));
                } catch (...) {
                    std::cerr << "Error: Invalid IP part: " << ip_part << std::endl;
                    return;
                }
                if (end == std::string::npos) break;
                start = end + 1;
            }
            try {
                mask = static_cast<uint8_t>(std::stoi(mask_str));
            } catch (...) {
                std::cerr << "Error: Invalid mask: " << mask_str << std::endl;
                return;
            }
        } else {
            std::cerr << "Error: IP/mask line missing '/': " << line << std::endl;
            return;
        }
    }
    // Read open communications
    while (std::getline(file, line)) {
        if (line.find("src_prt:") != std::string::npos && line.find("dst_port:") != std::string::npos) {
            size_t src_pos = line.find("src_prt:");
            size_t src_end = line.find(",", src_pos);
            std::string src_str = line.substr(src_pos + 8, src_end - src_pos - 8);
            size_t dst_pos = line.find("dst_port:");
            std::string dst_str = line.substr(dst_pos + 9);
            unsigned short src_port = 0, dst_port = 0;
            try {
                src_port = static_cast<unsigned short>(std::stoi(src_str));
            } catch (...) {
                std::cerr << "Error: Invalid src_port: " << src_str << std::endl;
                continue;
            }
            try {
                dst_port = static_cast<unsigned short>(std::stoi(dst_str));
            } catch (...) {
                std::cerr << "Error: Invalid dst_port: " << dst_str << std::endl;
                continue;
            }
            open_ports.push_back(open_port(dst_port, src_port));
        }
    }
    file.close();
}

void nic_sim::nic_flow(std::string packet_file) {
    std::ifstream file(packet_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open packet file: " << packet_file << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            // Create packet using factory
            generic_packet* packet = packet_factory(line);
            if (packet != nullptr) {
                // Validate packet
                if (packet->validate_packet(open_ports, nic_ip, mask, mac)) {
                    // Process packet
                    memory_dest dst;
                    if (packet->proccess_packet(open_ports, nic_ip, mask, dst)) {
                        // Store packet in appropriate location
                        std::string packet_str;
                        if (packet->as_string(packet_str)) {
                            switch (dst) {
                                case common::RQ:
                                    RQ.push_back(packet_str);
                                    break;
                                case common::TQ:
                                    TQ.push_back(packet_str);
                                    break;
                                case common::LOCAL_DRAM:
                                    break;
                            }
                        }
                    }
                }
                delete packet;
            }
        }
    }
    
    file.close();
}

void nic_sim::nic_print_results() {
    // Print LOCAL DRAM
    std::cout << "LOCAL DRAM:" << std::endl;
    for (const auto& port : open_ports) {
        std::cout << port.src_prt << " " << port.dst_prt << ": ";
        for (int i = 0; i < DATA_ARR_SIZE; i++) {
            if (i > 0) std::cout << " ";
            port.print_hex_byte(i);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Print RQ
    std::cout << "RQ:" << std::endl;
    for (const auto& packet : RQ) {
        std::cout << packet << std::endl;
    }
    std::cout << std::endl;
    
    // Print TQ
    std::cout << "TQ:" << std::endl;
    for (const auto& packet : TQ) {
        std::cout << packet << std::endl;
    }
}

nic_sim::~nic_sim() {
    // Destructor - vectors will be automatically cleaned up
}

generic_packet* nic_sim::packet_factory(std::string &packet) {
    // Determine packet type based on format
    // Count the number of '|' delimiters to determine layer
    
    int pipe_count = 0;
    for (char c : packet) {
        if (c == '|') pipe_count++;
    }
    
    // L3 packets have IP addresses (format: src_ip|dst_ip|ttl|checksum|dst_port|src_port|data)
    // Check if the first two parts contain dots (IP addresses)
    size_t first_pipe = packet.find('|');
    if (first_pipe != std::string::npos) {
        std::string first_part = packet.substr(0, first_pipe);
        size_t second_pipe = packet.find('|', first_pipe + 1);
        if (second_pipe != std::string::npos) {
            std::string second_part = packet.substr(first_pipe + 1, second_pipe - first_pipe - 1);
            
            // If both parts contain dots, it's likely an L3 packet
            if (first_part.find('.') != std::string::npos && second_part.find('.') != std::string::npos) {
                return new l3_packet(packet);
            }
        }
    }
    
    // L2 packets have MAC addresses (format: src_mac|dst_mac|...|checksum)
    // Check if the first two parts contain colons (MAC addresses)
    if (first_pipe != std::string::npos) {
        std::string first_part = packet.substr(0, first_pipe);
        size_t second_pipe = packet.find('|', first_pipe + 1);
        if (second_pipe != std::string::npos) {
            std::string second_part = packet.substr(first_pipe + 1, second_pipe - first_pipe - 1);
            
            // If both parts contain colons, it's likely an L2 packet
            if (first_part.find(':') != std::string::npos && second_part.find(':') != std::string::npos) {
                return new l2_packet(packet);
            }
        }
    }
    
    // L4 packets have ports (format: index|src_port|dest_port|data_bytes)
    if (pipe_count >= 3) {
        return new l4_packet(packet);
    }
    
    return nullptr;
} 