#!/bin/bash

mkdir -p results

make -j 4

# 0 aggressive drivers, varying density
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_sym.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0 -w 1000 -s 5000 -p | tail -n 1 >>results/flow_density_sym.csv
done

# 0 aggressive drivers, varying density, assymmetric rules
echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_density_asymmetric.csv
for density in $(seq 0 0.01 0.5); do
    ./ca -d $density -a 0 -w 1000 -s 5000 -p -y | tail -n 1 >>results/flow_density_asymmetric.csv
done

# # flow vs varying aggressive driver ratio at low density
# echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_aggressive_low_density.csv
# for aggressive in $(seq 0 0.1 1.0); do
#     ./ca -d 0.02 -a $aggressive -w 1000 -s 5000 -p | tail -n 1 >>results/flow_aggressive_low_density.csv
# done

# # flow vs aggressive driver at peak density
# echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_aggressive.csv
# for aggressive in $(seq 0 0.1 1.0); do
#     ./ca -d 0.09 -a $aggressive -w 1000 -s 5000 -p | tail -n 1 >>results/flow_aggressive.csv
# done

# # flow vs aggressive driver at high density
# echo "density,aggressive,flow,lane_change_rate,left_flow,right_flow" >results/flow_aggressive_high_density.csv
# for aggressive in $(seq 0 0.1 1.0); do
#     ./ca -d 0.4 -a $aggressive -w 1000 -s 5000 -p | tail -n 1 >>results/flow_aggressive_high_density.csv
# done

python3 scripts/graphs.py
