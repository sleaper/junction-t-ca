#!/bin/bash

make
./ca
python3 scripts/spacetime.py
python3 scripts/pls.py
