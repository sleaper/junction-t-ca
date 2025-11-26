#!/bin/bash

mkdir -p results

make -j 4

# 0 aggressive drivers, varying density, symmetric rules
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_sym.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0 -w 1000 -s 5000 -p | tail -n 1 >>results/flow_density_sym.csv
done

# 0 aggressive drivers, varying density, assymmetric rules
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_asymmetric.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0 -w 1000 -s 5000 -p -y | tail -n 1 >>results/flow_density_asymmetric.csv
done

# flow vs density with 20% aggressive drivers
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_03_aggressive.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0.3 -w 1000 -s 5000 -p | tail -n 1 >>results/flow_density_03_aggressive.csv
done

# flow vs density with 60% aggressive drivers
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_06_aggressive.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0.6 -w 1000 -s 5000 -p | tail -n 1 >>results/flow_density_06_aggressive.csv
done

# flow vs density with 90% aggressive drivers
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_09_aggressive.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0.9 -w 1000 -s 5000 -p | tail -n 1 >>results/flow_density_09_aggressive.csv
done

python3 scripts/graphs.py
