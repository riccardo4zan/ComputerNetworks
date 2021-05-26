#! /bin/sh

#Increment each ping size of 32 bits
size=32

#Number of pings
pings=45

#Number of ping each ping
attempts=120

for i in `seq 1 $pings`
do
    let tmp=size*i
    echo "Ping size: $tmp" > pings.data
    ping 88.80.187.84 -c $attempts -s $tmp -M do > pings.data
    echo "Percentuale completamento: $i / 46"
done
