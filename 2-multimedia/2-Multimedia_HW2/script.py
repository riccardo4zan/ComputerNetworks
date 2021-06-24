file = open("pings.data", "r")
lines = file.readlines()

for line in lines:
    
    if("Ping" in line):
        ping_size = line.split(" ")[2]
        print("Ping size: {}".format(ping_size),end="")

    elif("rtt" in line):
        data = line.split(" ")[3]
        rtt_min = data.split("/")[0]
        print("Min RTT: {}".format(rtt_min),end="\n\n")


