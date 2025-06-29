#include "L2.h"
#include "L3.h"
#include <iostream>
#include <iomanip>
#include <vector>

// Helper function to split a string by delimiter
static std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

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

l2_packet::l2_packet(const std::string& packet) : packet_data(packet) {
    // Split the packet by '|'
    std::vector<std::string> fields = split(packet, '|');
    if (fields.size() < 4) return;

    // Parse source MAC
    size_t start = 0, end = 0, i = 0;
    std::string& src_mac_str = fields[0];
    while (i < MAC_SIZE && (end = src_mac_str.find(':', start)) != std::string::npos) {
        std::string byte_str = src_mac_str.substr(start, end - start);
        src_mac[i++] = safe_stoi(byte_str, 16);
        start = end + 1;
    }
    if (i < MAC_SIZE && start < src_mac_str.size()) {
        std::string byte_str = src_mac_str.substr(start);
        src_mac[i++] = safe_stoi(byte_str, 16);
    }

    // Parse destination MAC
    start = 0; end = 0; i = 0;
    std::string& dst_mac_str = fields[1];
    while (i < MAC_SIZE && (end = dst_mac_str.find(':', start)) != std::string::npos) {
        std::string byte_str = dst_mac_str.substr(start, end - start);
        dst_mac[i++] = safe_stoi(byte_str, 16);
        start = end + 1;
    }
    if (i < MAC_SIZE && start < dst_mac_str.size()) {
        std::string byte_str = dst_mac_str.substr(start);
        dst_mac[i++] = safe_stoi(byte_str, 16);
    }

    // The last field is the checksum
    checksum = safe_stoul(fields.back(), 16);

    // The L3 packet string is the fields from index 2 to (size-2), joined by '|'
    l3_packet_data.clear();
    for (size_t j = 2; j < fields.size() - 1; ++j) {
        if (j > 2) l3_packet_data += "|";
        l3_packet_data += fields[j];
    }
}

bool l2_packet::validate_packet(open_port_vec open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               uint8_t mac[MAC_SIZE]) {
    // L2 validation: check if destination MAC matches NIC's MAC
    for (int i = 0; i < MAC_SIZE; i++) {
        if (dst_mac[i] != mac[i]) {
            return false;
        }
    }
    
    // Calculate checksum of the packet (excluding checksum field)
    uint32_t calculated_checksum = 0;
    for (int i = 0; i < MAC_SIZE; i++) calculated_checksum += src_mac[i];
    for (int i = 0; i < MAC_SIZE; i++) calculated_checksum += dst_mac[i];
    for (size_t i = 0; i < l3_packet_data.size(); ++i) calculated_checksum += static_cast<unsigned char>(l3_packet_data[i]);
    
    return calculated_checksum == checksum;
}

bool l2_packet::proccess_packet(open_port_vec &open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               memory_dest &dst) {
    // L2 processing: extract L3 packet and process it
    if (!l3_packet_data.empty()) {
        l3_packet l3_pkt(l3_packet_data);
        if (l3_pkt.validate_packet(open_ports, ip, mask, nullptr)) {
            return l3_pkt.proccess_packet(open_ports, ip, mask, dst);
        }
    }
    return false;
}

bool l2_packet::as_string(std::string &packet) {
    // L2 packets should not be output directly - they should be converted to L3
    // This function should not be called for L2 packets in TQ/RQ
    return false;
} 