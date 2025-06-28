/**
 * @file L4.h
 * @brief This header defines the L4 packet class for the NIC simulation project.
 *
 * L4 packets represent the Transport layer packets that include source and destination
 * ports and application data.
 */

#ifndef __L4__
#define __L4__

#include <cstdint>
#include "packets.hpp"

class l4_packet : public generic_packet {
public:
    /**
     * @fn l4_packet
     * @brief Constructor for L4 packet.
     * 
     * @param packet_str - String representation of the L4 packet.
     *
     * @return New L4 packet object.
     */
    l4_packet(const std::string& packet_str);

    /**
     * @fn validate_packet
     * @brief Validates the L4 packet by checking if communication is open.
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
     * @brief Processes the L4 packet by storing data in open_port struct.
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
     * @brief Converts the L4 packet to string format.
     *
     * @param packet - Output string representation.
     *
     * @return true on success, false on failure.
     */
    bool as_string(std::string &packet) override;

private:
    std::string packet_data;
    uint16_t src_port;
    uint16_t dst_port;
    std::string data;

    /**
     * @fn parse_packet
     * @brief Parses the packet string to extract L4 fields.
     */
    void parse_packet();

    /**
     * @fn find_open_port
     * @brief Finds the matching open port for this communication.
     *
     * @param open_ports - Vector containing all the NIC's open ports.
     *
     * @return Index of matching open port, or -1 if not found.
     */
    int find_open_port(const open_port_vec& open_ports);
};

#endif 