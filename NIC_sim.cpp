#include "NIC_sim.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>

// Safe string to integer conversion
static int safe_stoi(const std::string& s, int base = 10) {
    try {
        return std::stoi(s, nullptr, base);
    } catch (const std::exception& e) {
        return 0;
    }
}

// Safe string to unsigned long conversion
static unsigned long safe_stoul(const std::string& s, int base = 10) {
    try {
        return std::stoul(s, nullptr, base);
    } catch (const std::exception& e) {
        return 0;
    }
}

// Global variables to store NIC parameters (since we can't modify the class)
static uint8_t g_mac[MAC_SIZE];
static uint8_t g_ip[IP_V4_SIZE];
static uint8_t g_mask;

nic_sim::nic_sim(std::string param_file) {
    std::ifstream file(param_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open parameter file: " + param_file);
    }

    std::string line;
    
    // Read MAC address (first line)
    if (std::getline(file, line)) {
        size_t start = 0, end = 0, i = 0;
        while (i < MAC_SIZE && (end = line.find(':', start)) != std::string::npos) {
            std::string byte_str = line.substr(start, end - start);
            g_mac[i++] = safe_stoi(byte_str, 16);
            start = end + 1;
        }
        if (i < MAC_SIZE && start < line.size()) {
            std::string byte_str = line.substr(start);
            g_mac[i++] = safe_stoi(byte_str, 16);
        }
    }

    // Read IP address and mask (second line)
    if (std::getline(file, line)) {
        size_t slash_pos = line.find('/');
        if (slash_pos != std::string::npos) {
            std::string ip_str = line.substr(0, slash_pos);
            std::string mask_str = line.substr(slash_pos + 1);
            
            // Parse IP address
            size_t start = 0, end = 0, i = 0;
            while (i < IP_V4_SIZE && (end = ip_str.find('.', start)) != std::string::npos) {
                std::string octet_str = ip_str.substr(start, end - start);
                g_ip[i++] = safe_stoi(octet_str);
                start = end + 1;
            }
            if (i < IP_V4_SIZE && start < ip_str.size()) {
                std::string octet_str = ip_str.substr(start);
                g_ip[i++] = safe_stoi(octet_str);
            }
            
            // Parse mask
            g_mask = safe_stoi(mask_str);
        }
    }

    // Read open connections (remaining lines)
    while (std::getline(file, line)) {
        size_t comma_pos = line.find(',');
        if (comma_pos != std::string::npos) {
            std::string src_part = line.substr(0, comma_pos);
            std::string dst_part = line.substr(comma_pos + 1);
            
            // Parse source port
            size_t src_colon = src_part.find(':');
            std::string src_port_str = src_part.substr(src_colon + 1);
            uint16_t src_port = safe_stoi(src_port_str);
            
            // Parse destination port
            size_t dst_colon = dst_part.find(':');
            std::string dst_port_str = dst_part.substr(dst_colon + 1);
            uint16_t dst_port = safe_stoi(dst_port_str);
            
            open_ports.emplace_back(dst_port, src_port);
        }
    }
}

void nic_sim::nic_flow(std::string packet_file) {
    std::ifstream file(packet_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open packet file: " + packet_file);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            generic_packet* packet = packet_factory(line);
            if (packet != nullptr) {
                memory_dest dst;
                if (packet->proccess_packet(open_ports, g_ip, g_mask, dst)) {
                    std::string packet_str;
                    if (packet->as_string(packet_str)) {
                        if (dst == common::RQ) {
                            RQ.push_back(packet_str);
                        } else if (dst == common::TQ) {
                            TQ.push_back(packet_str);
                        }
                        // LOCAL_DRAM is handled directly in the packet processing
                    } else {
                        // Handle L2 packets that don't have as_string but need to be converted to L3
                        l2_packet* l2_pkt = dynamic_cast<l2_packet*>(packet);
                        if (l2_pkt != nullptr) {
                            // Create L3 packet from L2 data for output
                            l3_packet l3_pkt(l2_pkt->get_l3_data());
                            if (l3_pkt.as_string(packet_str)) {
                                if (dst == common::RQ) {
                                    RQ.push_back(packet_str);
                                } else if (dst == common::TQ) {
                                    TQ.push_back(packet_str);
                                }
                            }
                        }
                    }
                }
                delete packet;
            }
        }
    }
}

void nic_sim::nic_print_results() {
    // Check if any port has non-zero data
    bool has_data = false;
    for (const auto& port : open_ports) {
        for (int i = 0; i < DATA_ARR_SIZE; i++) {
            if (port.data[i] != 0) {
                has_data = true;
                break;
            }
        }
        if (has_data) break;
    }
    
    // Only print LOCAL DRAM if there's actual data
    if (has_data) {
        std::cout << "LOCAL DRAM:" << std::endl;
        for (const auto& port : open_ports) {
            std::cout << port.src_prt << " " << port.dst_prt << ": ";
            for (int i = 0; i < DATA_ARR_SIZE; i++) {
                // Create a non-const reference to work around the const issue
                const_cast<common::open_port&>(port).print_hex_byte(i);
                if (i < DATA_ARR_SIZE - 1) {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "RQ:" << std::endl;
    for (const auto& packet : RQ) {
        std::cout << packet << std::endl;
    }
    std::cout << std::endl;

    std::cout << "TQ:" << std::endl;
    for (const auto& packet : TQ) {
        std::cout << packet << std::endl;
    }
}

nic_sim::~nic_sim() {
    // No dynamic memory to clean up
}

generic_packet* nic_sim::packet_factory(std::string &packet) {
    // Determine packet type based on format
    // L2 packets have MAC addresses with colons AND IP addresses with dots
    if (packet.find(':') != std::string::npos && packet.find('.') != std::string::npos) {
        return new l2_packet(packet);
    }
    // L3 packets have IP addresses with dots and pipes, but no colons
    else if (packet.find('.') != std::string::npos && packet.find('|') != std::string::npos && packet.find(':') == std::string::npos) {
        return new l3_packet(packet);
    }
    // L4 packets have pipes but no dots or colons
    else if (packet.find('|') != std::string::npos && packet.find('.') == std::string::npos && packet.find(':') == std::string::npos) {
        return new l4_packet(packet);
    }
    
    return nullptr;
} 