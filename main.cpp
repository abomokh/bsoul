/**
 * @file main.hpp
 * @brief This C++ file contains the main function of the NIC simulation project.
 */

#include <iostream>
#include "NIC_sim.hpp"

int main(int argc, char *argv[]) {
    std::string param_file;
    std::string packet_file;
    
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <param_file> <packet_file>" << std::endl;
        return 1;
    }
    
    param_file = argv[1];
    packet_file = argv[2];
    
    try {
        nic_sim simulator(param_file);
        simulator.nic_flow(packet_file);
        simulator.nic_print_results();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}