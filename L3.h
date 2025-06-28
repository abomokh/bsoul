/**
 * @file L3.h
 * @brief This header defines the L3 packet class for the NIC simulation project.
 *
 * L3 packets represent the Network layer packets that include source and destination
 * IP addresses, TTL, checksum, and port information.
 */

#ifndef __L3__
#define __L3__

#include <cstdint>
#include "packets.hpp"

class l3_packet : public generic_packet {
public:
    /**
     * @fn l3_packet
     * @brief Constructor for L3 packet.
     * 
     * @param packet_str - String representation of the L3 packet.
     *
     * @return New L3 packet object.
     */
    l3_packet(const std::string& packet_str);

    /**
     * @fn validate_packet
     * @brief Validates the L3 packet by checking TTL and checksum.
     *
     * @param open_ports - Vector containing all the NIC's open ports.
     * @param ip - NIC's IP address.
     * @param mask - NIC's mask.
     * @param mac - NIC's MAC address.
     *
     * @return true if the packet is valid, false otherwise.
     */
    bool validate_packet(open_port_vec open_ports,
                        uint8_t ip[IP_V4_SIZE],
                        uint8_t mask,
                        uint8_t mac[MAC_SIZE]) override;

    /**
     * @fn proccess_packet
     * @brief Processes the L3 packet based on routing logic.
     *
     * @param open_ports - Vector containing all the NIC's open ports.
     * @param ip - NIC's IP address.
     * @param mask - NIC's mask.
     * @param dst - Reference to memory destination enum.
     *
     * @return true on success, false on failure.
     */
    bool proccess_packet(open_port_vec &open_ports,
                        uint8_t ip[IP_V4_SIZE],
                        uint8_t mask,
                        memory_dest &dst) override;

    /**
     * @fn as_string
     * @brief Converts the L3 packet to string format.
     *
     * @param packet - Output string representation.
     *
     * @return true on success, false on failure.
     */
    bool as_string(std::string &packet) override;

private:
    std::string packet_data;
    uint8_t src_ip[IP_V4_SIZE];
    uint8_t dst_ip[IP_V4_SIZE];
    uint8_t ttl;
    uint16_t checksum;
    uint16_t dst_port;
    uint16_t src_port;
    std::string l4_data;

    /**
     * @fn parse_packet
     * @brief Parses the packet string to extract L3 fields.
     */
    void parse_packet();

    /**
     * @fn validate_checksum
     * @brief Validates the packet checksum.
     *
     * @return true if checksum is valid, false otherwise.
     */
    bool validate_checksum();

    /**
     * @fn is_local_network
     * @brief Checks if the destination IP is in the local network.
     *
     * @param nic_ip - NIC's IP address.
     * @param mask - NIC's subnet mask.
     *
     * @return true if destination is in local network, false otherwise.
     */
    bool is_local_network(uint8_t nic_ip[IP_V4_SIZE], uint8_t mask);

    /**
     * @fn is_targeted_to_nic
     * @brief Checks if the packet is targeted to this NIC.
     *
     * @param nic_ip - NIC's IP address.
     *
     * @return true if packet is targeted to NIC, false otherwise.
     */
    bool is_targeted_to_nic(uint8_t nic_ip[IP_V4_SIZE]);
};

#endif 