#include <stdio.h>

/**
 * Funzione che effettua la decodifica da UTF-8 a UCS4
 * @param source: file contenente i caratteri UTF-8 da codificare
 * @param destination: file su cui scrivere i caratteri presenti in source 
 *                     utilizzando la codifica UCS4
 * @return 0 se l'operazione è andata a buon fine, >0 se si sono riscontrati errori
 */ 

unsigned int decode_UTF8(FILE *source, FILE *destination){

    //Buffer utilizzato per la lettura di 1 byte dal file source
    unsigned char c;
    
    //Valore utilizzato per la creazione di una maschera di valori 1 per and bit a bit
    unsigned char mask;
    
    //Valori utilizzati come bool, se il loro valore è 0 allora sono false, true altrimenti
    char error=0, file_end = 0;
    
    //Contiene la lunghezza del prefisso 
    unsigned int prefix_length;
    
    //Accumulatore nel quale verrà inserito il valore decodificato
    unsigned int value;

    do{
        //Lettura di 1 byte dal file source
        c = fgetc(source);

        //Controllo la condizione di fine del file
        if(feof(source)){
            file_end = 1;
            break;
        }
        
        if(ferror(source)){
            error = 1;
            break;
        }

        //Nel caso il valore sia minore di =x80 allora è stato codificato in un solo byte
        if (c < 0x80) value = c;

        else {
            
            //Identificazione della lunghezza attesa della codifica
            if(c < 0xE0) prefix_length=2;
            else if(c < 0xF0) prefix_length=3;
            else if(c < 0xF8) prefix_length=4;
            else if(c < 0xFC) prefix_length=5;
            else if(c < 0xFE) prefix_length=6;
            else if(c == 0xFE) prefix_length=7;
            else if(c > 0xFE) error=1;

            //Rimozione dell'header dal valore letto
            mask = 0xFF >> prefix_length;
            value = c & mask;

            /**
             * Reperimento dei byte seguenti a quello in cui è
             * stato individuato l'header. Il numero di byte da
             * reperire è dato dalla lunghezza del numero di valori
             * pari a '1' che si trovano nell'header.
             */
            for (int i=0; i<prefix_length-1; i++) {

                //Lettura di altri 4 byte dal file
                c = fgetc(source);

                /**
                 * Controllo degli errori. Un errore viene riscontrato se
                 * Il valore letto supera il massimo valore consetito per un trailer (0xBF = 1011 1111)
                 * Viene raggiunta la fine del file di input
                 * Altro tipo di errore nella manipolazione del file
                 */ 
                if (c > 0xBF || feof(source) || ferror(source)) { 
                    error = 1;
                    break;
                }

                //Rimozione del prefisso 10XX XXXX dal valore letto
                c = c & 0x3F;
                //Aggiunta dei 6 bit letti all'accumulatore value
                value = value << 6;
                value += c;
            }
        }

        //Gestione dell'errore nella decodifica
        if (!error){
            //Se non è stato rilevato nessun errore scrivi value nel file destination
            fwrite(&value, 4, 1, destination);
        }

    }while (!file_end && !error);

    return !error;  
}

int main(){
    FILE *fr,*fw;
    fr = fopen("UTF8.data","rb");
    fw = fopen("output.data","wb");

    int res = decode_UTF8(fr,fw);
    if(!res)printf("Errore nella decodifica\n");

    fclose(fr);
    fclose(fw);
}

