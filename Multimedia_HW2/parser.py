import numpy as np
import xlsxwriter as xls

def speed(lines):
    ping_sizes = []
    rtt_mins = []

    for line in lines:
        
        # Parse the length of the ping 
        if("Ping" in line):
            ps = float(line.split(" ")[2].split("\n")[0])
            ping_sizes.append(ps)

        # Parse the min rtt value
        elif("rtt" in line):
            data = line.split(" ")[3]
            rtt_min = float(data.split("/")[0])
            rtt_mins.append(rtt_min)

    res = np.polyfit(ping_sizes,rtt_mins,1)

    speed = (8*((2/res[0])*1000))/1000000
    return speed

def toXLSX(lines):
    sheet = xls.Workbook("dati.xlsx")
    worksheet = sheet.add_worksheet()

    worksheet.write("A1","SIZE")
    worksheet.write("B1","RTT MIN")
    worksheet.write("C1","RTT AVG")
    worksheet.write("D1","RTT MAX")
    worksheet.write("E1","RTT MDEV")

    line_index = 2

    for line in lines:

        if("Ping" in line):
            ps = line.split(" ")[2].split("\n")[0]
            worksheet.write("A"+str(line_index),ps)

        if("rtt" in line):
            values = line.split(" ")[3].split("/")
            rtt_min= values[0].replace(".",",")
            rtt_avg= values[1].replace(".",",")
            rtt_max= values[2].replace(".",",")
            rtt_mdev= values[3].replace(".",",")

            worksheet.write("B"+str(line_index),rtt_min)
            worksheet.write("C"+str(line_index),rtt_avg)
            worksheet.write("D"+str(line_index),rtt_max)
            worksheet.write("E"+str(line_index),rtt_mdev)

            line_index+=1   

    sheet.close()



#MAIN
data_file = input("Inserisci il nome del file contenente i dati: ")

file = open(data_file, "r")
lines = file.readlines()

print("Speed Mbit/s:",speed(lines))

want_xlsx = input("Premi Y per creare il file dati.xlsx")
if(want_xlsx == 'Y'):
    toXLSX(lines)
