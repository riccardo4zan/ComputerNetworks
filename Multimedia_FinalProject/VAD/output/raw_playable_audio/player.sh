#/bin/bash

# This script uses 'play' function in SOX (sudo apt install SOX)

# $1 refers to the first element passed as a parameter to this script
# -t raw    -> Raw format file
# -r 8k     -> Sampling rate = 8k
# -e signed -> Bytes are codified as integer with sign
# -b 8      -> Each sample is 8 bit
# -c 1      -> The file has only 1 channel

play -t raw -r 8k -e signed -b 8 -c 1 $1