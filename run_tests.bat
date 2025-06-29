@echo off
echo Running test2...
nic_sim.exe new_tests\test2_param.in new_tests\test2_packets.in > new_tests\test2_output_fixed.txt

echo Running test3...
nic_sim.exe new_tests\test3_param.in new_tests\test3_packets.in > new_tests\test3_output_fixed.txt

echo Running test4...
nic_sim.exe new_tests\test4_param.in new_tests\test4_packets.in > new_tests\test4_output_fixed.txt

echo Running test5...
nic_sim.exe new_tests\test5_param.in new_tests\test5_packets.in > new_tests\test5_output_fixed.txt

echo Running test6...
nic_sim.exe new_tests\test6_param.in new_tests\test6_packets.in > new_tests\test6_output_fixed.txt

echo All tests completed! 