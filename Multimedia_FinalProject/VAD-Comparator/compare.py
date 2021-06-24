from os import minor

number = input("Pick the file number: ")
file_name = "outputVAD"+number+".txt"

mio = open("Fox/"+file_name).readlines()[0]

como = open("Como/"+file_name).readlines()[0]
tumiaz = open("Tumiaz/"+file_name).readlines()[0]

others = [como,tumiaz]
names = ["como","tumiaz"]

for i in range(0,len(others)):

    tmp = others[i]

    print("\nCONFRONTO CON "+names[i])
    if(len(mio)!=len(tmp)):
        print("Errore nella lunghezza")
        print("Lunghezza mia:"+str(len(mio)))
        print("Lunghezza altro:"+str(len(tmp)))

    uguali=0
    diversi=0

    for i in range(0,len(mio)):
        if(mio[i]==tmp[i]):
            uguali+=1
        else:
            diversi+=1

    print("Uguali:"+str(uguali))
    print("Diversi:"+str(diversi))
    print("Differenza percentuale:"+str(diversi/(uguali+diversi)*100))
