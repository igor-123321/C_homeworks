#include <stdio.h>
#include <inttypes.h>
#include <string.h>

//RUSSIAN ALPHABET ALL SYMBOLS
const uint8_t cp_1251[] = {0xA8, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 
0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 
0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 
0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0xB8};
const uint8_t koi8_r[] = {0xB3, 0xE1, 0xE2, 0xF7, 0xE7, 0xE4, 0xE5, 0xF6, 0xFA, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 
0xF3, 0xF4, 0xF5, 0xE6, 0xE8, 0xE3, 0xFE, 0xFB, 0xFD, 0xFF, 0xF9, 0xF8, 0xFC, 0xE0, 0xF1, 0xC1, 0xC2, 0xD7, 0xC7, 0xC4, 0xC5, 
0xD6, 0xDA, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD2, 0xD3, 0xD4, 0xD5, 0xC6, 0xC8, 0xC3, 0xDE, 0xDB, 0xDD, 0xDF, 
0xD9, 0xD8, 0xDC, 0xC0, 0xD1, 0xA3};
const uint8_t iso_8859_5[] = {0xA1, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 
0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF1};
const uint32_t unicodes[] = {0x0401, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 
0x041C, 0x041D, 0x041E, 0x041F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 
0x042C, 0x042D, 0x042E, 0x042F, 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 
0x043C, 0x043D, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 
0x044C, 0x044D, 0x044E, 0x044F, 0x0451};

typedef enum encodings {
    _cp_1251=0, _koi8_r, _iso_8859_5
} encode;

const char *encoding_names[]={
    "CP-1251", "KOI8-R", "ISO-8859-5"
};

uint32_t getunicode(uint8_t symbol, enum encodings encode){
    for(int i=0; i < sizeof unicodes / sizeof unicodes[0]; i++)
        {
            switch (encode)
            {
            case _cp_1251:
                if(symbol == cp_1251[i]){
                    return unicodes[i];
                }
                break;
            case _koi8_r:
                if(symbol == koi8_r[i]){
                    return unicodes[i];
                }
                break;
            case _iso_8859_5:
                if(symbol == iso_8859_5[i]){
                    return unicodes[i];
                }
                break;
            }
        }
    return 0;
    };

int uft8_encode(unsigned char *utf8, uint32_t unicode){
    if(unicode <= 0x7F){
        utf8[0] = (unsigned char) unicode >> 2;//(unsigned char) unicode;
        utf8[1] = '\0';
        return 1;
    }
    else if (unicode > 0x7F && unicode <= 0x7FF)
    {
        //uint32_t tmp = (unicode & 0x3F);
        utf8[0] = (unsigned char) ((unicode >> 6  ) | 0xC0);
        utf8[1] = (unsigned char) ((unicode & 0x3F) | 0x80);
        utf8[2] = '\0';
        return 2;
    }
    else if (unicode > 0x7FF && unicode <= 0xFFFF)
    {
        utf8[0] = (unsigned char) ((unicode >> 12 ) | 0xE0);
        utf8[1] = (unsigned char) (((unicode >> 6 ) & 0x3F) | 0x80);
        utf8[2] = (unsigned char) ((unicode & 0x3F) | 0x80);
        utf8[3] = '\0';
        return 3;
    }
    else if (unicode > 0xFFFF && unicode <= 0x10FFFF)
    {
        utf8[0] = (unsigned char) ((unicode >> 18 ) | 0xF0);
        utf8[1] = (unsigned char) (((unicode >> 12) & 0x3F) | 0x80);
        utf8[2] = (unsigned char) (((unicode >> 6 ) & 0x3F) | 0x80);
        utf8[3] = (unsigned char) ((unicode & 0x3F) | 0x80);
        utf8[4] = '\0';
        return 4;
    }
    else
        utf8 = NULL;
        return 0;    
}

int main(int argc, char *argv[])
{
    if(argc != 4) 
    {
        printf("Use main <input_filename> <encoding> <out_filename>\nEncoding types: 1. CP-1251\t2. KOI8-R\t3. ISO-8859-5\n");
        return 0;
    }
    
    FILE *fromf = fopen(argv[1], "rb");
    //FILE *fromf = fopen(".//text//iso-8859-5.txt", "rb"); 
    if(fromf == NULL){
        //perror(".//text//iso-8859-5.txt");
        perror(argv[1]);
        return 1;
    }
    FILE *tof = fopen(argv[3], "w+b");
    //FILE *tof = fopen(".//text//result_utf-8.txt", "w+b");
    if(tof == NULL){
        //perror(".//text//result_utf-8.txt");
        perror(argv[3]);
        return 1;
    }

    char * file_encoding = argv[2]; //"ISO-8859-5";
    int j = 0;
    for(j; j<sizeof encoding_names/sizeof encoding_names[0]; j++){
        if(!strcmp(file_encoding,encoding_names[j])){
            break;
        }
    }
    if(j==3){
        printf("Wrong encoding type!\nPlease select one of this types:\nCP-1251 | KOI8-R | ISO-8859-5\n");
        return 1;
    }
    unsigned char symbol;
    while(fread(&symbol, 1, 1, fromf) == 1) {
        if(symbol < 128){
                fwrite(&symbol, sizeof symbol, 1, tof);
            }
        else{
            unsigned char utf8[4];
            int num_of_bytes = uft8_encode(utf8,getunicode(symbol,j));
            fwrite(utf8, num_of_bytes, 1, tof);
        }
    }
    fclose(tof);
    fclose(fromf);
    return 0;
}