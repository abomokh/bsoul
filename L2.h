/**
 * @file L2.h
 * @brief This header defines the L2 packet class for the NIC simulation project.
 *
 * L2 packets represent the MAC layer packets that include source and destination
 * MAC addresses and checksum validation.
 */

#ifndef __L2__
#define __L2__

#include <cstdint>
#include "packets.hpp"

class l2_packet : public generic_packet {
public:
    /**
     * @fn l2_packet
     * @brief Constructor for L2 packet.
     * 
     * @param packet_str - String representation of the L2 packet.
     *
     * @return New L2 packet object.
     */
    l2_packet(const std::string& packet_str);

    /**
     * @fn validate_packet
     * @brief Validates the L2 packet by checking destination MAC and checksum.
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
     * @brief Processes the L2 packet by stripping L2 headers and passing to L3.
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
     * @brief Converts the L2 packet to string format.
     *
     * @param packet - Output string representation.
     *
     * @return true on success, false on failure.
     */
    bool as_string(std::string &packet) override;

private:
    std::string packet_data;
    uint8_t src_mac[MAC_SIZE];
    uint8_t dst_mac[MAC_SIZE];
    std::string l3_data;
    uint16_t checksum;

    /**
     * @fn parse_packet
     * @brief Parses the packet string to extract L2 fields.
     */
    void parse_packet();

    /**
     * @fn validate_checksum
     * @brief Validates the packet checksum.
     *
     * @return true if checksum is valid, false otherwise.
     */
    bool validate_checksum();
};

#endif 