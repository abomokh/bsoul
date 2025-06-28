#include "L3.h"
#include "L4.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdint>
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

l3_packet::l3_packet(const std::string& packet) : packet_data(packet) {
    // Split the packet by '|'
    std::vector<std::string> fields = split(packet, '|');
    if (fields.size() < 8) return;

    // Parse source IP
    size_t start = 0, end = 0, i = 0;
    std::string& src_ip_str = fields[0];
    while (i < IP_V4_SIZE && (end = src_ip_str.find('.', start)) != std::string::npos) {
        std::string octet_str = src_ip_str.substr(start, end - start);
        src_ip[i++] = safe_stoi(octet_str);
        start = end + 1;
    }
    if (i < IP_V4_SIZE && start < src_ip_str.size()) {
        std::string octet_str = src_ip_str.substr(start);
        src_ip[i++] = safe_stoi(octet_str);
    }

    // Parse destination IP
    start = 0; end = 0; i = 0;
    std::string& dst_ip_str = fields[1];
    while (i < IP_V4_SIZE && (end = dst_ip_str.find('.', start)) != std::string::npos) {
        std::string octet_str = dst_ip_str.substr(start, end - start);
        dst_ip[i++] = safe_stoi(octet_str);
        start = end + 1;
    }
    if (i < IP_V4_SIZE && start < dst_ip_str.size()) {
        std::string octet_str = dst_ip_str.substr(start);
        dst_ip[i++] = safe_stoi(octet_str);
    }

    // Parse other fields
    ttl = safe_stoul(fields[2]);
    checksum = safe_stoul(fields[3]);
    source_port = safe_stoul(fields[4]);
    dest_port = safe_stoul(fields[5]);
    address = safe_stoul(fields[6]);

    // Parse data (hex format with spaces)
    std::string& data_str = fields[7];
    start = 0; i = 0;
    while (i < PACKET_DATA_SIZE && start < data_str.size()) {
        while (start < data_str.size() && data_str[start] == ' ') ++start;
        if (start >= data_str.size()) break;
        end = data_str.find(' ', start);
        if (end == std::string::npos) end = data_str.size();
        std::string byte_str = data_str.substr(start, end - start);
        data[i++] = safe_stoi(byte_str, 16);
        start = end + 1;
    }
}

bool l3_packet::validate_packet(open_port_vec open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               uint8_t mac[MAC_SIZE]) {
    if (ttl <= 0) {
        return false;
    }
    uint32_t calculated_checksum = calculate_checksum();
    return calculated_checksum == checksum;
}

bool l3_packet::proccess_packet(open_port_vec &open_ports,
                               uint8_t ip[IP_V4_SIZE],
                               uint8_t mask,
                               memory_dest &dst) {
    bool source_local = is_local_network(src_ip, ip, mask);
    bool dest_local = is_local_network(dst_ip, ip, mask);
    
    if (source_local && dest_local) {
        return false;
    }
    
    bool dest_is_nic = true;
    for (int i = 0; i < IP_V4_SIZE; i++) {
        if (dst_ip[i] != ip[i]) {
            dest_is_nic = false;
            break;
        }
    }
    
    if (dest_is_nic) {
        std::string l4_data = std::to_string(source_port) + "|" + std::to_string(dest_port) + "|" + std::to_string(address) + "|";
        for (int i = 0; i < PACKET_DATA_SIZE; i++) {
            if (i > 0) l4_data += " ";
            char tmp[3];
            snprintf(tmp, sizeof(tmp), "%02x", data[i]);
            l4_data += tmp;
        }
        l4_packet l4_pkt(l4_data);
        if (!l4_pkt.validate_packet(open_ports, ip, mask, nullptr)) {
            return false;
        }
        return l4_pkt.proccess_packet(open_ports, ip, mask, dst);
    }
    
    ttl--;
    checksum = calculate_checksum();
    if (ttl == 0) {
        return false;
    }
    
    if (source_local && !dest_local) {
        for (int i = 0; i < IP_V4_SIZE; i++) {
            src_ip[i] = ip[i];
        }
        checksum = calculate_checksum();
        dst = common::TQ;
        return true;
    }
    
    if (!source_local && dest_local) {
        dst = common::RQ;
    } else {
        dst = common::TQ;
    }
    return true;
}

bool l3_packet::as_string(std::string &packet) {
    std::string result;
    for (int i = 0; i < IP_V4_SIZE; i++) {
        if (i > 0) result += ".";
        result += std::to_string(src_ip[i]);
    }
    result += "|";
    for (int i = 0; i < IP_V4_SIZE; i++) {
        if (i > 0) result += ".";
        result += std::to_string(dst_ip[i]);
    }
    result += "|";
    result += std::to_string(ttl) + "|" + std::to_string(checksum) + "|" + std::to_string(source_port) + "|" + std::to_string(dest_port) + "|" + std::to_string(address) + "|";
    for (int i = 0; i < PACKET_DATA_SIZE; i++) {
        if (i > 0) result += " ";
        char tmp[3];
        snprintf(tmp, sizeof(tmp), "%02x", data[i]);
        result += tmp;
    }
    packet = result;
    return true;
}

uint32_t l3_packet::calculate_checksum() {
    uint32_t sum = 0;
    for (int i = 0; i < IP_V4_SIZE; i++) {
        sum += src_ip[i];
    }
    for (int i = 0; i < IP_V4_SIZE; i++) {
        sum += dst_ip[i];
    }
    sum += (ttl >> 24) & 0xFF;
    sum += (ttl >> 16) & 0xFF;
    sum += (ttl >> 8) & 0xFF;
    sum += ttl & 0xFF;
    sum += (source_port >> 8) & 0xFF;
    sum += source_port & 0xFF;
    sum += (dest_port >> 8) & 0xFF;
    sum += dest_port & 0xFF;
    sum += (address >> 24) & 0xFF;
    sum += (address >> 16) & 0xFF;
    sum += (address >> 8) & 0xFF;
    sum += address & 0xFF;
    for (int i = 0; i < PACKET_DATA_SIZE; i++) {
        sum += data[i];
    }
    return sum;
}

bool l3_packet::is_local_network(uint8_t ip_addr[IP_V4_SIZE], uint8_t nic_ip[IP_V4_SIZE], uint8_t mask) {
    for (int i = 0; i < IP_V4_SIZE; i++) {
        uint8_t nic_network = nic_ip[i] & ((1 << 8) - 1);
        uint8_t addr_network = ip_addr[i] & ((1 << 8) - 1);
        
        if (i < mask / 8) {
            if (nic_network != addr_network) {
                return false;
            }
        } else if (i == mask / 8) {
            int remaining_bits = mask % 8;
            uint8_t mask_byte = ((1 << remaining_bits) - 1) << (8 - remaining_bits);
            if ((nic_network & mask_byte) != (addr_network & mask_byte)) {
                return false;
            }
        }
    }
    return true;
} 