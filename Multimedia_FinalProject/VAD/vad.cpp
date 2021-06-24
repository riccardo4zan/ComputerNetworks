#include<iostream>
#include<fstream>
#include<string>
#include<vector>

//Elaboration functions
std::vector<std::vector<int8_t>> packetize(std::vector<int8_t> buffer);
float computePacketPower(std::vector<int8_t> packet);
int categorize(std::vector<int8_t> packet,float threshold);

//Output functions
void printResult(std::vector<int> results, std::string output_path);
void createPlayableFile(std::vector<std::vector<int8_t>> packets, std::vector<int> flags, std::string output_path);

int main(){

    //Input file
    std::string input_path;
    std::string output_path;

    //Input dei parametri
    std::cout<<"File di input"<<std::endl;
    std::getline(std::cin,input_path);
    std::cout<<"File di output"<<std::endl;
    std::getline(std::cin,output_path);

    //Open input file
    std::ifstream input(input_path, std::ios::binary);
    std::vector<int8_t> buffer(std::istreambuf_iterator<char>(input), {});
    input.close();

    //Create packages, each one composed of <=160 samples (20ms)
    std::vector<std::vector<int8_t>> packets = packetize(buffer);

    //Assuming first 200ms of the transmission are composed only of noise, 
    //calculate the average power of the noise signal (200ms -> 10 packets)
    unsigned int first_packages = 10;
    float avg_noise_power = 0;
    for(int i=0;i<first_packages;i++){
        avg_noise_power += computePacketPower(packets[i]);
    }
    avg_noise_power /= first_packages;

    //Categorize packets with bigger energies than the average noise as voice
    std::vector<int> results;
    for(int i=0;i<packets.size();i++){
        results.push_back(categorize(packets[i],avg_noise_power));
    }

    //Printing out results
    printResult(results, output_path);

    //Create playable audio file
    //createPlayableFile(packets,results,output_path+".data");
}

//Create packages, each one composed of <=160 samples (20ms)
std::vector<std::vector<int8_t>> packetize(std::vector<int8_t> buffer){

    std::vector<std::vector<int8_t>> packets;

    unsigned int index = 0;
    unsigned int buffer_size = buffer.size();

    while(index < buffer_size){        
        std::vector<int8_t>::const_iterator first = buffer.begin() + index;

        unsigned int end;
        if(index+160 < buffer_size){
            end=index+160;
        } else {
            end=buffer_size;
        }
        index=end;
        std::vector<int8_t>::const_iterator last = buffer.begin() + index;
        std::vector<int8_t> newVec(first, last);
        packets.push_back(newVec);
    }

    return packets;
}

float computePacketPower(std::vector<int8_t> packet){
    float power = 0;
    for(int i=0;i<packet.size();i++){
        power += packet[i]*packet[i];
    }
    return power;
}

//Recives as a parameter a packet and a threshold
//returns 1 if the packet contains voice, 0 otherwise
int categorize(std::vector<int8_t> packet, float threshold){
    float decision_threshold = threshold * 2;
    float packet_power = computePacketPower(packet);

    if(packet_power>decision_threshold) return 1;
    else return 0;
}

//Prints the result on a file in output_path
void printResult(std::vector<int> results, std::string output_path){
    std::ofstream output;
    output.open(output_path);
    for(int i=0;i<results.size();i++){
        output<<results[i];
    }
}

//Creates a playable raw file starting from packets and flags.
//Packets are raw bytes readed from the input file, flags are
//computed using the VAD algorythm.
//The output is a raw file without silence.
void createPlayableFile(std::vector<std::vector<int8_t>> packets, std::vector<int> flags, std::string output_path){
    std::ofstream output;
    output.open(output_path);
    for(int i=0;i<flags.size();i++){
        //Print to the output file only if the flag is set to 1
        if(flags[i]==1){
            //Print the packet i to the output file
            std::vector<int8_t> packet = packets[i];

            for(int j=0;j<packet.size();j++){
                output << packet[j];
            }
        }
    }
    output.close();
}
