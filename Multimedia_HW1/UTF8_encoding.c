#include <stdio.h>

void encode_UTF8(FILE *source, FILE *destination){

    //Buffer di 7 byte utilizzato per la scrittura sul file destination
    unsigned char buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    //Buffer utilizzato per la lettura di 4 byte dal file source
    unsigned int read;
    
    //Valore booleano, vale 0 se non ho raggiunto la fine del file source, 1 altrimenti
    char file_end = 0;
   
    do{
    
        //Lettura dal file source di 4 byte
        fread(&read,4,1,source);

        //Controllo la condizione di fine del file
        if(feof(source)){
            file_end = 1;
            break;
        }
        
        //Codifica dei 4 byte letti da source in UTF-8 e scrittura sul file destination
        if(read < 0x80){
            buffer[0] = read;
            fwrite(buffer,1,1,destination);
        } 
        
        /**
         * Se il valore letto nella variabile read non è codificabile in maniera diretta 
         * perchè occupa più di 7 byte di informazione allora lo scompongo.
         * Il codice seguente calcola quanti byte sono necessari per la codifica UTF-8 e ne esegue la codifica.
         * In buffer[0] viene inserito l'header UTF-8, mentre nelle posizioni seguenti vengono inseriti i trailer
         */ 
        else if (read < 0x800) {
            buffer[0] = 0xC0 | (read >> 6);             // 110x xxxx
            buffer[1] = 0x80 | (read & 0x3F);           // 10xx xxxx
            fwrite(buffer,1,2,destination);
        } 
        
        else if (read < 0x10000) {
            buffer[0] = 0xE0 | (read >> 12);            // 1110 xxxx
            buffer[1] = 0x80 | ((read >> 6) & 0x3F);    // 10xx xxxx
            buffer[2] = 0x80 | (read & 0x3F);           // 10xx xxxx
            fwrite(buffer,1,3,destination);
        } 
        
        else if (read < 0x200000) {
            buffer[0] = 0xF0 | (read >> 18);            // 1110 1xxx
            buffer[1] = 0x80 | ((read >> 12) & 0x3F);   // 10xx xxxx
            buffer[2] = 0x80 | ((read >> 6) & 0x3F);    // 10xx xxxx
            buffer[3] = 0x80 | (read & 0x3F);           // 10xx xxxx
            fwrite(buffer, 1, 4, destination);
        } 
        
        else if (read < 0x4000000) {
            buffer[0] = 0xF8 | (read >> 24);            // 1110 1xxx
            buffer[1] = 0x80 | ((read >> 18) & 0x3F);   // 10xx xxxx
            buffer[2] = 0x80 | ((read >> 12) & 0x3F);   // 10xx xxxx
            buffer[3] = 0x80 | ((read >> 6) & 0x3F);    // 10xx xxxx
            buffer[4] = 0x80 | (read & 0x3F);           // 10xx xxxx
            fwrite(buffer, 1, 5, destination);
        } 
        
        else if (read < 0x80000000) {
            buffer[0] = 0xFC | (read >> 30);            // 1111 110x
            buffer[1] = 0x80 | ((read >> 24) & 0x3F);   // 10xx xxxx
            buffer[2] = 0x80 | ((read >> 18) & 0x3F);   // 10xx xxxx
            buffer[3] = 0x80 | ((read >> 12) & 0x3F);   // 10xx xxxx
            buffer[4] = 0x80 | ((read >> 6) & 0x3F);    // 10xx xxxx
            buffer[5] = 0x80 | (read & 0x3F);           // 10xx xxxx
            fwrite(buffer, 1, 6, destination);
        } 
        
        else{         
            buffer[0] = 0xFE;                           // 1111 1110
            buffer[1] = 0x80 | (read >> 30);            // 10xx xxxx
            buffer[2] = 0x80 | ((read >> 24) & 0x3F);   // 10xx xxxx
            buffer[3] = 0x80 | ((read >> 18) & 0x3F);   // 10xx xxxx
            buffer[4] = 0x80 | ((read >> 12) & 0x3F);   // 10xx xxxx
            buffer[5] = 0x80 | ((read >> 6) & 0x3F);    // 10xx xxxx
            buffer[6] = 0x80 | (read & 0x3F);           // 10xx xxxx
            fwrite(buffer, 1, 7, destination);
        }

    }while (!file_end);  

}

int main(){

    FILE *fr,*fw;
    fr = fopen("input.data","rb");
    fw = fopen("UTF8.data","wb");

    encode_UTF8(fr,fw);

    fclose(fr);
    fclose(fw);

}
