#! /bin/sh

# Increment each ping size of 32 bits
size=32

# Number of pings
pings=46

for i in `seq 0 $pings`
do
    let tmp=size*i
    echo "Ping size: $tmp" >> pings.data
    ping 88.80.187.84 -c 20 -s $tmp -D >> pings.data
    echo "$i / 46"
done

