#ifndef L3_H
#define L3_H

#include "packets.hpp"
#include "common.hpp"
#include <string>
#include <cstdint>

class l3_packet : public generic_packet {
private:
    std::string packet_data;
    uint8_t src_ip[IP_V4_SIZE];
    uint8_t dst_ip[IP_V4_SIZE];
    uint32_t ttl;
    uint32_t checksum;
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t address;
    uint8_t data[PACKET_DATA_SIZE];
    std::string l4_data;

    uint32_t calculate_checksum();
    bool is_local_network(uint8_t ip_addr[IP_V4_SIZE], uint8_t nic_ip[IP_V4_SIZE], uint8_t mask);

public:
    /**
     * @fn l3_packet
     * @brief Constructor of the class.
     * 
     * @param packet - String representation of the packet.
     *
     * @return New packet object.
     */
    l3_packet(const std::string& packet);

    /**
     * @fn validate_packet
     * @brief Check whether the packet is valid.
     *
     * @param [in] open_ports - Vector containing all the NIC's open ports.
     * @param [in] ip - NIC's IP address.
     * @param [in] mask - NIC's mask; together with the IP,
     *               determines the NIC's local net.
     * @param [in] mac - NIC's MAC address.
     *
     * @return true if the packet is valid and ready for processing.
     *         false if the packet isn't valid and should be discarded.
     */
    bool validate_packet(open_port_vec open_ports,
                        uint8_t ip[IP_V4_SIZE],
                        uint8_t mask,
                        uint8_t mac[MAC_SIZE]) override;

    /**
     * @fn proccess_packet
     * @brief Modify the packet and return the memory location it should be
     *        stored in. In the case of local DRAM, the function will store
     *        the packet as a string in the relevant 'open_port' struct.
     *
     * @param [in] open_ports - Vector containing all the NIC's open ports.
     * @param [in] ip - NIC's IP address.
     * @param [in] mask - NIC's mask; together with the IP, determines the NIC's
     *        local net.
     * @param [out] dst - Reference to enum that indicate the memory space where
     *        the packet should be stored:
     *         LOCAL_DRAM - the function stored it to the currect struct.
     *         RQ - Should be stored as a string in RQ.
     *         TQ - Should be stored as string in TQ.
     *
     * @return true in success, false in failure (e.g memory allocation failed).
     */
    bool proccess_packet(open_port_vec &open_ports,
                        uint8_t ip[IP_V4_SIZE],
                        uint8_t mask,
                        memory_dest &dst) override;

    /**
     * @fn as_string
     * @brief Convert the packet to string.
     *
     * @param [out] packet - Packet as a string, ready to be stored in RQ/TQ.
     *
     * @return true in success, false in failure (e.g memory allocation failed).
     */
    virtual bool as_string(std::string &packet) override;

    /**
     * @brief Destructor
     */
    ~l3_packet() = default;
};

#endif 